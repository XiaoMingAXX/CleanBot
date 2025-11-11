/**
  ******************************************************************************
  * @file    usb_comm_task.c
  * @brief   USB通信任务实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "usb_comm_task.h"
#include "usb_comm.h"
#include "motor_ctrl_task.h"
#include "CleanBotApp.h"
#include "led.h"
#include "cmsis_os.h"
#include <string.h>

/* 外部应用对象 */
extern CleanBotApp_t *g_pCleanBotApp;

/* 通信协议帧头 */
#define PROTOCOL_HEADER          0xAA
#define PROTOCOL_TAIL            0x55

/* 命令类型 */
typedef enum {
    CMD_SET_WHEEL_SPEED = 0x01,      /* 设置轮电机速度 */
    CMD_SET_BRUSH_MOTOR = 0x02,      /* 设置边刷电机 */
    CMD_SET_PUMP_MOTOR = 0x03,       /* 设置水泵电机 */
    CMD_SET_FAN_MOTOR = 0x04,        /* 设置风机 */
    CMD_GET_WHEEL_SPEED = 0x05,      /* 获取轮电机速度 */
    CMD_GET_STATUS = 0x06,           /* 获取状态 */
    CMD_RESET = 0x07,                /* 复位 */
    CMD_ACK = 0x80,                  /* 应答 */
    CMD_NACK = 0x81                  /* 错误应答 */
} CommandType_t;

/* 接收缓冲区 */
#define RX_BUFFER_SIZE           256
static uint8_t rxBuffer[RX_BUFFER_SIZE];
static uint32_t rxBufferIndex = 0;

/* 连接状态检查周期（任务循环次数，每20ms一次，5次=100ms） */
#define CONNECTION_CHECK_INTERVAL    5
static uint32_t connectionCheckCounter = 0;

/**
 * @brief  计算校验和
 */
static uint8_t USBCommTask_CalculateChecksum(uint8_t *data, uint32_t len)
{
    uint8_t checksum = 0;
    for (uint32_t i = 0; i < len; i++) {
        checksum ^= data[i];
    }
    return checksum;
}

/**
 * @brief  发送数据包
 */
static void USBCommTask_SendPacket(CommandType_t cmd, uint8_t *data, uint32_t len)
{
    uint8_t packet[64];
    uint32_t packetIndex = 0;
    
    /* 帧头 */
    packet[packetIndex++] = PROTOCOL_HEADER;
    
    /* 命令类型 */
    packet[packetIndex++] = (uint8_t)cmd;
    
    /* 数据长度 */
    packet[packetIndex++] = (uint8_t)len;
    
    /* 数据 */
    if (data != NULL && len > 0) {
        memcpy(&packet[packetIndex], data, len);
        packetIndex += len;
    }
    
    /* 校验和 */
    uint8_t checksum = USBCommTask_CalculateChecksum(&packet[1], packetIndex - 1);
    packet[packetIndex++] = checksum;
    
    /* 帧尾 */
    packet[packetIndex++] = PROTOCOL_TAIL;
    
    /* 发送 */
    if (g_pCleanBotApp != NULL) {
        USB_Comm_Send(&g_pCleanBotApp->usbComm, packet, packetIndex);
    }
}

/**
 * @brief  处理设置轮电机速度命令
 */
static void USBCommTask_HandleSetWheelSpeed(uint8_t *data, uint32_t len)
{
    if (len < 8) {
        USBCommTask_SendPacket(CMD_NACK, NULL, 0);
        return;
    }
    
    /* 解析数据：4字节左速度(float) + 4字节右速度(float) */
    float leftSpeedMs, rightSpeedMs;
    memcpy(&leftSpeedMs, &data[0], 4);
    memcpy(&rightSpeedMs, &data[4], 4);
    
    /* 设置速度 */
    MotorCtrlTask_SetWheelSpeed(leftSpeedMs, rightSpeedMs);
    
    /* 发送应答 */
    USBCommTask_SendPacket(CMD_ACK, NULL, 0);
}

/**
 * @brief  处理获取轮电机速度命令
 */
static void USBCommTask_HandleGetWheelSpeed(void)
{
    float leftSpeedMs, rightSpeedMs;
    MotorCtrlTask_GetWheelSpeed(&leftSpeedMs, &rightSpeedMs);
    
    uint8_t data[8];
    memcpy(&data[0], &leftSpeedMs, 4);
    memcpy(&data[4], &rightSpeedMs, 4);
    
    USBCommTask_SendPacket(CMD_GET_WHEEL_SPEED, data, 8);
}

/**
 * @brief  处理设置边刷电机命令
 */
static void USBCommTask_HandleSetBrushMotor(uint8_t *data, uint32_t len)
{
    if (len < 2) {
        USBCommTask_SendPacket(CMD_NACK, NULL, 0);
        return;
    }
    
    BrushMotorLevel_t left = (BrushMotorLevel_t)data[0];
    BrushMotorLevel_t right = (BrushMotorLevel_t)data[1];
    
    MotorCtrlTask_SetBrushMotor(left, right);
    USBCommTask_SendPacket(CMD_ACK, NULL, 0);
}

/**
 * @brief  处理设置水泵电机命令
 */
static void USBCommTask_HandleSetPumpMotor(uint8_t *data, uint32_t len)
{
    if (len < 1) {
        USBCommTask_SendPacket(CMD_NACK, NULL, 0);
        return;
    }
    
    PumpMotorLevel_t level = (PumpMotorLevel_t)data[0];
    MotorCtrlTask_SetPumpMotor(level);
    USBCommTask_SendPacket(CMD_ACK, NULL, 0);
}

/**
 * @brief  处理设置风机命令
 */
static void USBCommTask_HandleSetFanMotor(uint8_t *data, uint32_t len)
{
    if (len < 1) {
        USBCommTask_SendPacket(CMD_NACK, NULL, 0);
        return;
    }
    
    FanMotorLevel_t level = (FanMotorLevel_t)data[0];
    MotorCtrlTask_SetFanMotor(level);
    USBCommTask_SendPacket(CMD_ACK, NULL, 0);
}

/**
 * @brief  处理接收到的数据包
 */
static void USBCommTask_ProcessPacket(uint8_t *packet, uint32_t len)
{
    if (len < 5) return;  /* 最小包长度：头+命令+长度+校验+尾 */
    
    /* 检查帧头帧尾 */
    if (packet[0] != PROTOCOL_HEADER || packet[len - 1] != PROTOCOL_TAIL) {
        return;
    }
    
    /* 提取命令和数据 */
    CommandType_t cmd = (CommandType_t)packet[1];
    uint32_t dataLen = packet[2];
    uint8_t *data = &packet[3];
    uint8_t checksum = packet[3 + dataLen];
    
    /* 校验校验和 */
    uint8_t calcChecksum = USBCommTask_CalculateChecksum(&packet[1], 2 + dataLen);
    if (calcChecksum != checksum) {
        USBCommTask_SendPacket(CMD_NACK, NULL, 0);
        return;
    }
    
    /* 处理命令 */
    switch (cmd) {
        case CMD_SET_WHEEL_SPEED:
            USBCommTask_HandleSetWheelSpeed(data, dataLen);
            break;
        case CMD_GET_WHEEL_SPEED:
            USBCommTask_HandleGetWheelSpeed();
            break;
        case CMD_SET_BRUSH_MOTOR:
            USBCommTask_HandleSetBrushMotor(data, dataLen);
            break;
        case CMD_SET_PUMP_MOTOR:
            USBCommTask_HandleSetPumpMotor(data, dataLen);
            break;
        case CMD_SET_FAN_MOTOR:
            USBCommTask_HandleSetFanMotor(data, dataLen);
            break;
        default:
            USBCommTask_SendPacket(CMD_NACK, NULL, 0);
            break;
    }
}

/**
 * @brief  初始化USB通信任务
 */
void USBCommTask_Init(void)
{
    rxBufferIndex = 0;
    memset(rxBuffer, 0, RX_BUFFER_SIZE);
}

/**
 * @brief  USB通信任务主函数
 */
void USBCommTask_Run(void *argument)
{
    USBCommTask_Init();
    connectionCheckCounter = 0;
    bool lastConnectedState = false;
    
    /* 初始化时检查连接状态并设置LED4 */
    if (g_pCleanBotApp != NULL) {
        USB_Comm_UpdateConnectionState(&g_pCleanBotApp->usbComm);
        lastConnectedState = USB_Comm_IsConnected(&g_pCleanBotApp->usbComm);
        /* 初始化LED4状态：未连接时常亮，连接时关闭 */
        if (lastConnectedState) {
            LED_Off(&g_pCleanBotApp->led4);
        } else {
            LED_On(&g_pCleanBotApp->led4);
        }
    }
    
    while (1) {
        if (g_pCleanBotApp == NULL) {
            osDelay(100);
            continue;
        }
        
        /* 周期性检查连接状态（用于自动检测重连） */
        connectionCheckCounter++;
        if (connectionCheckCounter >= CONNECTION_CHECK_INTERVAL) {
            connectionCheckCounter = 0;
            
            /* 更新连接状态 */
            USB_Comm_UpdateConnectionState(&g_pCleanBotApp->usbComm);
            
            /* 检查连接状态是否变化，更新LED4 */
            bool currentConnectedState = USB_Comm_IsConnected(&g_pCleanBotApp->usbComm);
            if (currentConnectedState != lastConnectedState) {
                lastConnectedState = currentConnectedState;
                
                /* LED4指示USB连接状态：未连接时常亮，连接时关闭 */
                if (currentConnectedState) {
                    /* 已连接：关闭LED4 */
                    LED_Off(&g_pCleanBotApp->led4);
                } else {
                    /* 未连接：LED4常亮 */
                    LED_On(&g_pCleanBotApp->led4);
                }
            }
        }
        
        /* 接收数据 */
        uint32_t received = USB_Comm_Receive(&g_pCleanBotApp->usbComm, 
                                             &rxBuffer[rxBufferIndex], 
                                             RX_BUFFER_SIZE - rxBufferIndex);
        
        if (received > 0) {
            rxBufferIndex += received;
            
            /* 查找完整的数据包 */
            for (uint32_t i = 0; i < rxBufferIndex; i++) {
                if (rxBuffer[i] == PROTOCOL_HEADER) {
                    /* 查找帧尾 */
                    for (uint32_t j = i + 1; j < rxBufferIndex; j++) {
                        if (rxBuffer[j] == PROTOCOL_TAIL) {
                            /* 找到完整数据包 */
                            uint32_t packetLen = j - i + 1;
                            USBCommTask_ProcessPacket(&rxBuffer[i], packetLen);
                            
                            /* 移除已处理的数据 */
                            memmove(&rxBuffer[0], &rxBuffer[j + 1], rxBufferIndex - j - 1);
                            rxBufferIndex -= packetLen;
                            i = 0;  /* 重新开始查找 */
                            break;
                        }
                    }
                }
            }
            
            /* 防止缓冲区溢出 */
            if (rxBufferIndex >= RX_BUFFER_SIZE - 10) {
                rxBufferIndex = 0;  /* 清空缓冲区 */
            }
        }
        USBCommTask_SendPacket(CMD_ACK, NULL, 0);
        osDelay(20);
    }
}

