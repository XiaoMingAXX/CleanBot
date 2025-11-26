/**
  ******************************************************************************
  * @file    usb_comm_task.c
  * @brief   USB通信任务实现（CDC协议 + 自定义帧）
  ******************************************************************************
  */

#include "usb_comm_task.h"
#include "usb_comm.h"
#include "motor_ctrl_task.h"
#include "imu_task.h"
#include "CleanBotApp.h"
#include "encoder.h"
#include "led.h"
#include "cmsis_os.h"
#include <math.h>
#include <string.h>

/* 外部应用对象 */
extern CleanBotApp_t *g_pCleanBotApp;

#ifndef USB_COMM_DEBUG_MODE
#define USB_COMM_DEBUG_MODE 1
#endif

/* ========================== 协议定义 ========================== */
#define USB_FRAME_HEADER0             0x55
#define USB_FRAME_HEADER1             0xAA
#define USB_PROTOCOL_VERSION          0x01
#define USB_MAX_PAYLOAD_SIZE          96U
#define USB_MAX_FRAME_SIZE            (USB_MAX_PAYLOAD_SIZE + 8U)

typedef enum {
    USB_MSG_CONTROL_CMD      = 0x10,
    USB_MSG_IMU_FEEDBACK     = 0x20,
    USB_MSG_WHEEL_FEEDBACK   = 0x21,
    USB_MSG_SENSOR_STATUS    = 0x22,
    USB_MSG_SYSTEM_STATUS    = 0x23,
    USB_MSG_ACK_REPLY        = 0x24
} UsbMsgId_t;

typedef enum {
    WORK_MODE_IDLE = 0,
    WORK_MODE_AUTO,
    WORK_MODE_EDGE,
    WORK_MODE_BOW,
    WORK_MODE_REMOTE,
    WORK_MODE_DOCK
} WorkMode_t;

typedef enum {
    ACK_STATUS_OK = 0,
    ACK_STATUS_FAIL = 1,
    ACK_STATUS_BUSY = 2
} AckStatus_t;

typedef enum {
    RX_WAIT_HEADER0 = 0,
    RX_WAIT_HEADER1,
    RX_READ_VERSION,
    RX_READ_LEN_L,
    RX_READ_LEN_H,
    RX_READ_MSG_ID,
    RX_READ_SEQ,
    RX_READ_PAYLOAD,
    RX_READ_CRC_L,
    RX_READ_CRC_H
} RxState_t;

typedef struct {
    RxState_t  state;
    uint8_t    version;
    uint16_t   payloadLen;
    uint8_t    msgId;
    uint8_t    seq;
    uint16_t   payloadIndex;
    uint8_t    payload[USB_MAX_PAYLOAD_SIZE];
    uint16_t   rxCrc;
} UsbRxParser_t;

typedef struct {
    float leftSpeedMs;
    float rightSpeedMs;
    WorkMode_t workMode;
    uint8_t fanLevel;
    uint8_t waterLevel;
    uint8_t brushLeftLevel;
    uint8_t brushRightLevel;
    uint8_t cmdSeq;
    bool ackRequired;
} ControlCommandState_t;

typedef struct {
    uint8_t imuSeq;
    uint8_t wheelSeq;
    uint8_t sensorSeq;
    uint8_t systemSeq;
    uint8_t ackSeq;
    uint8_t genericSeq;
} UsbSeqState_t;

/* 传感器状态位 */
#define FAULT_FLAG_USB_LOSS       (1U << 0)
#define FAULT_FLAG_BUMPER_LEFT    (1U << 1)
#define FAULT_FLAG_BUMPER_RIGHT   (1U << 2)
#define FAULT_FLAG_CLIFF          (1U << 3)
#define FAULT_FLAG_DOCK_FAILED    (1U << 4)

/* 物理常量 */
#define G_TO_M_S2                 9.80665f

/* 发送频率 */
#define PERIOD_WHEEL_MS           5U     /* 200Hz */
#define PERIOD_IMU_MS             5U    /* 100Hz */
#define PERIOD_SENSOR_MS          20U    /* 50Hz */
#define CONNECTION_POLL_MS        50U

/* 控制命令payload最小长度（不含保留字节） */
#define CONTROL_CMD_MIN_PAYLOAD   14U

/* ========================== 静态状态 ========================== */
static UsbRxParser_t          s_rxParser;
static ControlCommandState_t  s_ctrlState;
static UsbSeqState_t          s_seqState;
static uint8_t                s_heartbeatCounter = 0;
static bool                   s_usbSafeStopped = false;
static bool                   s_lastUsbConnected = false;
static uint32_t               s_lastWheelTick = 0;
static uint32_t               s_lastImuTick = 0;
static uint32_t               s_lastSensorTick = 0;
static uint32_t               s_lastConnPollTick = 0;

/* ========================== 工具函数声明 ========================== */
static uint16_t crc16_ccitt(const uint8_t *data, uint16_t len);
static void USBCommTask_ResetParser(void);
static void USBCommTask_ProcessByte(uint8_t byte);
static void USBCommTask_DispatchFrame(uint8_t msgId, uint8_t seq,
                                      const uint8_t *payload, uint16_t len);
static void USBCommTask_HandleControlCmd(uint8_t seq,
                                         const uint8_t *payload,
                                         uint16_t len);
static void USBCommTask_ApplyControl(const ControlCommandState_t *ctrl);
static void USBCommTask_SendAck(AckStatus_t status, uint8_t info);
static void USBCommTask_SendWheelTelemetry(void);
static void USBCommTask_SendImuTelemetry(void);
static void USBCommTask_SendSensorTelemetry(void);
static void USBCommTask_ProcessRxStream(void);
static void USBCommTask_HandleConnection(void);
static void USBCommTask_SafeStop(void);
static uint8_t USBCommTask_NextSeq(uint8_t msgId);
static void USBCommTask_SendFrame(uint8_t msgId,
                                  const uint8_t *payload,
                                  uint16_t payloadLen);
static void USBCommTask_UpdateLed(bool connected);
static BrushMotorLevel_t USBCommTask_ToBrushLevel(uint8_t level);
static PumpMotorLevel_t USBCommTask_ToPumpLevel(uint8_t level);
static FanMotorLevel_t USBCommTask_ToFanLevel(uint8_t level);
static WorkMode_t USBCommTask_ToWorkMode(uint8_t mode);
static uint8_t USBCommTask_GetDockStatus(void);

/* ========================== CRC实现 ========================== */
static uint16_t crc16_ccitt(const uint8_t *data, uint16_t len)
{
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; ++i) {
        crc ^= (uint16_t)data[i] << 8;
        for (uint8_t j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ 0x1021;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

/* ========================== 基础工具 ========================== */
static void USBCommTask_ResetParser(void)
{
    memset(&s_rxParser, 0, sizeof(s_rxParser));
    s_rxParser.state = RX_WAIT_HEADER0;
}

static uint8_t USBCommTask_NextSeq(uint8_t msgId)
{
    switch (msgId) {
        case USB_MSG_IMU_FEEDBACK:
            return s_seqState.imuSeq++;
        case USB_MSG_WHEEL_FEEDBACK:
            return s_seqState.wheelSeq++;
        case USB_MSG_SENSOR_STATUS:
            return s_seqState.sensorSeq++;
        case USB_MSG_SYSTEM_STATUS:
            return s_seqState.systemSeq++;
        case USB_MSG_ACK_REPLY:
            return s_seqState.ackSeq++;
        default:
            return s_seqState.genericSeq++;
    }
}

static void USBCommTask_SendFrame(uint8_t msgId,
                                  const uint8_t *payload,
                                  uint16_t payloadLen)
{
    if (g_pCleanBotApp == NULL || payloadLen > USB_MAX_PAYLOAD_SIZE) {
        return;
    }

    uint8_t frame[USB_MAX_FRAME_SIZE];
    uint16_t idx = 0;

    frame[idx++] = USB_FRAME_HEADER0;
    frame[idx++] = USB_FRAME_HEADER1;
    frame[idx++] = USB_PROTOCOL_VERSION;
    frame[idx++] = (uint8_t)(payloadLen & 0xFF);
    frame[idx++] = (uint8_t)((payloadLen >> 8) & 0xFF);
    frame[idx++] = msgId;
    frame[idx++] = USBCommTask_NextSeq(msgId);

    if (payloadLen > 0 && payload != NULL) {
        memcpy(&frame[idx], payload, payloadLen);
        idx += payloadLen;
    }

    uint16_t crc = crc16_ccitt(&frame[2], (uint16_t)(5U + payloadLen));
    frame[idx++] = (uint8_t)(crc & 0xFF);
    frame[idx++] = (uint8_t)((crc >> 8) & 0xFF);

    USB_Comm_Send(&g_pCleanBotApp->usbComm, frame, idx);
}

static void USBCommTask_UpdateLed(bool connected)
{
    if (g_pCleanBotApp == NULL) {
        return;
    }
    if (connected) {
        LED_Off(&g_pCleanBotApp->led4);
    } else {
        LED_On(&g_pCleanBotApp->led4);
    }
}

/* ========================== 解析器 ========================== */
static void USBCommTask_ProcessByte(uint8_t byte)
{
    switch (s_rxParser.state) {
        case RX_WAIT_HEADER0:
            if (byte == USB_FRAME_HEADER0) {
                s_rxParser.state = RX_WAIT_HEADER1;
            }
            break;
        case RX_WAIT_HEADER1:
            if (byte == USB_FRAME_HEADER1) {
                s_rxParser.state = RX_READ_VERSION;
            } else {
                s_rxParser.state = RX_WAIT_HEADER0;
            }
            break;
        case RX_READ_VERSION:
            s_rxParser.version = byte;
            s_rxParser.state = RX_READ_LEN_L;
            break;
        case RX_READ_LEN_L:
            s_rxParser.payloadLen = byte;
            s_rxParser.state = RX_READ_LEN_H;
            break;
        case RX_READ_LEN_H:
            s_rxParser.payloadLen |= ((uint16_t)byte << 8);
            if (s_rxParser.payloadLen > USB_MAX_PAYLOAD_SIZE) {
                USBCommTask_ResetParser();
            } else {
                s_rxParser.state = RX_READ_MSG_ID;
            }
            break;
        case RX_READ_MSG_ID:
            s_rxParser.msgId = byte;
            s_rxParser.state = RX_READ_SEQ;
            break;
        case RX_READ_SEQ:
            s_rxParser.seq = byte;
            s_rxParser.payloadIndex = 0;
            if (s_rxParser.payloadLen == 0) {
                s_rxParser.state = RX_READ_CRC_L;
            } else {
                s_rxParser.state = RX_READ_PAYLOAD;
            }
            break;
        case RX_READ_PAYLOAD:
            s_rxParser.payload[s_rxParser.payloadIndex++] = byte;
            if (s_rxParser.payloadIndex >= s_rxParser.payloadLen) {
                s_rxParser.state = RX_READ_CRC_L;
            }
            break;
        case RX_READ_CRC_L:
            s_rxParser.rxCrc = byte;
            s_rxParser.state = RX_READ_CRC_H;
            break;
        case RX_READ_CRC_H: {
            s_rxParser.rxCrc |= ((uint16_t)byte << 8);
            uint8_t crcBuf[USB_MAX_PAYLOAD_SIZE + 5U];
            uint16_t idx = 0;
            crcBuf[idx++] = s_rxParser.version;
            crcBuf[idx++] = (uint8_t)(s_rxParser.payloadLen & 0xFF);
            crcBuf[idx++] = (uint8_t)((s_rxParser.payloadLen >> 8) & 0xFF);
            crcBuf[idx++] = s_rxParser.msgId;
            crcBuf[idx++] = s_rxParser.seq;
            if (s_rxParser.payloadLen > 0) {
                memcpy(&crcBuf[idx], s_rxParser.payload, s_rxParser.payloadLen);
                idx += s_rxParser.payloadLen;
            }
            uint16_t calc = crc16_ccitt(crcBuf, idx);
            if (calc == s_rxParser.rxCrc && s_rxParser.version == USB_PROTOCOL_VERSION) {
                USBCommTask_DispatchFrame(s_rxParser.msgId,
                                          s_rxParser.seq,
                                          s_rxParser.payload,
                                          s_rxParser.payloadLen);
            }
            USBCommTask_ResetParser();
            break;
        }
        default:
            USBCommTask_ResetParser();
            break;
    }
}

static void USBCommTask_DispatchFrame(uint8_t msgId, uint8_t seq,
                                      const uint8_t *payload, uint16_t len)
{
    (void)seq;
    switch (msgId) {
        case USB_MSG_CONTROL_CMD:
            USBCommTask_HandleControlCmd(seq, payload, len);
            break;
        default:
            break;
    }
}

/* ========================== 控制命令处理 ========================== */
static void USBCommTask_HandleControlCmd(uint8_t seq,
                                         const uint8_t *payload,
                                         uint16_t len)
{
    bool needAck = (len > 13U) ? (payload[13] != 0U) : false;

    if (payload == NULL || len < CONTROL_CMD_MIN_PAYLOAD) {
        if (needAck) {
            USBCommTask_SendAck(ACK_STATUS_FAIL, 0x01);
        }
        return;
    }

    ControlCommandState_t nextCtrl = s_ctrlState;
    memcpy(&nextCtrl.leftSpeedMs,  &payload[0], 4);
    memcpy(&nextCtrl.rightSpeedMs, &payload[4], 4);
    nextCtrl.workMode       = USBCommTask_ToWorkMode(payload[8]);
    nextCtrl.brushLeftLevel = payload[9];
    nextCtrl.brushRightLevel= payload[10];
    nextCtrl.fanLevel       = payload[11];
    nextCtrl.waterLevel     = payload[12];
    nextCtrl.ackRequired    = needAck;
    nextCtrl.cmdSeq         = seq;

    s_ctrlState = nextCtrl;
    USBCommTask_ApplyControl(&s_ctrlState);

    if (needAck) {
        USBCommTask_SendAck(ACK_STATUS_OK, 0x00);
    }
}

static BrushMotorLevel_t USBCommTask_ToBrushLevel(uint8_t level)
{
    switch (level) {
        case 1:
            return BRUSH_MOTOR_LEVEL_LOW;
        case 2:
        case 3:
            return BRUSH_MOTOR_LEVEL_HIGH;
        case 0:
        default:
            return BRUSH_MOTOR_LEVEL_OFF;
    }
}

static PumpMotorLevel_t USBCommTask_ToPumpLevel(uint8_t level)
{
    switch (level) {
        case 1:  return PUMP_MOTOR_LEVEL_LOW;
        case 2:  return PUMP_MOTOR_LEVEL_MEDIUM;
        case 3:  return PUMP_MOTOR_LEVEL_HIGH;
        case 4:  return PUMP_MOTOR_LEVEL_TURBO;
        case 5:  return PUMP_MOTOR_LEVEL_ULTRA;
        default: return PUMP_MOTOR_LEVEL_OFF;
    }
}

static FanMotorLevel_t USBCommTask_ToFanLevel(uint8_t level)
{
    if (level > FAN_MOTOR_LEVEL_5) {
        level = FAN_MOTOR_LEVEL_5;
    }
    return (FanMotorLevel_t)level;
}

static WorkMode_t USBCommTask_ToWorkMode(uint8_t mode)
{
    if (mode > WORK_MODE_DOCK) {
        mode = WORK_MODE_IDLE;
    }
    return (WorkMode_t)mode;
}

static void USBCommTask_ApplyControl(const ControlCommandState_t *ctrl)
{
    if (ctrl == NULL) return;

    /* 挡位控制 */
    MotorCtrlTask_SetFanMotor(USBCommTask_ToFanLevel(ctrl->fanLevel));
    MotorCtrlTask_SetPumpMotor(USBCommTask_ToPumpLevel(ctrl->waterLevel));
    MotorCtrlTask_SetBrushMotor(
        USBCommTask_ToBrushLevel(ctrl->brushLeftLevel),
        USBCommTask_ToBrushLevel(ctrl->brushRightLevel));

    /* 轮速控制 */
    if (ctrl->workMode == WORK_MODE_DOCK) {
        if (g_pCleanBotApp != NULL) {
            IRHoming_t *homing = &g_pCleanBotApp->irHoming;
            if (homing != NULL && !IRHoming_IsDocked(homing)) {
                if (IRHoming_GetState(homing) == HOMING_STATE_IDLE) {
                    IRHoming_Start(homing, 0);
                }
            }
        }
        /* 轮速由IRHoming_Process处理，USB侧不再写速度 */
    } else {
        if (g_pCleanBotApp != NULL) {
            IRHoming_t *homing = &g_pCleanBotApp->irHoming;
            if (homing != NULL ) {
                IRHoming_Stop(homing);
            }
        }
        MotorCtrlTask_SetWheelSpeed(ctrl->leftSpeedMs, ctrl->rightSpeedMs);
    }

    s_usbSafeStopped = false;
}

static void USBCommTask_SendAck(AckStatus_t status, uint8_t info)
{
    uint8_t payload[3] = { USB_MSG_CONTROL_CMD, (uint8_t)status, info };
    USBCommTask_SendFrame(USB_MSG_ACK_REPLY, payload, sizeof(payload));
}

/* ========================== 遥测构建 ========================== */
static void USBCommTask_SendWheelTelemetry(void)
{
    if (g_pCleanBotApp == NULL) return;

    float wheelData[4];
    wheelData[0] = RAD_TO_DEG(Encoder_GetAngle(&g_pCleanBotApp->encoderWheelLeft));
    wheelData[1] = Encoder_GetSpeedMs(&g_pCleanBotApp->encoderWheelLeft);
    wheelData[2] = RAD_TO_DEG(Encoder_GetAngle(&g_pCleanBotApp->encoderWheelRight));
    wheelData[3] = Encoder_GetSpeedMs(&g_pCleanBotApp->encoderWheelRight);

    USBCommTask_SendFrame(USB_MSG_WHEEL_FEEDBACK,
                          (uint8_t*)wheelData,
                          sizeof(wheelData));
}
float imuData[9];
static void USBCommTask_SendImuTelemetry(void)
{
    float ax, ay, az;
    float gx, gy, gz;
    float roll, pitch, yaw;

    IMUTask_GetAccel(&ax, &ay, &az);
    IMUTask_GetGyro(&gx, &gy, &gz);
    IMUTask_GetEuler(&roll, &pitch, &yaw);

    
    imuData[0] = ax * G_TO_M_S2;
    imuData[1] = ay * G_TO_M_S2;
    imuData[2] = az * G_TO_M_S2;
    imuData[3] = DEG_TO_RAD(gx);
    imuData[4] = DEG_TO_RAD(gy);
    imuData[5] = DEG_TO_RAD(gz);
    imuData[6] = DEG_TO_RAD(roll);
    imuData[7] = DEG_TO_RAD(pitch);
    imuData[8] = DEG_TO_RAD(yaw);

    USBCommTask_SendFrame(USB_MSG_IMU_FEEDBACK,
                          (uint8_t*)imuData,
                          sizeof(imuData));
}

static uint8_t USBCommTask_GetDockStatus(void)
{
    if (g_pCleanBotApp == NULL) return 0;

    IRHoming_t *homing = &g_pCleanBotApp->irHoming;
    if (homing == NULL) return 0;

    HomingState_t state = IRHoming_GetState(homing);
    switch (state) {
        case HOMING_STATE_IDLE:
            return 0;
        case HOMING_STATE_SEARCHING:
        case HOMING_STATE_APPROACHING:
        case HOMING_STATE_ALIGNING:
        case HOMING_STATE_DOCKING:
            return 1;
        case HOMING_STATE_DOCKED:
            return 2;
        case HOMING_STATE_FAILED:
        case HOMING_STATE_TIMEOUT:
        
            return 3;
        default:
            return 0;
    }
}

static void USBCommTask_SendSensorTelemetry(void)
{
    if (g_pCleanBotApp == NULL) return;

    uint8_t payload[9] = {0};
    uint8_t faultFlags = 0;

    bool bumperLeft = PhotoGate_IsBlocked(&g_pCleanBotApp->photoGateLeft);
    bool bumperRight = PhotoGate_IsBlocked(&g_pCleanBotApp->photoGateRight);

    payload[0] = bumperLeft ? 1U : 0U;
    payload[1] = bumperRight ? 1U : 0U;
    payload[2] = g_pCleanBotApp->underLeftSuspended ? 1U : 0U;
    payload[3] = g_pCleanBotApp->underCenterSuspended ? 1U : 0U;
    payload[4] = g_pCleanBotApp->underRightSuspended ? 1U : 0U;

    if (!USB_Comm_IsConnected(&g_pCleanBotApp->usbComm)) {
        faultFlags |= FAULT_FLAG_USB_LOSS;
    }
    if (bumperLeft) {
        faultFlags |= FAULT_FLAG_BUMPER_LEFT;
    }
    if (bumperRight) {
        faultFlags |= FAULT_FLAG_BUMPER_RIGHT;
    }
    if (g_pCleanBotApp->underLeftSuspended ||
        g_pCleanBotApp->underCenterSuspended ||
        g_pCleanBotApp->underRightSuspended) {
        faultFlags |= FAULT_FLAG_CLIFF;
    }
    if (USBCommTask_GetDockStatus() == 3U) {
        faultFlags |= FAULT_FLAG_DOCK_FAILED;
    }

    payload[5] = faultFlags;
    payload[6] = s_heartbeatCounter++;
    payload[7] = USBCommTask_GetDockStatus();
    payload[8] = 0; /* reserved */
   
    USBCommTask_SendFrame(USB_MSG_SENSOR_STATUS, payload, sizeof(payload));
}

/* ========================== 数据接收 ========================== */
static void USBCommTask_ProcessRxStream(void)
{
    if (g_pCleanBotApp == NULL) return;

    uint8_t buffer[64];
    uint32_t received;

    while ((received = USB_Comm_Receive(&g_pCleanBotApp->usbComm,
                                        buffer,
                                        sizeof(buffer))) > 0) {
        for (uint32_t i = 0; i < received; ++i) {
            USBCommTask_ProcessByte(buffer[i]);
        }
    }
}

/* ========================== 安全/连接管理 ========================== */
static void USBCommTask_SafeStop(void)
{
    if (s_usbSafeStopped) {
        return;
    }
    MotorCtrlTask_SetWheelSpeed(0.0f, 0.0f);
    MotorCtrlTask_SetBrushMotor(BRUSH_MOTOR_LEVEL_OFF, BRUSH_MOTOR_LEVEL_OFF);
    MotorCtrlTask_SetPumpMotor(PUMP_MOTOR_LEVEL_OFF);
    MotorCtrlTask_SetFanMotor(FAN_MOTOR_LEVEL_OFF);
    if (g_pCleanBotApp != NULL) {
        IRHoming_Stop(&g_pCleanBotApp->irHoming);
    }
    s_usbSafeStopped = true;
}

static void USBCommTask_HandleConnection(void)
{
    if (g_pCleanBotApp == NULL) return;

    USB_Comm_UpdateConnectionState(&g_pCleanBotApp->usbComm);
    bool connected = USB_Comm_IsConnected(&g_pCleanBotApp->usbComm);
    if (connected != s_lastUsbConnected) {
        s_lastUsbConnected = connected;
        USBCommTask_UpdateLed(connected);
    }

    if (!connected && !USB_COMM_DEBUG_MODE) {
        USBCommTask_SafeStop();
    }
}

/* ========================== 任务入口 ========================== */
void USBCommTask_Init(void)
{
    memset(&s_ctrlState, 0, sizeof(s_ctrlState));
    memset(&s_seqState, 0, sizeof(s_seqState));
    s_ctrlState.workMode = WORK_MODE_IDLE;
    USBCommTask_ResetParser();
    s_lastWheelTick = osKernelGetTickCount();
    s_lastImuTick = s_lastWheelTick;
    s_lastSensorTick = s_lastWheelTick;
    s_lastConnPollTick = s_lastWheelTick;

    if (g_pCleanBotApp != NULL) {
        USB_Comm_UpdateConnectionState(&g_pCleanBotApp->usbComm);
        s_lastUsbConnected = USB_Comm_IsConnected(&g_pCleanBotApp->usbComm);
        USBCommTask_UpdateLed(s_lastUsbConnected);
    } else {
        s_lastUsbConnected = false;
        USBCommTask_UpdateLed(false);
    }
}

void USBCommTask_Run(void *argument)
{
    (void)argument;
    USBCommTask_Init();

    while (1) {
        if (g_pCleanBotApp == NULL) {
            osDelay(50);
            continue;
        }

        USBCommTask_ProcessRxStream();

        uint32_t now = osKernelGetTickCount();
        if ((now - s_lastWheelTick) >= PERIOD_WHEEL_MS) {
            s_lastWheelTick = now;
            USBCommTask_SendWheelTelemetry();
        }
        if ((now - s_lastImuTick) >= PERIOD_IMU_MS) {
            s_lastImuTick = now;
            USBCommTask_SendImuTelemetry();
        }
        if ((now - s_lastSensorTick) >= PERIOD_SENSOR_MS) {
            s_lastSensorTick = now;
            USBCommTask_SendSensorTelemetry();
        }
        if ((now - s_lastConnPollTick) >= CONNECTION_POLL_MS) {
            s_lastConnPollTick = now;
            USBCommTask_HandleConnection();
        }

        osDelay(1);
    }
}
