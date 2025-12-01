/**
  ******************************************************************************
  * @file    hw_config.h
  * @brief   硬件配置头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  * @attention
  * 此文件包含所有硬件相关的配置定义，包括引脚定义、定时器配置等
  ******************************************************************************
  */

#ifndef __HW_CONFIG_H__
#define __HW_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>

/* ============================================
   引脚定义 (Pin Definitions)
   ============================================ */

/* LED控制引脚 */
#define LED1_PORT              LEDCONTROL1_GPIO_Port
#define LED1_PIN               LEDCONTROL1_Pin
#define LED2_PORT              LEDCONTROL2_GPIO_Port
#define LED2_PIN               LEDCONTROL2_Pin
#define LED3_PORT              LEDCONTROL3_GPIO_Port
#define LED3_PIN               LEDCONTROL3_Pin
#define LED4_PORT              LEDCONTROL4_GPIO_Port
#define LED4_PIN               LEDCONTROL4_Pin

/* 蜂鸣器控制引脚 */
#define BUZZER_PORT            CONTROLBUZZER_GPIO_Port
#define BUZZER_PIN             CONTROLBUZZER_Pin

/* 电机控制引脚 */
/* 轮电机1 (左轮) */
#define WHEEL_MOTOR1_PWM_PORT  MOTOR1PWM_GPIO_Port
#define WHEEL_MOTOR1_PWM_PIN   MOTOR1PWM_Pin
#define WHEEL_MOTOR1_DIR_PORT  MOTOR1DRC_GPIO_Port
#define WHEEL_MOTOR1_DIR_PIN   MOTOR1DRC_Pin

/* 轮电机2 (右轮) */
#define WHEEL_MOTOR2_PWM_PORT  MOTOR2PWM_GPIO_Port
#define WHEEL_MOTOR2_PWM_PIN   MOTOR2PWM_Pin
#define WHEEL_MOTOR2_DIR_PORT  MOTOR2DRC_GPIO_Port
#define WHEEL_MOTOR2_DIR_PIN   MOTOR2DRC_Pin

/* 边刷电机1 (左边刷) */
#define BRUSH_MOTOR1_PWM_PORT  CleanMotor1_PWM_GPIO_Port
#define BRUSH_MOTOR1_PWM_PIN   CleanMotor1_PWM_Pin

/* 边刷电机2 (右边刷) */
#define BRUSH_MOTOR2_PWM_PORT  CleanMotor2_PWM_GPIO_Port
#define BRUSH_MOTOR2_PWM_PIN   CleanMotor2_PWM_Pin

/* 吸尘电机 */
#define FAN_MOTOR_PWM_PORT     FAN_PWM_GPIO_Port
#define FAN_MOTOR_PWM_PIN      FAN_PWM_Pin

/* 水箱增压电机 */
#define PUMP_MOTOR_PWM_PORT    PUMP_PWM_GPIO_Port
#define PUMP_MOTOR_PWM_PIN     PUMP_PWM_Pin

/* 编码器引脚 */
/* 左轮编码器 - 使用TIM2 */
#define ENCODER_WHEEL_LEFT_TIM  &htim2

/* 右轮编码器 - 使用TIM3 */
#define ENCODER_WHEEL_RIGHT_TIM &htim3

/* 风机编码器 - 使用TIM4 */
#define ENCODER_FAN_TIM        &htim4

/* 红外传感器引脚 */
#define IR_SENSOR_LEFT_PORT    L_RECEIVE_GPIO_Port
#define IR_SENSOR_LEFT_PIN     L_RECEIVE_Pin
#define IR_SENSOR_RIGHT_PORT   R_RECEIVE_GPIO_Port
#define IR_SENSOR_RIGHT_PIN    R_RECEIVE_Pin
#define IR_SENSOR_FRONT_LEFT_PORT   L_FOLLOW_CHECK_SIGNAL_GPIO_Port
#define IR_SENSOR_FRONT_LEFT_PIN    L_FOLLOW_CHECK_SIGNAL_Pin
#define IR_SENSOR_FRONT_RIGHT_PORT  R_FOLLOW_CHECK_SIGNAL_GPIO_Port
#define IR_SENSOR_FRONT_RIGHT_PIN   R_FOLLOW_CHECK_SIGNAL_Pin

/* 光电门引脚 */
#define PHOTO_GATE_LEFT_PORT   IFHIT_L_GPIO_Port
#define PHOTO_GATE_LEFT_PIN    IFHIT_L_Pin
#define PHOTO_GATE_RIGHT_PORT  IFHIT_R_GPIO_Port
#define PHOTO_GATE_RIGHT_PIN   IFHIT_R_Pin

/* ============================================
   定时器配置 (Timer Configuration)
   ============================================ */

/* PWM定时器配置 */
/* 轮电机1 PWM - 需要根据实际配置修改 */
extern TIM_HandleTypeDef htim1;  /* 假设使用TIM1 */
#define WHEEL_MOTOR1_PWM_TIM    &htim1
#define WHEEL_MOTOR1_PWM_CH     TIM_CHANNEL_1

/* 轮电机2 PWM - 需要根据实际配置修改 */
#define WHEEL_MOTOR2_PWM_TIM    &htim1
#define WHEEL_MOTOR2_PWM_CH     TIM_CHANNEL_2

/* 边刷电机1 PWM - 需要根据实际配置修改 */
extern TIM_HandleTypeDef htim8;  /* 假设使用TIM8 */
#define BRUSH_MOTOR1_PWM_TIM    &htim8
#define BRUSH_MOTOR1_PWM_CH     TIM_CHANNEL_1

/* 边刷电机2 PWM - 需要根据实际配置修改 */
#define BRUSH_MOTOR2_PWM_TIM    &htim8
#define BRUSH_MOTOR2_PWM_CH     TIM_CHANNEL_2

/* 吸尘电机 PWM - 需要根据实际配置修改 */
extern TIM_HandleTypeDef htim9;  /* 假设使用TIM9 */
#define FAN_MOTOR_PWM_TIM       &htim9
#define FAN_MOTOR_PWM_CH        TIM_CHANNEL_1

/* 水箱增压电机 PWM - 需要根据实际配置修改 */
extern TIM_HandleTypeDef htim10;  /* 假设使用TIM10 */
#define PUMP_MOTOR_PWM_TIM      &htim10
#define PUMP_MOTOR_PWM_CH       TIM_CHANNEL_1

/* 蜂鸣器 PWM - 需要根据实际配置修改 */
#define BUZZER_PWM_TIM          &htim11  /* 假设使用TIM11 */
#define BUZZER_PWM_CH           TIM_CHANNEL_1

/* ============================================
   编码器配置 (Encoder Configuration)
   ============================================ */

/* 编码器参数 */
#define ENCODER_WHEEL_PPR       2260      /* 轮电机编码器每转脉冲数 */
#define ENCODER_WHEEL_GEAR_RATIO 1    /* 轮电机减速比 */
#define ENCODER_WHEEL_PULSE_PER_METER  11050  /* 轮电机每米脉冲数（需要实际测量后设置） */

#define ENCODER_FAN_PPR         7       /* 风机编码器每转脉冲数 */
#define ENCODER_FAN_GEAR_RATIO  1       /* 风机减速比 */

/* ============================================
   PID参数配置 (PID Configuration)
   ============================================ */

/* 左轮PID参数 */
#define PID_WHEEL_LEFT_KP       8.0f
#define PID_WHEEL_LEFT_KI       0.1f
#define PID_WHEEL_LEFT_KD       0.05f
#define PID_WHEEL_LEFT_OUT_MAX  1000.0f
#define PID_WHEEL_LEFT_OUT_MIN  -1000.0f

/* 右轮PID参数 */
#define PID_WHEEL_RIGHT_KP      8.0f
#define PID_WHEEL_RIGHT_KI      0.1f
#define PID_WHEEL_RIGHT_KD      0.05f
#define PID_WHEEL_RIGHT_OUT_MAX 1000.0f
#define PID_WHEEL_RIGHT_OUT_MIN -1000.0f

/* 风机PID参数 */
#define PID_FAN_KP              1.0f
#define PID_FAN_KI              0.1f
#define PID_FAN_KD              0.05f
#define PID_FAN_OUT_MAX         1000.0f
#define PID_FAN_OUT_MIN         0.0f

/* ============================================
   电机速度限制 (Motor Speed Limits)
   ============================================ */

#define MOTOR_SPEED_MAX         1000    /* 最大速度 (0-1000) */
#define MOTOR_SPEED_MIN         0       /* 最小速度 */
#define WHEEL_MOTOR_SPEED_MAX   1000    /* 轮电机最大速度 */
#define BRUSH_MOTOR_SPEED_MAX   1000     /* 边刷电机最大速度 */
#define FAN_MOTOR_SPEED_MAX     1000    /* 吸尘电机最大速度 */
#define PUMP_MOTOR_SPEED_MAX    1000     /* 水泵电机最大速度 */

/* 边刷电机速度档位 (0-100) */
#define BRUSH_MOTOR_SPEED_OFF   0       /* 关闭 */
#define BRUSH_MOTOR_SPEED_LOW   500      /* 低速 */
#define BRUSH_MOTOR_SPEED_HIGH  800      /* 高速 */

/* 水箱增压电机速度档位 (0-100) */
#define PUMP_MOTOR_SPEED_OFF    0       /* 关闭 */
#define PUMP_MOTOR_SPEED_LOW   300      /* 低速 */
#define PUMP_MOTOR_SPEED_MEDIUM 500      /* 中速 */
#define PUMP_MOTOR_SPEED_HIGH   700      /* 高速 */
#define PUMP_MOTOR_SPEED_TURBO  800     /* 超高速 */
#define PUMP_MOTOR_SPEED_ULTRA  1000     /* 最大水量 */

/* 风机速度档位 (RPM) */
#define FAN_MOTOR_SPEED_OFF     0       /* 关闭 */
#define FAN_MOTOR_SPEED_1       300    /* 档位1 */
#define FAN_MOTOR_SPEED_2       500    /* 档位2 */
#define FAN_MOTOR_SPEED_3       700    /* 档位3 */
#define FAN_MOTOR_SPEED_4       800    /* 档位4 */
#define FAN_MOTOR_SPEED_5       1000    /* 档位5 */

/* ============================================
   USB通信配置 (USB Communication Config)
   ============================================ */

#define USB_COMM_RX_BUFFER_SIZE    512
#define USB_COMM_TX_BUFFER_SIZE    512
#define USB_COMM_PACKET_SIZE       64

/* ============================================
   系统配置 (System Configuration)
   ============================================ */

/* 系统时钟频率 (Hz) */
#define SYSTEM_CLOCK_FREQ       168000000UL

/* 控制周期 (ms) */
#define CONTROL_PERIOD_MS       10
#define PID_UPDATE_PERIOD_MS    10
#define SENSOR_UPDATE_PERIOD_MS 50

/* ============================================
   中断优先级配置 (Interrupt Priority)
   ============================================ */

/* FreeRTOS中断优先级 */
#define FREERTOS_SYSTICK_PRIORITY    15
#define FREERTOS_PENDSV_PRIORITY     15

/* 编码器中断优先级 */
#define ENCODER_IRQ_PRIORITY        5

/* USB中断优先级 */
#define USB_IRQ_PRIORITY            6

/* 外部中断优先级 */
#define EXTI_IRQ_PRIORITY           7

#ifdef __cplusplus
}
#endif

#endif /* __HW_CONFIG_H__ */

