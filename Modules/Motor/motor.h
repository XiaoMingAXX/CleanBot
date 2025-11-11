/**
  ******************************************************************************
  * @file    motor.h
  * @brief   电机控制基类头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __MOTOR_H__
#define __MOTOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* 电机类型枚举 */
typedef enum {
    MOTOR_TYPE_WHEEL = 0,      /* 轮电机 */
    MOTOR_TYPE_BRUSH,          /* 边刷电机 */
    MOTOR_TYPE_FAN,            /* 吸尘电机 */
    MOTOR_TYPE_PUMP            /* 水箱增压电机 */
} MotorType_t;

/* 电机状态枚举 */
typedef enum {
    MOTOR_STATE_STOP = 0,      /* 停止 */
    MOTOR_STATE_FORWARD,       /* 正转 */
    MOTOR_STATE_BACKWARD,      /* 反转 */
    MOTOR_STATE_BRAKE          /* 刹车 */
} MotorState_t;

/* 电机控制结构体（基类） */
typedef struct Motor Motor_t;

/* 电机虚拟函数表 */
typedef struct {
    void (*SetSpeed)(Motor_t *motor, int16_t speed);       /* 设置速度 */
    void (*SetDirection)(Motor_t *motor, MotorState_t dir); /* 设置方向 */
    void (*Stop)(Motor_t *motor);                           /* 停止 */
    void (*Brake)(Motor_t *motor);                          /* 刹车 */
    int16_t (*GetSpeed)(Motor_t *motor);                    /* 获取当前速度 */
    MotorState_t (*GetState)(Motor_t *motor);               /* 获取当前状态 */
} MotorVTable_t;

/* 电机结构体 */
struct Motor {
    const MotorVTable_t *vtable;  /* 虚拟函数表 */
    MotorType_t type;             /* 电机类型 */
    MotorState_t state;           /* 当前状态 */
    int16_t currentSpeed;         /* 当前速度 (0-1000) */
    int16_t targetSpeed;          /* 目标速度 (0-1000) */
    uint32_t pwmChannel;          /* PWM通道A (INA) */
    uint32_t pwmChannelB;         /* PWM通道B (INB)，0表示不使用双PWM */
    GPIO_TypeDef *dirPort;        /* 方向控制端口（单PWM模式使用） */
    uint16_t dirPin;              /* 方向控制引脚（单PWM模式使用） */
    TIM_HandleTypeDef *htim;      /* 定时器句柄 */
    bool enabled;                 /* 使能标志 */
    bool dualPWM;                 /* 是否使用双PWM模式 */
};

/* 函数声明 */
void Motor_Init(Motor_t *motor, MotorType_t type, TIM_HandleTypeDef *htim, 
                uint32_t channel, GPIO_TypeDef *dirPort, uint16_t dirPin);
void Motor_InitDualPWM(Motor_t *motor, MotorType_t type, TIM_HandleTypeDef *htim, 
                       uint32_t channelA, uint32_t channelB);
void Motor_SetSpeed(Motor_t *motor, int16_t speed);
void Motor_SetDirection(Motor_t *motor, MotorState_t dir);
void Motor_Stop(Motor_t *motor);
void Motor_Brake(Motor_t *motor);
int16_t Motor_GetSpeed(Motor_t *motor);
MotorState_t Motor_GetState(Motor_t *motor);
void Motor_Enable(Motor_t *motor);
void Motor_Disable(Motor_t *motor);
bool Motor_IsEnabled(Motor_t *motor);

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_H__ */

