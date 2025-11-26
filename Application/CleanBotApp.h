/**
  ******************************************************************************
  * @file    CleanBotApp.h
  * @brief   扫地机器人应用层头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __CLEANBOT_APP_H__
#define __CLEANBOT_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "motor.h"
#include "encoder.h"
#include "pid_controller.h"
#include "ir_sensor.h"
#include "photo_gate.h"
#include "led.h"
#include "buzzer.h"
#include "usb_comm.h"
#include "motor_ctrl_task.h"
#include "ir_homing.h"
#include <stdint.h>
#include <stdbool.h>

/* 机器人状态 */
typedef enum {
    CLEANBOT_STATE_IDLE = 0,       /* 空闲 */
    CLEANBOT_STATE_CLEANING,       /* 清扫中 */
    CLEANBOT_STATE_CHARGING,       /* 充电中 */
    CLEANBOT_STATE_ERROR           /* 错误 */
} CleanBotState_t;

/* 机器人结构体 */
typedef struct {
    /* 电机 */
    Motor_t wheelMotorLeft;        /* 左轮电机 */
    Motor_t wheelMotorRight;       /* 右轮电机 */
    Motor_t brushMotorLeft;        /* 左边刷电机 */
    Motor_t brushMotorRight;       /* 右边刷电机 */
    Motor_t fanMotor;              /* 吸尘电机 */
    Motor_t pumpMotor;             /* 水箱增压电机 */
    
    /* 编码器 */
    Encoder_t encoderWheelLeft;    /* 左轮编码器 */
    Encoder_t encoderWheelRight;   /* 右轮编码器 */
    Encoder_t encoderFan;          /* 风机编码器 */
    
    /* PID控制器 */
    PIDController_t pidWheelLeft;  /* 左轮PID */
    PIDController_t pidWheelRight; /* 右轮PID */
    PIDController_t pidFan;        /* 风机PID */
    
    /* 传感器 */
    IR_Sensor_t irSensorLeft;      /* 左侧红外传感器 */
    IR_Sensor_t irSensorRight;     /* 右侧红外传感器 */
    IR_Sensor_t irSensorFrontLeft; /* 左前红外传感器 */
    IR_Sensor_t irSensorFrontRight;/* 右前红外传感器 */
    
    PhotoGate_t photoGateLeft;     /* 左侧光电门 */
    PhotoGate_t photoGateRight;    /* 右侧光电门 */
    
    /* 下视传感器状态 */
    bool underLeftSuspended;       /* 左前下视传感器悬空（高电平） */
    bool underRightSuspended;      /* 右前下视传感器悬空（高电平） */
    bool underCenterSuspended;     /* 中间下视传感器悬空（高电平） */
    
    /* 指示器 */
    LED_t led1;                    /* LED1 */
    LED_t led2;                    /* LED2 */
    LED_t led3;                    /* LED3 */
    LED_t led4;                    /* LED4 */
    Buzzer_t buzzer;               /* 蜂鸣器 */
    
    /* 通信 */
    USB_Comm_t usbComm;            /* USB通信 */
    
    /* 回冲定位 */
    IRHoming_t irHoming;           /* 红外回冲定位模块 */
    
    /* 状态 */
    CleanBotState_t state;         /* 当前状态 */
    bool enabled;                  /* 使能标志 */
} CleanBotApp_t;

/* 函数声明 */
void CleanBotApp_Init(CleanBotApp_t *app);
void CleanBotApp_Update(CleanBotApp_t *app);  /* 主更新函数，需要在任务中周期性调用 */
void CleanBotApp_Start(CleanBotApp_t *app);
void CleanBotApp_Stop(CleanBotApp_t *app);
void CleanBotApp_SetState(CleanBotApp_t *app, CleanBotState_t state);
CleanBotState_t CleanBotApp_GetState(CleanBotApp_t *app);

/* 控制函数（已废弃，请使用MotorCtrlTask接口） */
void CleanBotApp_SetWheelSpeed(CleanBotApp_t *app, int16_t leftSpeed, int16_t rightSpeed);
void CleanBotApp_SetBrushSpeed(CleanBotApp_t *app, int16_t leftSpeed, int16_t rightSpeed);
void CleanBotApp_SetFanSpeed(CleanBotApp_t *app, int16_t speed);
void CleanBotApp_SetPumpSpeed(CleanBotApp_t *app, int16_t speed);

/* 获取应用实例 */
CleanBotApp_t* CleanBotApp_GetInstance(void);

/* 测试函数 */
void CleanBotApp_Test_SetWheelSpeedMs(float leftSpeedMs, float rightSpeedMs);
void CleanBotApp_Test_SetBrushMotor(uint8_t leftLevel, uint8_t rightLevel);
void CleanBotApp_Test_SetPumpMotor(uint8_t level);
void CleanBotApp_Test_SetFanMotor(uint8_t level);

#ifdef __cplusplus
}
#endif

#endif /* __CLEANBOT_APP_H__ */

