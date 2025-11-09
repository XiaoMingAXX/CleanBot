/**
  ******************************************************************************
  * @file    cleanbot_config.h
  * @brief   CleanBot项目总配置文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  * @attention
  * 此文件是项目的总配置文件，包含所有模块的头文件引用
  * 建议在应用层代码中只包含此文件，而不是包含各个模块的头文件
  ******************************************************************************
  */

#ifndef __CLEANBOT_CONFIG_H__
#define __CLEANBOT_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================
   STM32 HAL库
   ============================================ */
#include "main.h"
#include "stm32f4xx_hal.h"

/* ============================================
   FreeRTOS
   ============================================ */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"

/* ============================================
   硬件配置
   ============================================ */
#include "hw_config.h"
#include "system_config.h"

/* ============================================
   公共定义和工具
   ============================================ */
#include "common_def.h"
#include "common_utils.h"

/* ============================================
   功能模块
   ============================================ */
/* 电机控制模块 */
#include "motor.h"

/* 编码器模块 */
#include "encoder.h"

/* PID控制器模块 */
#include "pid_controller.h"

/* 传感器模块 */
#include "ir_sensor.h"
#include "photo_gate.h"

/* 指示器模块 */
#include "led.h"
#include "buzzer.h"

/* 通信模块 */
#include "usb_comm.h"

/* 工具模块 */
#include "ring_buffer.h"
#include "nec_decode.h"

/* ============================================
   应用层
   ============================================ */
#include "CleanBotApp.h"

/* ============================================
   任务模块
   ============================================ */
#include "sensor_manager.h"
#include "sensor_task.h"
#include "motor_ctrl_task.h"
#include "usb_comm_task.h"

#ifdef __cplusplus
}
#endif

#endif /* __CLEANBOT_CONFIG_H__ */

