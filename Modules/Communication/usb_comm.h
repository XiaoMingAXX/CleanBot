/**
  ******************************************************************************
  * @file    usb_comm.h
  * @brief   USB通信模块头文件（USB CDC虚拟串口）
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __USB_COMM_H__
#define __USB_COMM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "usb_device.h"
#include "usbd_cdc_if.h"
#include "ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>

/* USB通信配置 */
#define USB_COMM_RX_BUFFER_SIZE     512  /* 接收缓冲区大小 */
#define USB_COMM_TX_BUFFER_SIZE     512  /* 发送缓冲区大小 */

/* USB通信结构体 */
typedef struct {
    RingBuffer_t rxBuffer;         /* 接收缓冲区 */
    RingBuffer_t txBuffer;         /* 发送缓冲区 */
    uint8_t rxData[USB_COMM_RX_BUFFER_SIZE];  /* 接收数据缓冲区 */
    uint8_t txData[USB_COMM_TX_BUFFER_SIZE];  /* 发送数据缓冲区 */
    bool connected;                /* USB连接状态 */
    bool enabled;                  /* 使能标志 */
} USB_Comm_t;

/* 函数声明 */
void USB_Comm_Init(USB_Comm_t *comm);
void USB_Comm_Enable(USB_Comm_t *comm);
void USB_Comm_Disable(USB_Comm_t *comm);
uint32_t USB_Comm_Send(USB_Comm_t *comm, const uint8_t *data, uint32_t len);
uint32_t USB_Comm_Receive(USB_Comm_t *comm, uint8_t *data, uint32_t len);
uint32_t USB_Comm_GetRxCount(USB_Comm_t *comm);
uint32_t USB_Comm_GetTxFree(USB_Comm_t *comm);
bool USB_Comm_IsConnected(USB_Comm_t *comm);
void USB_Comm_SetConnected(USB_Comm_t *comm, bool connected);  /* 设置连接状态 */
void USB_Comm_UpdateConnectionState(USB_Comm_t *comm);  /* 更新连接状态（通过检查USB设备状态） */
void USB_Comm_RxCpltCallback(USB_Comm_t *comm, uint8_t *buf, uint32_t len);  /* 接收完成回调 */
void USB_Comm_TxCpltCallback(USB_Comm_t *comm);  /* 发送完成回调 */

#ifdef __cplusplus
}
#endif

#endif /* __USB_COMM_H__ */

