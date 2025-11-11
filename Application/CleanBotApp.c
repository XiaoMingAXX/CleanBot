/**
  ******************************************************************************
  * @file    CleanBotApp.c
  * @brief   扫地机器人应用层实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "CleanBotApp.h"
#include "tim.h"
#include "gpio.h"
#include "usb_device.h"
#include "cmsis_os.h"

/* 外部全局变量（需要在CubeMX生成的文件中定义） */
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim10;

/* 全局应用对象 */
static CleanBotApp_t g_CleanBotApp;
CleanBotApp_t *g_pCleanBotApp = &g_CleanBotApp;

/**
  * @brief  初始化扫地机器人应用
  * @param  app: 应用对象指针
  * @retval None
  */
void CleanBotApp_Init(CleanBotApp_t *app)
{
    if (app == NULL) return;
    
    /* 初始化电机 */
    /* 左轮电机 - 使用双PWM模式：INA (TIM4_CH1) 和 INB (TIM4_CH2) */
    Motor_InitDualPWM(&app->wheelMotorLeft, MOTOR_TYPE_WHEEL, &htim4, 
                      TIM_CHANNEL_3, TIM_CHANNEL_4);
    
    /* 右轮电机 - 使用双PWM模式：INA (TIM4_CH3) 和 INB (TIM4_CH4) */
    Motor_InitDualPWM(&app->wheelMotorRight, MOTOR_TYPE_WHEEL, &htim4, 
                      TIM_CHANNEL_1, TIM_CHANNEL_2);
    
    /* 左边刷电机 */
    Motor_Init(&app->brushMotorLeft, MOTOR_TYPE_BRUSH, &htim3, TIM_CHANNEL_3, 
               NULL, 0);  /* 如果边刷电机不需要方向控制，传NULL */
    
    /* 右边刷电机 */
    Motor_Init(&app->brushMotorRight, MOTOR_TYPE_BRUSH, &htim3, TIM_CHANNEL_4, 
               NULL, 0);
    
    /* 吸尘电机 */
    Motor_Init(&app->fanMotor, MOTOR_TYPE_FAN, &htim3, TIM_CHANNEL_2, 
               NULL, 0);
    
    /* 水箱增压电机 */
    Motor_Init(&app->pumpMotor, MOTOR_TYPE_PUMP, &htim3, TIM_CHANNEL_1, 
               NULL, 0);
    
    /* 初始化编码器 */
    /* 左轮编码器 - 需要根据实际硬件配置调整定时器和参数 */
    Encoder_Init(&app->encoderWheelLeft, ENCODER_TYPE_WHEEL_LEFT, &htim2, 
                 ENCODER_WHEEL_PPR, ENCODER_WHEEL_GEAR_RATIO);
    Encoder_SetPulsePerMeter(&app->encoderWheelLeft, ENCODER_WHEEL_PULSE_PER_METER);
    
    /* 右轮编码器 */
    Encoder_Init(&app->encoderWheelRight, ENCODER_TYPE_WHEEL_RIGHT, &htim1, 
                 ENCODER_WHEEL_PPR, ENCODER_WHEEL_GEAR_RATIO);
    Encoder_SetPulsePerMeter(&app->encoderWheelRight, ENCODER_WHEEL_PULSE_PER_METER);
    
    /* 风机编码器 */
    Encoder_Init(&app->encoderFan, ENCODER_TYPE_FAN, &htim5, 
                 ENCODER_FAN_PPR, ENCODER_FAN_GEAR_RATIO);
    
    /* 启动编码器 */
    Encoder_Start(&app->encoderWheelLeft);
    Encoder_Start(&app->encoderWheelRight);
    Encoder_Start(&app->encoderFan);
    
    /* 初始化PID控制器 */
    /* 左轮PID - 需要根据实际调参 */
    PID_Init(&app->pidWheelLeft, PID_WHEEL_LEFT_KP, PID_WHEEL_LEFT_KI, PID_WHEEL_LEFT_KD);
    PID_SetOutputLimit(&app->pidWheelLeft, PID_WHEEL_LEFT_OUT_MIN, PID_WHEEL_LEFT_OUT_MAX);
    
    /* 右轮PID */
    PID_Init(&app->pidWheelRight, PID_WHEEL_RIGHT_KP, PID_WHEEL_RIGHT_KI, PID_WHEEL_RIGHT_KD);
    PID_SetOutputLimit(&app->pidWheelRight, PID_WHEEL_RIGHT_OUT_MIN, PID_WHEEL_RIGHT_OUT_MAX);
    
    /* 风机PID */
    PID_Init(&app->pidFan, PID_FAN_KP, PID_FAN_KI, PID_FAN_KD);
    PID_SetOutputLimit(&app->pidFan, PID_FAN_OUT_MIN, PID_FAN_OUT_MAX);
    
    /* 初始化红外传感器 */
    IR_Sensor_Init(&app->irSensorLeft, IR_SENSOR_LEFT, 
                   L_RECEIVE_GPIO_Port, L_RECEIVE_Pin);
    IR_Sensor_Init(&app->irSensorRight, IR_SENSOR_RIGHT, 
                   R_RECEIVE_GPIO_Port, R_RECEIVE_Pin);
    IR_Sensor_Init(&app->irSensorFrontLeft, IR_SENSOR_FRONT_LEFT, 
                   L_FOLLOW_CHECK_SIGNAL_GPIO_Port, L_FOLLOW_CHECK_SIGNAL_Pin);
    IR_Sensor_Init(&app->irSensorFrontRight, IR_SENSOR_FRONT_RIGHT, 
                   R_FOLLOW_CHECK_SIGNAL_GPIO_Port, R_FOLLOW_CHECK_SIGNAL_Pin);
    
    /* 使能红外传感器 */
    IR_Sensor_Enable(&app->irSensorLeft);
    IR_Sensor_Enable(&app->irSensorRight);
    IR_Sensor_Enable(&app->irSensorFrontLeft);
    IR_Sensor_Enable(&app->irSensorFrontRight);
    
    /* 初始化光电门 */
    PhotoGate_Init(&app->photoGateLeft, PHOTO_GATE_LEFT, 
                   IFHIT_L_GPIO_Port, IFHIT_L_Pin);
    PhotoGate_Init(&app->photoGateRight, PHOTO_GATE_RIGHT, 
                   IFHIT_R_GPIO_Port, IFHIT_R_Pin);
    
    /* 使能光电门 */
    PhotoGate_Enable(&app->photoGateLeft);
    PhotoGate_Enable(&app->photoGateRight);
    
    /* 初始化LED */
    LED_Init(&app->led1, LED_1, LEDCONTROL1_GPIO_Port, LEDCONTROL1_Pin);
    LED_Init(&app->led2, LED_2, LEDCONTROL2_GPIO_Port, LEDCONTROL2_Pin);
    LED_Init(&app->led3, LED_3, LEDCONTROL3_GPIO_Port, LEDCONTROL3_Pin);
    LED_Init(&app->led4, LED_4, LEDCONTROL4_GPIO_Port, LEDCONTROL4_Pin);
    
    /* 初始化蜂鸣器 */
    Buzzer_Init(&app->buzzer, CONTROLBUZZER_GPIO_Port, CONTROLBUZZER_Pin, 
                &htim10, TIM_CHANNEL_1);  /* 需要根据实际硬件调整 */
    
    /* 初始化USB通信 */
    USB_Comm_Init(&app->usbComm);
    USB_Comm_Enable(&app->usbComm);
    
    /* 初始化状态 */
    app->state = CLEANBOT_STATE_IDLE;
    app->enabled = false;
}

/**
  * @brief  更新应用（需要在任务中周期性调用）
  * @param  app: 应用对象指针
  * @retval None
  */
void CleanBotApp_Update(CleanBotApp_t *app)
{
    /* 此函数已不再使用，电机控制和传感器处理已移至独立任务 */
    /* 保留此函数以保持接口兼容性 */
}

/**
  * @brief  启动应用
  * @param  app: 应用对象指针
  * @retval None
  */
void CleanBotApp_Start(CleanBotApp_t *app)
{
    if (app == NULL) return;
    
    app->enabled = true;
    app->state = CLEANBOT_STATE_CLEANING;
    
    /* 使能电机 */
    Motor_Enable(&app->wheelMotorLeft);
    Motor_Enable(&app->wheelMotorRight);
    Motor_Enable(&app->brushMotorLeft);
    Motor_Enable(&app->brushMotorRight);
    Motor_Enable(&app->fanMotor);
    Motor_Enable(&app->pumpMotor);
    
    /* 蜂鸣提示 */
    Buzzer_Beep(&app->buzzer, 2000, 100);
}

/**
  * @brief  停止应用
  * @param  app: 应用对象指针
  * @retval None
  */
void CleanBotApp_Stop(CleanBotApp_t *app)
{
    if (app == NULL) return;
    
    /* 停止所有电机 */
    Motor_Stop(&app->wheelMotorLeft);
    Motor_Stop(&app->wheelMotorRight);
    Motor_Stop(&app->brushMotorLeft);
    Motor_Stop(&app->brushMotorRight);
    Motor_Stop(&app->fanMotor);
    Motor_Stop(&app->pumpMotor);
    
    app->state = CLEANBOT_STATE_IDLE;
    app->enabled = false;
}

/**
  * @brief  设置状态
  * @param  app: 应用对象指针
  * @param  state: 状态
  * @retval None
  */
void CleanBotApp_SetState(CleanBotApp_t *app, CleanBotState_t state)
{
    if (app == NULL) return;
    app->state = state;
}

/**
  * @brief  获取状态
  * @param  app: 应用对象指针
  * @retval 状态
  */
CleanBotState_t CleanBotApp_GetState(CleanBotApp_t *app)
{
    if (app == NULL) return CLEANBOT_STATE_IDLE;
    return app->state;
}

/**
  * @brief  设置轮子速度
  * @param  app: 应用对象指针
  * @param  leftSpeed: 左轮速度 (0-1000)
  * @param  rightSpeed: 右轮速度 (0-1000)
  * @retval None
  */
void CleanBotApp_SetWheelSpeed(CleanBotApp_t *app, int16_t leftSpeed, int16_t rightSpeed)
{
    /* 此函数已废弃，请使用MotorCtrlTask_SetWheelSpeed */
    /* 保留此函数以保持接口兼容性 */
    (void)app;
    (void)leftSpeed;
    (void)rightSpeed;
}

/**
  * @brief  设置边刷速度
  * @param  app: 应用对象指针
  * @param  leftSpeed: 左边刷速度 (0-1000)
  * @param  rightSpeed: 右边刷速度 (0-1000)
  * @retval None
  */
void CleanBotApp_SetBrushSpeed(CleanBotApp_t *app, int16_t leftSpeed, int16_t rightSpeed)
{
    /* 此函数已废弃，请使用MotorCtrlTask_SetBrushMotor */
    /* 保留此函数以保持接口兼容性 */
    (void)app;
    (void)leftSpeed;
    (void)rightSpeed;
}

/**
  * @brief  设置风机速度
  * @param  app: 应用对象指针
  * @param  speed: 速度 (0-1000)
  * @retval None
  */
void CleanBotApp_SetFanSpeed(CleanBotApp_t *app, int16_t speed)
{
    /* 此函数已废弃，请使用MotorCtrlTask_SetFanMotor */
    /* 保留此函数以保持接口兼容性 */
    (void)app;
    (void)speed;
}

/**
  * @brief  设置水泵速度
  * @param  app: 应用对象指针
  * @param  speed: 速度 (0-1000)
  * @retval None
  */
void CleanBotApp_SetPumpSpeed(CleanBotApp_t *app, int16_t speed)
{
    /* 此函数已废弃，请使用MotorCtrlTask_SetPumpMotor */
    /* 保留此函数以保持接口兼容性 */
    (void)app;
    (void)speed;
}

/* 获取全局应用对象 */
CleanBotApp_t* CleanBotApp_GetInstance(void)
{
    return &g_CleanBotApp;
}

/**
 * @brief  测试函数：设置轮电机速度（m/s）
 */
void CleanBotApp_Test_SetWheelSpeedMs(float leftSpeedMs, float rightSpeedMs)
{
    MotorCtrlTask_SetWheelSpeed(leftSpeedMs, rightSpeedMs);
}

/**
 * @brief  测试函数：设置边刷电机
 */
void CleanBotApp_Test_SetBrushMotor(uint8_t leftLevel, uint8_t rightLevel)
{
    MotorCtrlTask_SetBrushMotor((BrushMotorLevel_t)leftLevel, (BrushMotorLevel_t)rightLevel);
}

/**
 * @brief  测试函数：设置水泵电机
 */
void CleanBotApp_Test_SetPumpMotor(uint8_t level)
{
    MotorCtrlTask_SetPumpMotor((PumpMotorLevel_t)level);
}

/**
 * @brief  测试函数：设置风机
 */
void CleanBotApp_Test_SetFanMotor(uint8_t level)
{
    MotorCtrlTask_SetFanMotor((FanMotorLevel_t)level);
}

