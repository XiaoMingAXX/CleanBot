/**
  ******************************************************************************
  * @file    usb_comm.c
  * @brief   USB通信模块实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "usb_comm.h"
#include "usbd_cdc_if.h"
#include "usb_device.h"
#include "cmsis_os.h"
#include <stddef.h>  /* 定义NULL */

static void USB_Comm_TryStartTx(USB_Comm_t *comm);
static inline uint32_t USB_Comm_EnterCritical(void)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    return primask;
}

static inline void USB_Comm_ExitCritical(uint32_t primask)
{
    __set_PRIMASK(primask);
}

/**
 * @brief  初始化USB通信
 * @param  comm: USB通信对象指针
 * @retval None
 */
void USB_Comm_Init(USB_Comm_t *comm)
{
    if (comm == NULL) return;
    
    RingBuffer_Init(&comm->rxBuffer, comm->rxData, USB_COMM_RX_BUFFER_SIZE);
    RingBuffer_Init(&comm->txBuffer, comm->txData, USB_COMM_TX_BUFFER_SIZE);
    comm->connected = false;
    comm->enabled = true;
    comm->txBusy = false;
}

/**
  * @brief  使能USB通信
  * @param  comm: USB通信对象指针
  * @retval None
  */
void USB_Comm_Enable(USB_Comm_t *comm)
{
    if (comm == NULL) return;
    comm->enabled = true;
}

/**
  * @brief  禁用USB通信
  * @param  comm: USB通信对象指针
  * @retval None
  */
void USB_Comm_Disable(USB_Comm_t *comm)
{
    if (comm == NULL) return;
    comm->enabled = false;
}

/**
  * @brief  发送数据
  * @param  comm: USB通信对象指针
  * @param  data: 数据指针
  * @param  len: 数据长度
  * @retval 实际发送的字节数
  */
uint32_t USB_Comm_Send(USB_Comm_t *comm, const uint8_t *data, uint32_t len)
{
    if (comm == NULL || data == NULL || len == 0 || !comm->enabled) return 0;
    
    if (!comm->connected) return 0;
    
    /* 将数据放入发送缓冲区 */
    uint32_t primask = USB_Comm_EnterCritical();
    uint32_t written = RingBuffer_PutData(&comm->txBuffer, data, len);
    USB_Comm_ExitCritical(primask);
    
    /* 尝试通过USB发送数据 */
    if (written > 0) {
        USB_Comm_TryStartTx(comm);
    }
    
    return written;
}

/**
  * @brief  接收数据
  * @param  comm: USB通信对象指针
  * @param  data: 数据缓冲区
  * @param  len: 缓冲区大小
  * @retval 实际接收的字节数
  */
uint32_t USB_Comm_Receive(USB_Comm_t *comm, uint8_t *data, uint32_t len)
{
    if (comm == NULL || data == NULL || len == 0 || !comm->enabled) return 0;
    
    return RingBuffer_GetData(&comm->rxBuffer, data, len);
}

/**
  * @brief  获取接收数据数量
  * @param  comm: USB通信对象指针
  * @retval 接收数据数量
  */
uint32_t USB_Comm_GetRxCount(USB_Comm_t *comm)
{
    if (comm == NULL) return 0;
    return RingBuffer_GetCount(&comm->rxBuffer);
}

/**
  * @brief  获取发送缓冲区空闲空间
  * @param  comm: USB通信对象指针
  * @retval 空闲空间大小
  */
uint32_t USB_Comm_GetTxFree(USB_Comm_t *comm)
{
    if (comm == NULL) return 0;
    return RingBuffer_GetFree(&comm->txBuffer);
}

/**
  * @brief  检查USB是否连接
  * @param  comm: USB通信对象指针
  * @retval 是否连接
  */
bool USB_Comm_IsConnected(USB_Comm_t *comm)
{
    if (comm == NULL) return false;
    return comm->connected;
}

/**
  * @brief  设置连接状态
  * @param  comm: USB通信对象指针
  * @param  connected: 连接状态
  * @retval None
  */
void USB_Comm_SetConnected(USB_Comm_t *comm, bool connected)
{
    if (comm == NULL) return;
    comm->connected = connected;
    if (!connected) {
        uint32_t primask = USB_Comm_EnterCritical();
        comm->txBusy = false;
        RingBuffer_Reset(&comm->txBuffer);
        USB_Comm_ExitCritical(primask);
    }
}

/**
  * @brief  更新连接状态（通过检查USB设备状态）
  * @param  comm: USB通信对象指针
  * @retval None
  */
void USB_Comm_UpdateConnectionState(USB_Comm_t *comm)
{
    if (comm == NULL) return;
    
    extern USBD_HandleTypeDef hUsbDeviceFS;
    
    /* 检查USB设备状态，如果状态为CONFIGURED，则表示已连接并配置完成 */
    /* 注意：USBD_STATE_CONFIGURED 表示设备已配置，这是真正可以通信的状态 */
    uint8_t devState = hUsbDeviceFS.dev_state;
    bool isConnected = (devState == USBD_STATE_CONFIGURED);
    
    /* 更新连接状态 */
    if (comm->connected != isConnected) {
        comm->connected = isConnected;
        
        /* 如果断开连接，清空发送缓冲区 */
        if (!isConnected) {
            uint32_t primask = USB_Comm_EnterCritical();
            RingBuffer_Reset(&comm->txBuffer);
            comm->txBusy = false;
            USB_Comm_ExitCritical(primask);
        }
    }
}

/**
  * @brief  接收完成回调（需要在USB CDC回调函数中调用）
  * @param  comm: USB通信对象指针
  * @param  buf: 接收缓冲区
  * @param  len: 接收长度
  * @retval None
  */
void USB_Comm_RxCpltCallback(USB_Comm_t *comm, uint8_t *buf, uint32_t len)
{
    if (comm == NULL || buf == NULL || len == 0 || !comm->enabled) return;
    
    /* 将接收到的数据放入接收缓冲区 */
    RingBuffer_PutData(&comm->rxBuffer, buf, len);
}

/**
  * @brief  发送完成回调（需要在USB CDC回调函数中调用）
  * @param  comm: USB通信对象指针
  * @retval None
  */
void USB_Comm_TxCpltCallback(USB_Comm_t *comm)
{
    if (comm == NULL || !comm->enabled) return;
    
    uint32_t primask = USB_Comm_EnterCritical();
    comm->txBusy = false;
    USB_Comm_ExitCritical(primask);
    USB_Comm_TryStartTx(comm);
}

/**
 * @brief  若USB端点空闲则启动一次发送
 */
static void USB_Comm_TryStartTx(USB_Comm_t *comm)
{
    if (comm == NULL || !comm->enabled || !comm->connected) return;
    
    extern USBD_HandleTypeDef hUsbDeviceFS;
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
    if (hcdc == NULL || hcdc->TxState != 0) {
        return;
    }
    
    uint32_t primask = USB_Comm_EnterCritical();
    if (comm->txBusy) {
        USB_Comm_ExitCritical(primask);
        return;
    }
    uint32_t sendLen = RingBuffer_GetData(&comm->txBuffer,
                                          comm->txPacket,
                                          USB_COMM_TX_PACKET_SIZE);
    if (sendLen == 0) {
        USB_Comm_ExitCritical(primask);
        return;
    }
    comm->txBusy = true;
    USB_Comm_ExitCritical(primask);
    
    if (CDC_Transmit_FS(comm->txPacket, (uint16_t)sendLen) != USBD_OK) {
        primask = USB_Comm_EnterCritical();
        comm->txBusy = false;
        /* 发送失败时将数据写回缓冲区前部，避免丢包 */
        for (int32_t i = (int32_t)sendLen - 1; i >= 0; --i) {
            RingBuffer_PutFront(&comm->txBuffer, comm->txPacket[i]);
        }
        USB_Comm_ExitCritical(primask);
    }
}

