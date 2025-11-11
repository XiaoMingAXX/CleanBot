/**
  ******************************************************************************
  * @file    motor_ctrl_task.c
  * @brief   电机控制任务实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "motor_ctrl_task.h"
#include "CleanBotApp.h"
#include "led.h"
#include "cmsis_os.h"
#include <string.h>

/* 外部应用对象 */
extern CleanBotApp_t *g_pCleanBotApp;

/* 电机控制结构体 */
static MotorCtrl_t g_MotorCtrl;

/* LED2闪烁状态 */
static struct {
    uint32_t lastToggleTime;
    bool state;
} led2State;

#define LED2_TOGGLE_INTERVAL_MS    500

/**
 * @brief  初始化电机控制任务
 */
void MotorCtrlTask_Init(void)
{
    memset(&g_MotorCtrl, 0, sizeof(MotorCtrl_t));
    
    g_MotorCtrl.wheelMotor.enabled = false;
    g_MotorCtrl.brushMotorLeft = BRUSH_MOTOR_LEVEL_OFF;
    g_MotorCtrl.brushMotorRight = BRUSH_MOTOR_LEVEL_OFF;
    g_MotorCtrl.pumpMotor = PUMP_MOTOR_LEVEL_OFF;
    g_MotorCtrl.fanMotor = FAN_MOTOR_LEVEL_OFF;
    
    led2State.lastToggleTime = 0;
    led2State.state = false;
}

/**
 * @brief  LED2状态指示
 */
static void MotorCtrlTask_UpdateLED2(void)
{
    if (g_pCleanBotApp == NULL) return;
    
    uint32_t currentTime = HAL_GetTick();
    
    if (currentTime - led2State.lastToggleTime >= LED2_TOGGLE_INTERVAL_MS) {
        led2State.state = !led2State.state;
        if (led2State.state) {
            LED_On(&g_pCleanBotApp->led2);
        } else {
            LED_Off(&g_pCleanBotApp->led2);
        }
        led2State.lastToggleTime = currentTime;
    }
}

/**
 * @brief  轮电机PID控制
 */
static void MotorCtrlTask_WheelMotorControl(void)
{
    if (g_pCleanBotApp == NULL) return;
    
    if (!g_MotorCtrl.wheelMotor.enabled) {
        Motor_Stop(&g_pCleanBotApp->wheelMotorLeft);
        Motor_Stop(&g_pCleanBotApp->wheelMotorRight);
        return;
    }
    
    /* 更新编码器速度 */
    Encoder_Update(&g_pCleanBotApp->encoderWheelLeft);
    Encoder_Update(&g_pCleanBotApp->encoderWheelRight);
    
    /* 获取当前速度 (m/s) */
    float leftSpeedMs = Encoder_GetSpeedMs(&g_pCleanBotApp->encoderWheelLeft);
    float rightSpeedMs = Encoder_GetSpeedMs(&g_pCleanBotApp->encoderWheelRight);
    
    /* PID控制 - 目标速度转换为RPM（需要根据实际参数计算） */
    /* 简化处理：直接将m/s转换为PWM占空比 */
    /* 这里需要根据实际硬件特性调整转换系数 */
    float leftTargetRPM = g_MotorCtrl.wheelMotor.leftSpeedMs * 60.0f / (3.14159f * 0.1f);  /* 假设轮子直径0.1m */
    float rightTargetRPM = g_MotorCtrl.wheelMotor.rightSpeedMs * 60.0f / (3.14159f * 0.1f);
    
    /* 设置PID目标值 */
    PID_SetTarget(&g_pCleanBotApp->pidWheelLeft, leftTargetRPM);
    PID_SetTarget(&g_pCleanBotApp->pidWheelRight, rightTargetRPM);
    
    /* 获取当前RPM */
    float leftCurrentRPM = Encoder_GetSpeed(&g_pCleanBotApp->encoderWheelLeft);
    float rightCurrentRPM = Encoder_GetSpeed(&g_pCleanBotApp->encoderWheelRight);
    
    /* PID计算 */
    float leftOutput = PID_Compute(&g_pCleanBotApp->pidWheelLeft, leftCurrentRPM);
    float rightOutput = PID_Compute(&g_pCleanBotApp->pidWheelRight, rightCurrentRPM);
    
    /* 设置电机速度和方向 */
    /* 左轮电机 */
    if (leftTargetRPM >= 0) {
        /* 正向 */
        Motor_SetDirection(&g_pCleanBotApp->wheelMotorLeft, MOTOR_STATE_FORWARD);
        /* 限制速度范围在0-1000 */
        int16_t leftSpeed = (int16_t)leftOutput;
        if (leftSpeed < 0) leftSpeed = 0;
        if (leftSpeed > 1000) leftSpeed = 1000;
        Motor_SetSpeed(&g_pCleanBotApp->wheelMotorLeft, leftSpeed);
    } else {
        /* 反向 */
        Motor_SetDirection(&g_pCleanBotApp->wheelMotorLeft, MOTOR_STATE_BACKWARD);
        /* 取绝对值并限制速度范围在0-1000 */
        int16_t leftSpeed = (int16_t)(-leftOutput);
        if (leftSpeed < 0) leftSpeed = 0;
        if (leftSpeed > 1000) leftSpeed = 1000;
        Motor_SetSpeed(&g_pCleanBotApp->wheelMotorLeft, leftSpeed);
    }
    
    /* 右轮电机 */
    if (rightTargetRPM >= 0) {
        /* 正向 */
        Motor_SetDirection(&g_pCleanBotApp->wheelMotorRight, MOTOR_STATE_FORWARD);
        /* 限制速度范围在0-1000 */
        int16_t rightSpeed = (int16_t)rightOutput;
        if (rightSpeed < 0) rightSpeed = 0;
        if (rightSpeed > 1000) rightSpeed = 1000;
        Motor_SetSpeed(&g_pCleanBotApp->wheelMotorRight, rightSpeed);
    } else {
        /* 反向 */
        Motor_SetDirection(&g_pCleanBotApp->wheelMotorRight, MOTOR_STATE_BACKWARD);
        /* 取绝对值并限制速度范围在0-1000 */
        int16_t rightSpeed = (int16_t)(-rightOutput);
        if (rightSpeed < 0) rightSpeed = 0;
        if (rightSpeed > 1000) rightSpeed = 1000;
        Motor_SetSpeed(&g_pCleanBotApp->wheelMotorRight, rightSpeed);
    }
}

/**
 * @brief  边刷电机控制
 */
static void MotorCtrlTask_BrushMotorControl(void)
{
    if (g_pCleanBotApp == NULL) return;
    
    /* 左边刷 */
    int16_t leftSpeed = 0;
    switch (g_MotorCtrl.brushMotorLeft) {
        case BRUSH_MOTOR_LEVEL_OFF:
            leftSpeed = BRUSH_MOTOR_SPEED_OFF;
            break;
        case BRUSH_MOTOR_LEVEL_LOW:
            leftSpeed = BRUSH_MOTOR_SPEED_LOW;
            break;
        case BRUSH_MOTOR_LEVEL_HIGH:
            leftSpeed = BRUSH_MOTOR_SPEED_HIGH;
            break;
    }
    Motor_SetSpeed(&g_pCleanBotApp->brushMotorLeft, leftSpeed);
    Motor_SetDirection(&g_pCleanBotApp->brushMotorLeft, MOTOR_STATE_FORWARD);
    
    /* 右边刷 */
    int16_t rightSpeed = 0;
    switch (g_MotorCtrl.brushMotorRight) {
        case BRUSH_MOTOR_LEVEL_OFF:
            rightSpeed = BRUSH_MOTOR_SPEED_OFF;
            break;
        case BRUSH_MOTOR_LEVEL_LOW:
            rightSpeed = BRUSH_MOTOR_SPEED_LOW;
            break;
        case BRUSH_MOTOR_LEVEL_HIGH:
            rightSpeed = BRUSH_MOTOR_SPEED_HIGH;
            break;
    }
    Motor_SetSpeed(&g_pCleanBotApp->brushMotorRight, rightSpeed);
    Motor_SetDirection(&g_pCleanBotApp->brushMotorRight, MOTOR_STATE_FORWARD);
}

/**
 * @brief  水箱增压电机控制
 */
static void MotorCtrlTask_PumpMotorControl(void)
{
    if (g_pCleanBotApp == NULL) return;
    
    int16_t speed = 0;
    switch (g_MotorCtrl.pumpMotor) {
        case PUMP_MOTOR_LEVEL_OFF:
            speed = PUMP_MOTOR_SPEED_OFF;
            break;
        case PUMP_MOTOR_LEVEL_LOW:
            speed = PUMP_MOTOR_SPEED_LOW;
            break;
        case PUMP_MOTOR_LEVEL_MEDIUM:
            speed = PUMP_MOTOR_SPEED_MEDIUM;
            break;
        case PUMP_MOTOR_LEVEL_HIGH:
            speed = PUMP_MOTOR_SPEED_HIGH;
            break;
    }
    Motor_SetSpeed(&g_pCleanBotApp->pumpMotor, speed);
    Motor_SetDirection(&g_pCleanBotApp->pumpMotor, MOTOR_STATE_FORWARD);
}

/**
 * @brief  风机PID控制
 */
static void MotorCtrlTask_FanMotorControl(void)
{
    if (g_pCleanBotApp == NULL) return;
    
    float targetRPM = 0;
    switch (g_MotorCtrl.fanMotor) {
        case FAN_MOTOR_LEVEL_OFF:
            targetRPM = FAN_MOTOR_SPEED_OFF;
            break;
        case FAN_MOTOR_LEVEL_1:
            targetRPM = FAN_MOTOR_SPEED_1;
            break;
        case FAN_MOTOR_LEVEL_2:
            targetRPM = FAN_MOTOR_SPEED_2;
            break;
        case FAN_MOTOR_LEVEL_3:
            targetRPM = FAN_MOTOR_SPEED_3;
            break;
        case FAN_MOTOR_LEVEL_4:
            targetRPM = FAN_MOTOR_SPEED_4;
            break;
        case FAN_MOTOR_LEVEL_5:
            targetRPM = FAN_MOTOR_SPEED_5;
            break;
    }
    
    if (targetRPM == 0) {
        Motor_Stop(&g_pCleanBotApp->fanMotor);
        return;
    }
    
    /* 更新编码器速度 */
    Encoder_Update(&g_pCleanBotApp->encoderFan);
    
    /* 设置PID目标值 */
    PID_SetTarget(&g_pCleanBotApp->pidFan, targetRPM);
    
    /* 获取当前RPM */
    float currentRPM = Encoder_GetSpeed(&g_pCleanBotApp->encoderFan);
    
    /* PID计算 */
    float output = PID_Compute(&g_pCleanBotApp->pidFan, currentRPM);
    
    /* 设置电机速度 */
    Motor_SetDirection(&g_pCleanBotApp->fanMotor, MOTOR_STATE_FORWARD);
    Motor_SetSpeed(&g_pCleanBotApp->fanMotor, (int16_t)output);
}
float leftCurrentRPM,rightCurrentRPM=0.0;
/**
 * @brief  电机控制任务主函数
 */
void MotorCtrlTask_Run(void *argument)
{
    MotorCtrlTask_Init();
    
    while (1) {
        /* 更新LED2状态 */
        MotorCtrlTask_UpdateLED2();
        
        // /* 轮电机控制 */
        // MotorCtrlTask_WheelMotorControl();
        
        // /* 边刷电机控制 */
        // MotorCtrlTask_BrushMotorControl();
        
        // /* 水箱增压电机控制 */
        // MotorCtrlTask_PumpMotorControl();
        
        // /* 风机控制 */
        // MotorCtrlTask_FanMotorControl();
        // Motor_SetSpeed(&g_pCleanBotApp->fanMotor, 500);
        // Motor_SetSpeed(&g_pCleanBotApp->pumpMotor, 500);
        // Motor_SetSpeed(&g_pCleanBotApp->brushMotorLeft, 500);
        // Motor_SetSpeed(&g_pCleanBotApp->brushMotorRight, 500);
         Motor_SetDirection(&g_pCleanBotApp->wheelMotorLeft, MOTOR_STATE_BACKWARD);
         Motor_SetDirection(&g_pCleanBotApp->wheelMotorRight, MOTOR_STATE_BACKWARD);
         Motor_SetSpeed(&g_pCleanBotApp->wheelMotorLeft, 0);
         Motor_SetSpeed(&g_pCleanBotApp->wheelMotorRight, 0);
        leftCurrentRPM = Encoder_GetSpeed(&g_pCleanBotApp->encoderWheelLeft);
        rightCurrentRPM = Encoder_GetSpeed(&g_pCleanBotApp->encoderWheelRight);

        osDelay(5);  /* 5ms控制周期 */
    }
}

/**
 * @brief  设置轮电机目标速度
 */
void MotorCtrlTask_SetWheelSpeed(float leftSpeedMs, float rightSpeedMs)
{
    g_MotorCtrl.wheelMotor.leftSpeedMs = leftSpeedMs;
    g_MotorCtrl.wheelMotor.rightSpeedMs = rightSpeedMs;
    g_MotorCtrl.wheelMotor.enabled = true;
}

/**
 * @brief  设置边刷电机档位
 */
void MotorCtrlTask_SetBrushMotor(BrushMotorLevel_t left, BrushMotorLevel_t right)
{
    g_MotorCtrl.brushMotorLeft = left;
    g_MotorCtrl.brushMotorRight = right;
}

/**
 * @brief  设置水箱增压电机档位
 */
void MotorCtrlTask_SetPumpMotor(PumpMotorLevel_t level)
{
    g_MotorCtrl.pumpMotor = level;
}

/**
 * @brief  设置风机档位
 */
void MotorCtrlTask_SetFanMotor(FanMotorLevel_t level)
{
    g_MotorCtrl.fanMotor = level;
}

/**
 * @brief  获取轮电机当前速度
 */
void MotorCtrlTask_GetWheelSpeed(float *leftSpeedMs, float *rightSpeedMs)
{
    if (leftSpeedMs != NULL) {
        *leftSpeedMs = Encoder_GetSpeedMs(&g_pCleanBotApp->encoderWheelLeft);
    }
    if (rightSpeedMs != NULL) {
        *rightSpeedMs = Encoder_GetSpeedMs(&g_pCleanBotApp->encoderWheelRight);
    }
}

