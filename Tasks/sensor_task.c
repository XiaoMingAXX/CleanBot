/**
  ******************************************************************************
  * @file    sensor_task.c
  * @brief   传感器任务实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "sensor_task.h"
#include "sensor_manager.h"
#include "ir_sensor.h"
#include "photo_gate.h"
#include "led.h"
#include "CleanBotApp.h"
#include "cmsis_os.h"

/* 外部应用对象 */
extern CleanBotApp_t *g_pCleanBotApp;

/* 需要包含main.h获取GPIO定义 */
#include "main.h"

/* 按钮单击/双击识别状态 */
static struct {
    uint32_t pressTime;
    uint32_t releaseTime;
    bool waitingSecondClick;
    uint32_t firstClickTime;
} button1State, button2State;

/* LED闪烁状态 */
static struct {
    uint32_t blinkStartTime;
    uint8_t blinkCount;
    bool isBlinking;
} led3BlinkState;

#define BUTTON_CLICK_TIMEOUT_MS     500
#define BUTTON_DOUBLE_CLICK_GAP_MS  300
#define LED_BLINK_INTERVAL_MS       200
#define LED_BLINK_COUNT             2

/**
 * @brief  初始化传感器任务
 */
void SensorTask_Init(void)
{
    /* 初始化按钮状态 */
    button1State.pressTime = 0;
    button1State.releaseTime = 0;
    button1State.waitingSecondClick = false;
    button1State.firstClickTime = 0;
    
    button2State.pressTime = 0;
    button2State.releaseTime = 0;
    button2State.waitingSecondClick = false;
    button2State.firstClickTime = 0;
    
    /* 初始化LED闪烁状态 */
    led3BlinkState.blinkStartTime = 0;
    led3BlinkState.blinkCount = 0;
    led3BlinkState.isBlinking = false;
}

/**
 * @brief  LED闪烁处理
 */
static void SensorTask_HandleLEDBlink(void)
{
    if (g_pCleanBotApp == NULL) return;
    if (!led3BlinkState.isBlinking) return;
    
    uint32_t currentTime = HAL_GetTick();
    uint32_t elapsed = currentTime - led3BlinkState.blinkStartTime;
    uint32_t phase = elapsed / LED_BLINK_INTERVAL_MS;
    
    if (phase >= led3BlinkState.blinkCount * 2) {
        /* 闪烁完成 */
        led3BlinkState.isBlinking = false;
        LED_Off(&g_pCleanBotApp->led3);
    } else {
        /* 根据相位控制LED */
        if (phase % 2 == 0) {
            LED_On(&g_pCleanBotApp->led3);
        } else {
            LED_Off(&g_pCleanBotApp->led3);
        }
    }
}

/**
 * @brief  启动LED闪烁
 */
static void SensorTask_StartLEDBlink(uint8_t count)
{
    if (g_pCleanBotApp == NULL) return;
    led3BlinkState.blinkStartTime = HAL_GetTick();
    led3BlinkState.blinkCount = count;
    led3BlinkState.isBlinking = true;
    LED_On(&g_pCleanBotApp->led3);
}

/**
 * @brief  处理按钮1事件
 */
static void SensorTask_HandleButton1Event(SensorEvent_t *event)
{
    if (g_pCleanBotApp == NULL) return;
    
    uint32_t currentTime = HAL_GetTick();
    
    if (event->type == SENSOR_EVENT_BUTTON1_PRESS) {
        button1State.pressTime = currentTime;
        /* 按下时点亮LED3 */
        LED_On(&g_pCleanBotApp->led3);
    } else if (event->type == SENSOR_EVENT_BUTTON1_RELEASE) {
        button1State.releaseTime = currentTime;
        
        /* 检查是否在双击窗口内 */
        if (button1State.waitingSecondClick) {
            uint32_t gap = currentTime - button1State.firstClickTime;
            if (gap < BUTTON_DOUBLE_CLICK_GAP_MS) {
                /* 双击 */
                // TODO: 处理双击事件
                SensorTask_StartLEDBlink(2);
                button1State.waitingSecondClick = false;
            } else {
                /* 超时，视为单击 */
                // TODO: 处理单击事件
                SensorTask_StartLEDBlink(1);
                button1State.waitingSecondClick = false;
            }
        } else {
            /* 第一次单击，等待第二次 */
            button1State.firstClickTime = currentTime;
            button1State.waitingSecondClick = true;
        }
    }
}

/**
 * @brief  处理按钮2事件
 */
static void SensorTask_HandleButton2Event(SensorEvent_t *event)
{
    if (g_pCleanBotApp == NULL) return;
    
    uint32_t currentTime = HAL_GetTick();
    
    if (event->type == SENSOR_EVENT_BUTTON2_PRESS) {
        button2State.pressTime = currentTime;
        LED_On(&g_pCleanBotApp->led3);
    } else if (event->type == SENSOR_EVENT_BUTTON2_RELEASE) {
        button2State.releaseTime = currentTime;
        
        if (button2State.waitingSecondClick) {
            uint32_t gap = currentTime - button2State.firstClickTime;
            if (gap < BUTTON_DOUBLE_CLICK_GAP_MS) {
                /* 双击 */
                SensorTask_StartLEDBlink(2);
                button2State.waitingSecondClick = false;
            } else {
                /* 单击 */
                SensorTask_StartLEDBlink(1);
                button2State.waitingSecondClick = false;
            }
        } else {
            button2State.firstClickTime = currentTime;
            button2State.waitingSecondClick = true;
        }
    }
}

/**
 * @brief  处理红外传感器事件（NEC解码）
 */
static void SensorTask_HandleIREvent(SensorEvent_t *event, IR_Sensor_t *sensor, int index)
{
    /* 从事件中提取边沿信息（这里需要实际实现NEC解码） */
    /* 由于NEC解码需要精确的时间测量，建议使用定时器捕获 */
    /* 这里简化处理，实际应该在中断中使用定时器捕获边沿时间 */
    
    /* 临时处理：闪烁LED表示收到红外信号 */
    if (g_pCleanBotApp != NULL) {
        SensorTask_StartLEDBlink(2);
    }
    
    /* TODO: 实现完整的NEC解码逻辑 */
}

/**
 * @brief  处理光电门事件
 */
static void SensorTask_HandlePhotoGateEvent(SensorEvent_t *event)
{
    if (g_pCleanBotApp == NULL) return;
    
    if (event->type == SENSOR_EVENT_PHOTO_GATE_LEFT) {
        g_pCleanBotApp->photoGateLeft.state = (event->data == 1) ? PHOTO_GATE_BLOCKED : PHOTO_GATE_CLEAR;
        /* 碰撞检测：闪烁LED */
        if (event->data == 1) {
            SensorTask_StartLEDBlink(2);
        }
    } else if (event->type == SENSOR_EVENT_PHOTO_GATE_RIGHT) {
        g_pCleanBotApp->photoGateRight.state = (event->data == 1) ? PHOTO_GATE_BLOCKED : PHOTO_GATE_CLEAR;
        if (event->data == 1) {
            SensorTask_StartLEDBlink(2);
        }
    }
}

/**
 * @brief  检查按钮单击超时
 */
static void SensorTask_CheckButtonTimeout(void)
{
    uint32_t currentTime = HAL_GetTick();
    
    /* 检查按钮1 */
    if (button1State.waitingSecondClick) {
        if (currentTime - button1State.firstClickTime > BUTTON_DOUBLE_CLICK_GAP_MS) {
            /* 超时，视为单击 */
            SensorTask_StartLEDBlink(1);
            button1State.waitingSecondClick = false;
        }
    }
    
    /* 检查按钮2 */
    if (button2State.waitingSecondClick) {
        if (currentTime - button2State.firstClickTime > BUTTON_DOUBLE_CLICK_GAP_MS) {
            SensorTask_StartLEDBlink(1);
            button2State.waitingSecondClick = false;
        }
    }
}

/**
 * @brief  传感器任务主函数
 */
void SensorTask_Run(void *argument)
{
    SensorEvent_t event;
    SensorManager_t *sensorManager = SensorManager_GetInstance();
    
    SensorTask_Init();
    SensorManager_Start(sensorManager);
    
    while (1) {
        /* 处理LED闪烁 */
        SensorTask_HandleLEDBlink();
        
        /* 检查按钮滤波检测 */
        SensorManager_CheckButtonDebounce(sensorManager);
        
        /* 检查按钮超时 */
        SensorTask_CheckButtonTimeout();
        
        /* 从队列获取事件 */
        if (SensorManager_GetEvent(sensorManager, &event, 10)) {
            if (g_pCleanBotApp != NULL) {
                switch (event.type) {
                    case SENSOR_EVENT_IR_LEFT:
                        SensorTask_HandleIREvent(&event, &g_pCleanBotApp->irSensorLeft, 0);
                        break;
                    case SENSOR_EVENT_IR_RIGHT:
                        SensorTask_HandleIREvent(&event, &g_pCleanBotApp->irSensorRight, 1);
                        break;
                    case SENSOR_EVENT_IR_FRONT_LEFT:
                        SensorTask_HandleIREvent(&event, &g_pCleanBotApp->irSensorFrontLeft, 2);
                        break;
                    case SENSOR_EVENT_IR_FRONT_RIGHT:
                        SensorTask_HandleIREvent(&event, &g_pCleanBotApp->irSensorFrontRight, 3);
                        break;
                    case SENSOR_EVENT_PHOTO_GATE_LEFT:
                    case SENSOR_EVENT_PHOTO_GATE_RIGHT:
                        SensorTask_HandlePhotoGateEvent(&event);
                        break;
                    case SENSOR_EVENT_BUTTON1_PRESS:
                    case SENSOR_EVENT_BUTTON1_RELEASE:
                        SensorTask_HandleButton1Event(&event);
                        break;
                    case SENSOR_EVENT_BUTTON2_PRESS:
                    case SENSOR_EVENT_BUTTON2_RELEASE:
                        SensorTask_HandleButton2Event(&event);
                        break;
                    default:
                        break;
                }
            }
        }
        
        osDelay(10);
    }
}

