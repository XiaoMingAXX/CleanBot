/**
  ******************************************************************************
  * @file    motor_ctrl_task.h
  * @brief   电机控制任务头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __MOTOR_CTRL_TASK_H__
#define __MOTOR_CTRL_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cleanbot_config.h"

/* 轮电机速度控制（m/s） */
typedef struct {
    float leftSpeedMs;      /* 左轮目标速度 (m/s) */
    float rightSpeedMs;     /* 右轮目标速度 (m/s) */
    bool enabled;           /* 使能标志 */
} WheelMotorCtrl_t;

/* 边刷电机控制 */
typedef enum {
    BRUSH_MOTOR_LEVEL_OFF = 0,
    BRUSH_MOTOR_LEVEL_LOW,
    BRUSH_MOTOR_LEVEL_HIGH
} BrushMotorLevel_t;

/* 水箱增压电机控制 */
typedef enum {
    PUMP_MOTOR_LEVEL_OFF = 0,
    PUMP_MOTOR_LEVEL_LOW,
    PUMP_MOTOR_LEVEL_MEDIUM,
    PUMP_MOTOR_LEVEL_HIGH,
    PUMP_MOTOR_LEVEL_TURBO,
    PUMP_MOTOR_LEVEL_ULTRA
} PumpMotorLevel_t;

/* 风机速度控制 */
typedef enum {
    FAN_MOTOR_LEVEL_OFF = 0,
    FAN_MOTOR_LEVEL_1,
    FAN_MOTOR_LEVEL_2,
    FAN_MOTOR_LEVEL_3,
    FAN_MOTOR_LEVEL_4,
    FAN_MOTOR_LEVEL_5
} FanMotorLevel_t;

/* 电机控制结构体 */
typedef struct {
    WheelMotorCtrl_t wheelMotor;
    BrushMotorLevel_t brushMotorLeft;
    BrushMotorLevel_t brushMotorRight;
    PumpMotorLevel_t pumpMotor;
    FanMotorLevel_t fanMotor;
} MotorCtrl_t;

/* 函数声明 */
void MotorCtrlTask_Init(void);
void MotorCtrlTask_Run(void *argument);

/* 设置轮电机目标速度（主应用层调用） */
void MotorCtrlTask_SetWheelSpeed(float leftSpeedMs, float rightSpeedMs);

/* 设置边刷电机档位 */
void MotorCtrlTask_SetBrushMotor(BrushMotorLevel_t left, BrushMotorLevel_t right);

/* 设置水箱增压电机档位 */
void MotorCtrlTask_SetPumpMotor(PumpMotorLevel_t level);

/* 设置风机档位 */
void MotorCtrlTask_SetFanMotor(FanMotorLevel_t level);

/* 获取轮电机当前速度（m/s） */
void MotorCtrlTask_GetWheelSpeed(float *leftSpeedMs, float *rightSpeedMs);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_CTRL_TASK_H__ */

