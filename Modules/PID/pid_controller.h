/**
  ******************************************************************************
  * @file    pid_controller.h
  * @brief   PID控制器头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __PID_CONTROLLER_H__
#define __PID_CONTROLLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* PID控制器结构体 */
typedef struct {
    float kp;                   /* 比例系数 */
    float ki;                   /* 积分系数 */
    float kd;                   /* 微分系数 */
    float target;               /* 目标值 */
    float current;              /* 当前值 */
    float error;                /* 误差 */
    float lastError;            /* 上次误差 */
    float integral;             /* 积分项 */
    float derivative;           /* 微分项 */
    float output;               /* 输出值 */
    float outputMax;            /* 输出最大值 */
    float outputMin;            /* 输出最小值 */
    float integralMax;          /* 积分限幅 */
    float integralMin;          /* 积分限幅 */
    uint32_t lastTime;          /* 上次计算时间 */
    bool enabled;               /* 使能标志 */
} PIDController_t;

/* 函数声明 */
void PID_Init(PIDController_t *pid, float kp, float ki, float kd);
void PID_SetTarget(PIDController_t *pid, float target);
void PID_SetParams(PIDController_t *pid, float kp, float ki, float kd);
void PID_SetOutputLimit(PIDController_t *pid, float min, float max);
void PID_SetIntegralLimit(PIDController_t *pid, float min, float max);
float PID_Compute(PIDController_t *pid, float current);
void PID_Reset(PIDController_t *pid);
void PID_Enable(PIDController_t *pid);
void PID_Disable(PIDController_t *pid);
float PID_GetOutput(PIDController_t *pid);
float PID_GetError(PIDController_t *pid);

#ifdef __cplusplus
}
#endif

#endif /* __PID_CONTROLLER_H__ */

