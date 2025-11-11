/**
  ******************************************************************************
  * @file    pid_controller.c
  * @brief   PID控制器实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "pid_controller.h"
#include "cmsis_os.h"
#include <stddef.h>  /* 定义NULL */

/**
  * @brief  初始化PID控制器
  * @param  pid: PID控制器对象指针
  * @param  kp: 比例系数
  * @param  ki: 积分系数
  * @param  kd: 微分系数
  * @retval None
  */
void PID_Init(PIDController_t *pid, float kp, float ki, float kd)
{
    if (pid == NULL) return;
    
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->target = 0.0f;
    pid->current = 0.0f;
    pid->error = 0.0f;
    pid->lastError = 0.0f;
    pid->integral = 0.0f;
    pid->derivative = 0.0f;
    pid->output = 0.0f;
    pid->outputMax = 1000.0f;
    pid->outputMin = -1000.0f;
    pid->integralMax = 1000.0f;
    pid->integralMin = -1000.0f;
    pid->lastTime = osKernelGetTickCount();
    pid->enabled = true;
}

/**
  * @brief  设置目标值
  * @param  pid: PID控制器对象指针
  * @param  target: 目标值
  * @retval None
  */
void PID_SetTarget(PIDController_t *pid, float target)
{
    if (pid == NULL) return;
    pid->target = target;
}

/**
  * @brief  设置PID参数
  * @param  pid: PID控制器对象指针
  * @param  kp: 比例系数
  * @param  ki: 积分系数
  * @param  kd: 微分系数
  * @retval None
  */
void PID_SetParams(PIDController_t *pid, float kp, float ki, float kd)
{
    if (pid == NULL) return;
    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

/**
  * @brief  设置输出限幅
  * @param  pid: PID控制器对象指针
  * @param  min: 最小值
  * @param  max: 最大值
  * @retval None
  */
void PID_SetOutputLimit(PIDController_t *pid, float min, float max)
{
    if (pid == NULL) return;
    pid->outputMin = min;
    pid->outputMax = max;
}

/**
  * @brief  设置积分限幅
  * @param  pid: PID控制器对象指针
  * @param  min: 最小值
  * @param  max: 最大值
  * @retval None
  */
void PID_SetIntegralLimit(PIDController_t *pid, float min, float max)
{
    if (pid == NULL) return;
    pid->integralMin = min;
    pid->integralMax = max;
}

/**
  * @brief  PID计算
  * @param  pid: PID控制器对象指针
  * @param  current: 当前值
  * @retval 输出值
  */
float PID_Compute(PIDController_t *pid, float current)
{
    if (pid == NULL || !pid->enabled) return 0.0f;
    
    uint32_t currentTime = osKernelGetTickCount();
    float dt = (currentTime - pid->lastTime) / 1000.0f;  /* 转换为秒 */
    
    if (dt <= 0.0f) dt = 0.001f;  /* 防止除零 */
    
    pid->current = current;
    pid->error = pid->target - pid->current;
    
    /* 比例项 */
    float pTerm = pid->kp * pid->error;
    
    /* 积分项 */
    pid->integral += pid->error * dt;
    
    /* 积分限幅 */
    if (pid->integral > pid->integralMax) {
        pid->integral = pid->integralMax;
    } else if (pid->integral < pid->integralMin) {
        pid->integral = pid->integralMin;
    }
    
    float iTerm = pid->ki * pid->integral;
    
    /* 微分项 */
    pid->derivative = (pid->error - pid->lastError) / dt;
    float dTerm = pid->kd * pid->derivative;
    
    /* 计算输出 */
    pid->output = pTerm + iTerm + dTerm;
    
    /* 输出限幅 */
    if (pid->output > pid->outputMax) {
        pid->output = pid->outputMax;
    } else if (pid->output < pid->outputMin) {
        pid->output = pid->outputMin;
    }
    
    pid->lastError = pid->error;
    pid->lastTime = currentTime;
    
    return pid->output;
}

/**
  * @brief  复位PID控制器
  * @param  pid: PID控制器对象指针
  * @retval None
  */
void PID_Reset(PIDController_t *pid)
{
    if (pid == NULL) return;
    
    pid->error = 0.0f;
    pid->lastError = 0.0f;
    pid->integral = 0.0f;
    pid->derivative = 0.0f;
    pid->output = 0.0f;
    pid->lastTime = osKernelGetTickCount();
}

/**
  * @brief  使能PID控制器
  * @param  pid: PID控制器对象指针
  * @retval None
  */
void PID_Enable(PIDController_t *pid)
{
    if (pid == NULL) return;
    pid->enabled = true;
    PID_Reset(pid);
}

/**
  * @brief  禁用PID控制器
  * @param  pid: PID控制器对象指针
  * @retval None
  */
void PID_Disable(PIDController_t *pid)
{
    if (pid == NULL) return;
    pid->enabled = false;
    pid->output = 0.0f;
}

/**
  * @brief  获取输出值
  * @param  pid: PID控制器对象指针
  * @retval 输出值
  */
float PID_GetOutput(PIDController_t *pid)
{
    if (pid == NULL) return 0.0f;
    return pid->output;
}

/**
  * @brief  获取误差值
  * @param  pid: PID控制器对象指针
  * @retval 误差值
  */
float PID_GetError(PIDController_t *pid)
{
    if (pid == NULL) return 0.0f;
    return pid->error;
}

