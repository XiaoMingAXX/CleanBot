/**
  ******************************************************************************
  * @file    imu_task.c
  * @brief   IMU任务（WIT9011，USART3+DMA+空闲中断）实现
  ******************************************************************************
  */

#include "imu_task.h"
#include "cmsis_os.h"
#include "usart.h"
#include "stm32f4xx_hal.h"
#include "ring_buffer.h"
#include <string.h>
#include <stdbool.h>

/* ---------------- 配置区 ---------------- */
#define IMU_UART_HANDLE                huart3
#define IMU_DMA_RX_BUFFER_SIZE         256
#define IMU_RING_BUFFER_SIZE           1024

/* WIT9011帧定义：0x55 | id | 8字节数据 | 校验(累加低8位) */
#define WIT_FRAME_HEAD                 0x55
#define WIT_ID_ACC                     0x51
#define WIT_ID_GYRO                    0x52
#define WIT_ID_ANGLE                   0x53
#define WIT_FRAME_LEN                  11

/* -------------------------------------- */

/* DMA接收临时缓冲（HAL UART RxEvent DMA to IDLE使用） */
static uint8_t s_imuDmaRxBuf[IMU_DMA_RX_BUFFER_SIZE];
/* 任务侧环形缓冲（拼接所有收到的字节流） */
static RingBuffer_t s_rxRing;
static uint8_t s_rxRingStorage[IMU_RING_BUFFER_SIZE];

/* 最近一次解算结果（单位：deg、deg/s、g） */
static volatile float s_roll = 0.0f, s_pitch = 0.0f, s_yaw = 0.0f;
static volatile float s_gx = 0.0f, s_gy = 0.0f, s_gz = 0.0f;
static volatile float s_ax = 0.0f, s_ay = 0.0f, s_az = 0.0f;

/* 工具函数：WIT 16位有符号，缩放因子见WIT文档
   - 加速度：原始单位 mg (±16g) -> g：raw/32768*16
   - 角速度：原始单位 deg/s (±2000dps)：raw/32768*2000
   - 角度：原始单位 deg (±180)：raw/32768*180
*/
static inline float wit_to_acc_g(int16_t raw)   { return ((float)raw) * 16.0f / 32768.0f; }
static inline float wit_to_gyro_dps(int16_t raw){ return ((float)raw) * 2000.0f / 32768.0f; }
static inline float wit_to_angle_deg(int16_t raw){ return ((float)raw) * 180.0f / 32768.0f; }

/* 校验：帧内前10字节累加低8位与最后一字节相等 */
static bool wit_check_sum(const uint8_t *frame)
{
	uint8_t sum = 0;
	for (int i = 0; i < WIT_FRAME_LEN - 1; i++) sum += frame[i];
	return (sum == frame[10]);
}

/* 解析一帧数据 */
static void wit_parse_frame(const uint8_t *frame)
{
	uint8_t id = frame[1];
	const uint8_t *p = &frame[2];
	int16_t x = (int16_t)((p[1] << 8) | p[0]);
	int16_t y = (int16_t)((p[3] << 8) | p[2]);
	int16_t z = (int16_t)((p[5] << 8) | p[4]);
	/* p[6], p[7] 保留/温度，不用 */

	switch (id) {
	case WIT_ID_ACC:
		s_ax = wit_to_acc_g(x);
		s_ay = wit_to_acc_g(y);
		s_az = wit_to_acc_g(z);
		break;
	case WIT_ID_GYRO:
		s_gx = wit_to_gyro_dps(x);
		s_gy = wit_to_gyro_dps(y);
		s_gz = wit_to_gyro_dps(z);
		break;
	case WIT_ID_ANGLE:
		s_roll  = wit_to_angle_deg(x);
		s_pitch = wit_to_angle_deg(y);
		s_yaw   = wit_to_angle_deg(z);
		break;
	default:
		break;
	}
}

/* 从环形缓冲中找帧并解析 */
static void wit_consume_ring(void)
{
	uint32_t count = RingBuffer_GetCount(&s_rxRing);
	if (count < WIT_FRAME_LEN) return;

	/* 使用一个小窗口缓存，避免逐字节Get带来过多函数调用 */
	uint8_t window[WIT_FRAME_LEN];
	uint8_t byte = 0;
	/* 简单状态机：找0x55 -> 尝试取余下10字节 -> 校验通过则解析，否则丢1字节继续 */
	while (RingBuffer_GetCount(&s_rxRing) >= WIT_FRAME_LEN) {
		/* 对齐到帧头0x55 */
		RingBuffer_Get(&s_rxRing, &byte);
		if (byte != WIT_FRAME_HEAD) {
			continue;
		}
		window[0] = WIT_FRAME_HEAD;
		/* 取余下10字节；若不够则把已取的放回（简单处理：直接退出等待下次） */
		if (RingBuffer_GetCount(&s_rxRing) < (WIT_FRAME_LEN - 1)) {
			/* 将帧头“丢弃”，等下次更多数据 */
			break;
		}
		for (int i = 1; i < WIT_FRAME_LEN; i++) {
			RingBuffer_Get(&s_rxRing, &window[i]);
		}
		/* 校验并解析 */
		if (wit_check_sum(window)) {
			wit_parse_frame(window);
		} else {
			/* 校验失败，丢弃此帧并继续 */
		}
	}
}


/* 初始化：启动DMA接收到IDLE */
static void imu_uart_start_rx_to_idle(void)
{
	/* 使用HAL扩展API：接收至IDLE事件（DMA） */
	/* 说明：如果CubeMX库版本支持HAL_UARTEx_ReceiveToIdle_DMA，则优先用该方式，
	   在IDLE或缓冲满时触发 HAL_UARTEx_RxEventCallback 回调。 */
	HAL_UARTEx_ReceiveToIdle_DMA(&IMU_UART_HANDLE, s_imuDmaRxBuf, IMU_DMA_RX_BUFFER_SIZE);
	/* 使能DMA的半传输/传输完成中断由HAL配置；
	   如需传统IDLE中断手动处理，可额外打开IDLEIE，但这里不需要重复。 */
}

/* HAL DMA IDLE接收回调：把block拷入环形缓冲 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
	if (huart != &IMU_UART_HANDLE) return;
	/* 将收到的Size字节写入环形缓冲（溢出时丢弃超出部分） */
	uint32_t written = RingBuffer_PutData(&s_rxRing, s_imuDmaRxBuf, Size);
	(void)written;
	/* 继续接收 */
	HAL_UARTEx_ReceiveToIdle_DMA(&IMU_UART_HANDLE, s_imuDmaRxBuf, IMU_DMA_RX_BUFFER_SIZE);
}

/* 公共接口 */
void IMUTask_GetEuler(float *roll, float *pitch, float *yaw)
{
	if (roll)  *roll  = s_roll;
	if (pitch) *pitch = s_pitch;
	if (yaw)   *yaw   = s_yaw;
}
void IMUTask_GetGyro(float *gx, float *gy, float *gz)
{
	if (gx) *gx = s_gx;
	if (gy) *gy = s_gy;
	if (gz) *gz = s_gz;
}
void IMUTask_GetAccel(float *ax, float *ay, float *az)
{
	if (ax) *ax = s_ax;
	if (ay) *ay = s_ay;
	if (az) *az = s_az;
}

/* 任务主体：持续消费环缓 -> 解析 -> 按200Hz上报 */
void IMUTask_Run(void *argument)
{
	/* 初始化环缓与DMA接收 */
	RingBuffer_Init(&s_rxRing, s_rxRingStorage, IMU_RING_BUFFER_SIZE);
	imu_uart_start_rx_to_idle();

	for (;;) {
		/* 消费解析字节流 */
		wit_consume_ring();
		osDelay(1);
	}
}


