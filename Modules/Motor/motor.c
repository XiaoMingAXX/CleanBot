/**
  ******************************************************************************
  * @file    motor.c
  * @brief   电机控制基类实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "motor.h"

/* 私有函数声明 */
static void Motor_Private_SetSpeed(Motor_t *motor, int16_t speed);
static void Motor_Private_SetDirection(Motor_t *motor, MotorState_t dir);
static void Motor_Private_Stop(Motor_t *motor);
static void Motor_Private_Brake(Motor_t *motor);
static int16_t Motor_Private_GetSpeed(Motor_t *motor);
static MotorState_t Motor_Private_GetState(Motor_t *motor);

/* 虚拟函数表 */
static const MotorVTable_t Motor_VTable = {
    .SetSpeed = Motor_Private_SetSpeed,
    .SetDirection = Motor_Private_SetDirection,
    .Stop = Motor_Private_Stop,
    .Brake = Motor_Private_Brake,
    .GetSpeed = Motor_Private_GetSpeed,
    .GetState = Motor_Private_GetState
};

/**
  * @brief  初始化电机
  * @param  motor: 电机对象指针
  * @param  type: 电机类型
  * @param  htim: 定时器句柄
  * @param  channel: PWM通道
  * @param  dirPort: 方向控制端口
  * @param  dirPin: 方向控制引脚
  * @retval None
  */
void Motor_Init(Motor_t *motor, MotorType_t type, TIM_HandleTypeDef *htim, 
                uint32_t channel, GPIO_TypeDef *dirPort, uint16_t dirPin)
{
    if (motor == NULL) return;
    
    motor->vtable = &Motor_VTable;
    motor->type = type;
    motor->state = MOTOR_STATE_STOP;
    motor->currentSpeed = 0;
    motor->targetSpeed = 0;
    motor->pwmChannel = channel;
    motor->dirPort = dirPort;
    motor->dirPin = dirPin;
    motor->htim = htim;
    motor->enabled = false;
    
    /* 启动PWM */
    if (htim != NULL) {
        HAL_TIM_PWM_Start(htim, channel);
    }
    
    /* 初始化方向引脚为低电平 */
    if (dirPort != NULL) {
        HAL_GPIO_WritePin(dirPort, dirPin, GPIO_PIN_RESET);
    }
}

/**
  * @brief  设置电机速度
  * @param  motor: 电机对象指针
  * @param  speed: 速度值 (0-1000)
  * @retval None
  */
static void Motor_Private_SetSpeed(Motor_t *motor, int16_t speed)
{
    if (motor == NULL || motor->htim == NULL) return;
    
    /* 限制速度范围 */
    if (speed > 1000) speed = 1000;
    if (speed < 0) speed = 0;
    
    motor->targetSpeed = speed;
    
    if (!motor->enabled) {
        motor->currentSpeed = 0;
        __HAL_TIM_SET_COMPARE(motor->htim, motor->pwmChannel, 0);
        return;
    }
    
    /* 计算PWM占空比 */
    uint32_t arr = __HAL_TIM_GET_AUTORELOAD(motor->htim);
    uint32_t ccr = (speed * arr) / 1000;
    
    motor->currentSpeed = speed;
    __HAL_TIM_SET_COMPARE(motor->htim, motor->pwmChannel, ccr);
}

/**
  * @brief  设置电机方向
  * @param  motor: 电机对象指针
  * @param  dir: 方向
  * @retval None
  */
static void Motor_Private_SetDirection(Motor_t *motor, MotorState_t dir)
{
    if (motor == NULL || motor->dirPort == NULL) return;
    
    motor->state = dir;
    
    switch (dir) {
        case MOTOR_STATE_FORWARD:
            HAL_GPIO_WritePin(motor->dirPort, motor->dirPin, GPIO_PIN_SET);
            break;
        case MOTOR_STATE_BACKWARD:
            HAL_GPIO_WritePin(motor->dirPort, motor->dirPin, GPIO_PIN_RESET);
            break;
        case MOTOR_STATE_STOP:
        case MOTOR_STATE_BRAKE:
            HAL_GPIO_WritePin(motor->dirPort, motor->dirPin, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

/**
  * @brief  停止电机
  * @param  motor: 电机对象指针
  * @retval None
  */
static void Motor_Private_Stop(Motor_t *motor)
{
    if (motor == NULL) return;
    
    motor->state = MOTOR_STATE_STOP;
    motor->targetSpeed = 0;
    motor->currentSpeed = 0;
    
    if (motor->htim != NULL) {
        __HAL_TIM_SET_COMPARE(motor->htim, motor->pwmChannel, 0);
    }
}

/**
  * @brief  刹车
  * @param  motor: 电机对象指针
  * @retval None
  */
static void Motor_Private_Brake(Motor_t *motor)
{
    if (motor == NULL) return;
    
    motor->state = MOTOR_STATE_BRAKE;
    motor->targetSpeed = 0;
    motor->currentSpeed = 0;
    
    /* 刹车时设置方向引脚为低电平，PWM为0 */
    if (motor->dirPort != NULL) {
        HAL_GPIO_WritePin(motor->dirPort, motor->dirPin, GPIO_PIN_RESET);
    }
    if (motor->htim != NULL) {
        __HAL_TIM_SET_COMPARE(motor->htim, motor->pwmChannel, 0);
    }
}

/**
  * @brief  获取当前速度
  * @param  motor: 电机对象指针
  * @retval 当前速度值
  */
static int16_t Motor_Private_GetSpeed(Motor_t *motor)
{
    if (motor == NULL) return 0;
    return motor->currentSpeed;
}

/**
  * @brief  获取当前状态
  * @param  motor: 电机对象指针
  * @retval 当前状态
  */
static MotorState_t Motor_Private_GetState(Motor_t *motor)
{
    if (motor == NULL) return MOTOR_STATE_STOP;
    return motor->state;
}

/* 公共接口函数 */
void Motor_SetSpeed(Motor_t *motor, int16_t speed)
{
    if (motor != NULL && motor->vtable != NULL && motor->vtable->SetSpeed != NULL) {
        motor->vtable->SetSpeed(motor, speed);
    }
}

void Motor_SetDirection(Motor_t *motor, MotorState_t dir)
{
    if (motor != NULL && motor->vtable != NULL && motor->vtable->SetDirection != NULL) {
        motor->vtable->SetDirection(motor, dir);
    }
}

void Motor_Stop(Motor_t *motor)
{
    if (motor != NULL && motor->vtable != NULL && motor->vtable->Stop != NULL) {
        motor->vtable->Stop(motor);
    }
}

void Motor_Brake(Motor_t *motor)
{
    if (motor != NULL && motor->vtable != NULL && motor->vtable->Brake != NULL) {
        motor->vtable->Brake(motor);
    }
}

int16_t Motor_GetSpeed(Motor_t *motor)
{
    if (motor != NULL && motor->vtable != NULL && motor->vtable->GetSpeed != NULL) {
        return motor->vtable->GetSpeed(motor);
    }
    return 0;
}

MotorState_t Motor_GetState(Motor_t *motor)
{
    if (motor != NULL && motor->vtable != NULL && motor->vtable->GetState != NULL) {
        return motor->vtable->GetState(motor);
    }
    return MOTOR_STATE_STOP;
}

void Motor_Enable(Motor_t *motor)
{
    if (motor == NULL) return;
    motor->enabled = true;
}

void Motor_Disable(Motor_t *motor)
{
    if (motor == NULL) return;
    motor->enabled = false;
    Motor_Stop(motor);
}

bool Motor_IsEnabled(Motor_t *motor)
{
    if (motor == NULL) return false;
    return motor->enabled;
}

