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
#include "nec_decode.h"
#include "ir_homing.h"
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
    if (event == NULL || sensor == NULL || g_pCleanBotApp == NULL) {
        return;
    }
    
    /* 从事件数据中提取边沿信息 */
    /* event->data 格式：bit 0 = 电平(0=低,1=高), bit 1-31 = 时间间隔(微秒) */
    bool level = (event->data & 0x01) != 0;
    
    /* 使用事件时间戳作为绝对时间（微秒）传递给解码器 */
    /* 解码器内部会计算时间间隔 */
    uint32_t absoluteTime = event->timestamp;
    
    /* 处理边沿信号到NEC解码器 */
    bool decodeComplete = NEC_Decoder_ProcessEdge(&sensor->decoder, absoluteTime, level);
    
    /* 检查解码是否完成 */
    if (decodeComplete || NEC_Decoder_IsDataReady(&sensor->decoder)) {
        /* 获取解码数据 */
        NEC_Data_t necData = NEC_Decoder_GetData(&sensor->decoder);
        
        if (necData.valid) {
            /* 解码成功，处理NEC数据 */
            /* 闪烁LED表示收到有效的NEC信号 */
            SensorTask_StartLEDBlink(2);
            
            /* 更新红外回冲定位模块的接收器数据 */
            IRPosition_t position;
            switch (index) {
                case 0:  /* 左侧红外传感器 */
                    position = IR_POSITION_LEFT;
                    break;
                case 1:  /* 右侧红外传感器 */
                    position = IR_POSITION_RIGHT;
                    break;
                case 2:  /* 左前红外传感器 */
                    position = IR_POSITION_FRONT_LEFT;
                    break;
                case 3:  /* 右前红外传感器 */
                    position = IR_POSITION_FRONT_RIGHT;
                    break;
                default:
                    position = IR_POSITION_LEFT;  /* 默认值，不应该到达这里 */
                    break;
            }
            IRHoming_UpdateReceiver(&g_pCleanBotApp->irHoming, position, &necData);
        } else {
            /* 解码失败，可能是时序错误或数据无效 */
            /* 可以闪烁一次LED表示错误 */
            SensorTask_StartLEDBlink(1);
        }
    }
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

    IRHoming_UpdateBumperState(&g_pCleanBotApp->irHoming,
                               g_pCleanBotApp->photoGateLeft.state == PHOTO_GATE_BLOCKED,
                               g_pCleanBotApp->photoGateRight.state == PHOTO_GATE_BLOCKED);
}

/**
 * @brief  处理下视传感器事件
 */
static void SensorTask_HandleUnderSensorEvent(SensorEvent_t *event)
{
    if (g_pCleanBotApp == NULL) return;
    
    bool isSuspended = (event->data == 1);  /* 1=悬空（高电平），0=地面（低电平） */
    
    switch (event->type) {
        case SENSOR_EVENT_UNDER_LEFT:
            g_pCleanBotApp->underLeftSuspended = isSuspended;
            /* TODO: 根据左前下视传感器状态执行相应操作 */
            /* 例如：如果悬空，可能需要停止或减速 */
            if (isSuspended) {
                /* 检测到悬空，可以闪烁LED提示 */
                SensorTask_StartLEDBlink(1);
            }
            break;
            
        case SENSOR_EVENT_UNDER_RIGHT:
            g_pCleanBotApp->underRightSuspended = isSuspended;
            /* TODO: 根据右前下视传感器状态执行相应操作 */
            if (isSuspended) {
                SensorTask_StartLEDBlink(1);
            }
            break;
            
        case SENSOR_EVENT_UNDER_CENTER:
            g_pCleanBotApp->underCenterSuspended = isSuspended;
            /* TODO: 根据中间下视传感器状态执行相应操作 */
            if (isSuspended) {
                SensorTask_StartLEDBlink(1);
            }
            break;
            
        default:
            break;
    }
    
    /* 如果所有下视传感器都检测到悬空，可能需要紧急停止 */
    if (g_pCleanBotApp->underLeftSuspended && 
        g_pCleanBotApp->underRightSuspended && 
        g_pCleanBotApp->underCenterSuspended) {
        /* 所有下视传感器都悬空，可能处于危险状态 */
        /* TODO: 执行紧急停止或报警 */
        SensorTask_StartLEDBlink(3);  /* 快速闪烁3次表示危险 */
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
        
        /* 处理红外回冲定位（如果使能） */
        if (g_pCleanBotApp != NULL) {
            IRHoming_Process(&g_pCleanBotApp->irHoming);
        }
        
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
                    case SENSOR_EVENT_UNDER_LEFT:
                    case SENSOR_EVENT_UNDER_RIGHT:
                    case SENSOR_EVENT_UNDER_CENTER:
                        SensorTask_HandleUnderSensorEvent(&event);
                        break;
                    default:
                        break;
                }
            }
        }
        
        osDelay(10);
    }
}

