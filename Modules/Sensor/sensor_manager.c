/**
  ******************************************************************************
  * @file    sensor_manager.c
  * @brief   传感器管理模块实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "sensor_manager.h"
#include "ir_sensor.h"
#include "photo_gate.h"
#include "main.h"
#include "cmsis_os.h"

/* 全局传感器管理器实例 */
static SensorManager_t g_SensorManager;

/* 按钮配置 */
#define BUTTON_CLICK_TIMEOUT_MS     500     /* 单击超时时间 (ms) */
#define BUTTON_DOUBLE_CLICK_GAP_MS  300     /* 双击间隔时间 (ms) */
#define BUTTON_DEBOUNCE_TIME_MS     10      /* 按钮滤波时间 (ms) */

/* 中断回调中使用的静态变量（用于边沿检测） */
static struct {
    uint32_t lastEdgeTime[4];   /* 红外传感器上次边沿时间 */
    bool lastLevel[4];          /* 红外传感器上次电平 */
} irSensorState;

/**
 * @brief  初始化传感器管理器
 */
void SensorManager_Init(SensorManager_t *manager)
{
    if (manager == NULL) return;
    
    /* 创建事件队列 */
    manager->eventQueue = xQueueCreate(20, sizeof(SensorEvent_t));
    
    /* 初始化按钮状态 */
    manager->button1.pressTime = 0;
    manager->button1.releaseTime = 0;
    manager->button1.isPressed = false;
    manager->button1.lastState = true;  /* 默认高电平（未按下） */
    manager->button1.clickCount = 0;
    manager->button1.lastClickTime = 0;
    manager->button1.debouncePending = false;
    manager->button1.debounceTime = 0;
    manager->button1.debounceState = false;
    
    manager->button2.pressTime = 0;
    manager->button2.releaseTime = 0;
    manager->button2.isPressed = false;
    manager->button2.lastState = true;
    manager->button2.clickCount = 0;
    manager->button2.lastClickTime = 0;
    manager->button2.debouncePending = false;
    manager->button2.debounceTime = 0;
    manager->button2.debounceState = false;
    
    /* 初始化传感器状态 */
    manager->photoGateLeftBlocked = false;
    manager->photoGateRightBlocked = false;
    
    /* 初始化下视传感器状态 */
    manager->underLeftSuspended = false;
    manager->underRightSuspended = false;
    manager->underCenterSuspended = false;
    
    /* 初始化红外传感器状态 */
    for (int i = 0; i < 4; i++) {
        manager->irSensors[i].dataReady = false;
        irSensorState.lastEdgeTime[i] = 0;
        irSensorState.lastLevel[i] = false;
    }
    
    manager->enabled = false;
}

/**
 * @brief  启动传感器管理器
 */
void SensorManager_Start(SensorManager_t *manager)
{
    if (manager == NULL) return;
    manager->enabled = true;
}

/**
 * @brief  停止传感器管理器
 */
void SensorManager_Stop(SensorManager_t *manager)
{
    if (manager == NULL) return;
    manager->enabled = false;
}

/**
 * @brief  红外传感器中断处理（左侧）
 */
void SensorManager_IRQHandler_IR_Left(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    bool currentLevel = HAL_GPIO_ReadPin(L_RECEIVE_GPIO_Port, L_RECEIVE_Pin) == GPIO_PIN_SET;
    uint32_t currentTime = HAL_GetTick() * 1000;  /* 转换为微秒（近似） */
    
    /* 检测边沿 */
    if (currentLevel != irSensorState.lastLevel[0]) {
        uint32_t period = currentTime - irSensorState.lastEdgeTime[0];
        
        /* 发送边沿事件到队列（在任务中处理NEC解码） */
        event.type = SENSOR_EVENT_IR_LEFT;
        event.timestamp = currentTime;
        event.data = (currentLevel ? 1 : 0) | (period << 1);  /* 临时存储边沿信息 */
        
        xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
        
        irSensorState.lastEdgeTime[0] = currentTime;
        irSensorState.lastLevel[0] = currentLevel;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  红外传感器中断处理（右侧）
 */
void SensorManager_IRQHandler_IR_Right(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    bool currentLevel = HAL_GPIO_ReadPin(R_RECEIVE_GPIO_Port, R_RECEIVE_Pin) == GPIO_PIN_SET;
    uint32_t currentTime = HAL_GetTick() * 1000;
    
    if (currentLevel != irSensorState.lastLevel[1]) {
        uint32_t period = currentTime - irSensorState.lastEdgeTime[1];
        
        event.type = SENSOR_EVENT_IR_RIGHT;
        event.timestamp = currentTime;
        event.data = (currentLevel ? 1 : 0) | (period << 1);
        
        xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
        
        irSensorState.lastEdgeTime[1] = currentTime;
        irSensorState.lastLevel[1] = currentLevel;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  红外传感器中断处理（左前）
 */
void SensorManager_IRQHandler_IR_FrontLeft(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    bool currentLevel = HAL_GPIO_ReadPin(L_FOLLOW_CHECK_SIGNAL_GPIO_Port, L_FOLLOW_CHECK_SIGNAL_Pin) == GPIO_PIN_SET;
    uint32_t currentTime = HAL_GetTick() * 1000;
    
    if (currentLevel != irSensorState.lastLevel[2]) {
        uint32_t period = currentTime - irSensorState.lastEdgeTime[2];
        
        event.type = SENSOR_EVENT_IR_FRONT_LEFT;
        event.timestamp = currentTime;
        event.data = (currentLevel ? 1 : 0) | (period << 1);
        
        xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
        
        irSensorState.lastEdgeTime[2] = currentTime;
        irSensorState.lastLevel[2] = currentLevel;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  红外传感器中断处理（右前）
 */
void SensorManager_IRQHandler_IR_FrontRight(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    bool currentLevel = HAL_GPIO_ReadPin(R_FOLLOW_CHECK_SIGNAL_GPIO_Port, R_FOLLOW_CHECK_SIGNAL_Pin) == GPIO_PIN_SET;
    uint32_t currentTime = HAL_GetTick() * 1000;
    
    if (currentLevel != irSensorState.lastLevel[3]) {
        uint32_t period = currentTime - irSensorState.lastEdgeTime[3];
        
        event.type = SENSOR_EVENT_IR_FRONT_RIGHT;
        event.timestamp = currentTime;
        event.data = (currentLevel ? 1 : 0) | (period << 1);
        
        xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
        
        irSensorState.lastEdgeTime[3] = currentTime;
        irSensorState.lastLevel[3] = currentLevel;
    }
    
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  光电门中断处理（左侧）
 */
void SensorManager_IRQHandler_PhotoGate_Left(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    /* 读取GPIO状态（高电平表示碰撞） */
    bool isBlocked = HAL_GPIO_ReadPin(IFHIT_L_GPIO_Port, IFHIT_L_Pin) == GPIO_PIN_SET;
    
    event.type = SENSOR_EVENT_PHOTO_GATE_LEFT;
    event.timestamp = HAL_GetTick();
    event.data = isBlocked ? 1 : 0;
    
    xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  光电门中断处理（右侧）
 */
void SensorManager_IRQHandler_PhotoGate_Right(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    bool isBlocked = HAL_GPIO_ReadPin(IFHIT_R_GPIO_Port, IFHIT_R_Pin) == GPIO_PIN_SET;
    
    event.type = SENSOR_EVENT_PHOTO_GATE_RIGHT;
    event.timestamp = HAL_GetTick();
    event.data = isBlocked ? 1 : 0;
    
    xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  按钮1中断处理（带10ms滤波检测）
 */
void SensorManager_IRQHandler_Button1(void)
{
    /* 读取GPIO状态（低电平表示按下） */
    bool isPressed = HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_RESET;
    
    /* 记录滤波检测信息 */
    g_SensorManager.button1.debouncePending = true;
    g_SensorManager.button1.debounceTime = HAL_GetTick();
    g_SensorManager.button1.debounceState = isPressed;
}

/**
 * @brief  按钮2中断处理（带10ms滤波检测）
 */
void SensorManager_IRQHandler_Button2(void)
{
    /* 读取GPIO状态（低电平表示按下） */
    bool isPressed = HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin) == GPIO_PIN_RESET;
    
    /* 记录滤波检测信息 */
    g_SensorManager.button2.debouncePending = true;
    g_SensorManager.button2.debounceTime = HAL_GetTick();
    g_SensorManager.button2.debounceState = isPressed;
}

/**
 * @brief  按钮滤波检测（在Sensor任务中定期调用）
 */
void SensorManager_CheckButtonDebounce(SensorManager_t *manager)
{
    if (manager == NULL) return;
    
    uint32_t currentTime = HAL_GetTick();
    SensorEvent_t event;
    
    /* 检查按钮1滤波 */
    if (manager->button1.debouncePending) {
        /* 检查是否已经过了滤波时间 */
        if ((currentTime - manager->button1.debounceTime) >= BUTTON_DEBOUNCE_TIME_MS) {
            /* 再次读取GPIO状态确认 */
            bool isPressed = HAL_GPIO_ReadPin(BUTTON1_GPIO_Port, BUTTON1_Pin) == GPIO_PIN_RESET;
            
            /* 如果状态与记录的一致，说明是有效事件 */
            if (isPressed == manager->button1.debounceState) {
                if (isPressed && !manager->button1.lastState) {
                    /* 按下事件 */
                    event.type = SENSOR_EVENT_BUTTON1_PRESS;
                    event.timestamp = currentTime;
                    event.data = 0;
                    xQueueSend(manager->eventQueue, &event, 0);
                } else if (!isPressed && manager->button1.lastState) {
                    /* 释放事件 */
                    event.type = SENSOR_EVENT_BUTTON1_RELEASE;
                    event.timestamp = currentTime;
                    event.data = 0;
                    xQueueSend(manager->eventQueue, &event, 0);
                }
                
                manager->button1.lastState = isPressed;
            }
            
            /* 清除滤波待检测标志 */
            manager->button1.debouncePending = false;
        }
    }
    
    /* 检查按钮2滤波 */
    if (manager->button2.debouncePending) {
        /* 检查是否已经过了滤波时间 */
        if ((currentTime - manager->button2.debounceTime) >= BUTTON_DEBOUNCE_TIME_MS) {
            /* 再次读取GPIO状态确认 */
            bool isPressed = HAL_GPIO_ReadPin(BUTTON2_GPIO_Port, BUTTON2_Pin) == GPIO_PIN_RESET;
            
            /* 如果状态与记录的一致，说明是有效事件 */
            if (isPressed == manager->button2.debounceState) {
                if (isPressed && !manager->button2.lastState) {
                    /* 按下事件 */
                    event.type = SENSOR_EVENT_BUTTON2_PRESS;
                    event.timestamp = currentTime;
                    event.data = 0;
                    xQueueSend(manager->eventQueue, &event, 0);
                } else if (!isPressed && manager->button2.lastState) {
                    /* 释放事件 */
                    event.type = SENSOR_EVENT_BUTTON2_RELEASE;
                    event.timestamp = currentTime;
                    event.data = 0;
                    xQueueSend(manager->eventQueue, &event, 0);
                }
                
                manager->button2.lastState = isPressed;
            }
            
            /* 清除滤波待检测标志 */
            manager->button2.debouncePending = false;
        }
    }
}

/**
 * @brief  下视传感器中断处理（左前）
 */
void SensorManager_IRQHandler_UnderLeft(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    /* 读取GPIO状态（高电平表示悬空） */
    bool isSuspended = HAL_GPIO_ReadPin(L_FOLLOW_CHECK_SIGNAL_GPIO_Port, L_FOLLOW_CHECK_SIGNAL_Pin) == GPIO_PIN_SET;
    
    event.type = SENSOR_EVENT_UNDER_LEFT;
    event.timestamp = HAL_GetTick();
    event.data = isSuspended ? 1 : 0;  /* 1=悬空，0=地面 */
    
    /* 更新状态 */
    g_SensorManager.underLeftSuspended = isSuspended;
    
    xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  下视传感器中断处理（右前）
 */
void SensorManager_IRQHandler_UnderRight(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    /* 读取GPIO状态（高电平表示悬空） */
    bool isSuspended = HAL_GPIO_ReadPin(R_FOLLOW_CHECK_SIGNAL_GPIO_Port, R_FOLLOW_CHECK_SIGNAL_Pin) == GPIO_PIN_SET;
    
    event.type = SENSOR_EVENT_UNDER_RIGHT;
    event.timestamp = HAL_GetTick();
    event.data = isSuspended ? 1 : 0;  /* 1=悬空，0=地面 */
    
    /* 更新状态 */
    g_SensorManager.underRightSuspended = isSuspended;
    
    xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  下视传感器中断处理（中间）
 */
void SensorManager_IRQHandler_UnderCenter(void)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    SensorEvent_t event;
    
    /* 读取GPIO状态（高电平表示悬空） */
    bool isSuspended = HAL_GPIO_ReadPin(S_FOLLOW_CHECK_SIGNAL_GPIO_Port, S_FOLLOW_CHECK_SIGNAL_Pin) == GPIO_PIN_SET;
    
    event.type = SENSOR_EVENT_UNDER_CENTER;
    event.timestamp = HAL_GetTick();
    event.data = isSuspended ? 1 : 0;  /* 1=悬空，0=地面 */
    
    /* 更新状态 */
    g_SensorManager.underCenterSuspended = isSuspended;
    
    xQueueSendFromISR(g_SensorManager.eventQueue, &event, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/**
 * @brief  获取事件
 */
bool SensorManager_GetEvent(SensorManager_t *manager, SensorEvent_t *event, uint32_t timeout)
{
    if (manager == NULL || event == NULL) return false;
    
    if (xQueueReceive(manager->eventQueue, event, pdMS_TO_TICKS(timeout)) == pdTRUE) {
        return true;
    }
    return false;
}

/**
 * @brief  获取全局传感器管理器实例
 */
SensorManager_t* SensorManager_GetInstance(void)
{
    return &g_SensorManager;
}

