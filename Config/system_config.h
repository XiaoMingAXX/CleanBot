/**
  ******************************************************************************
  * @file    system_config.h
  * @brief   系统配置头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  * @attention
  * 此文件包含系统级别的配置，如任务优先级、堆栈大小等
  ******************************************************************************
  */

#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "FreeRTOSConfig.h"
#include <stdint.h>

/* ============================================
   FreeRTOS任务配置
   ============================================ */

/* 任务优先级定义 */
#define TASK_PRIORITY_IDLE          0
#define TASK_PRIORITY_LOW           1
#define TASK_PRIORITY_NORMAL        2
#define TASK_PRIORITY_HIGH          3
#define TASK_PRIORITY_REALTIME      4

/* 应用任务优先级 */
#define TASK_PRIORITY_CLEANBOT_APP  TASK_PRIORITY_NORMAL
#define TASK_PRIORITY_MOTOR_CTRL    TASK_PRIORITY_HIGH
#define TASK_PRIORITY_PID_CTRL      TASK_PRIORITY_HIGH
#define TASK_PRIORITY_SENSOR        TASK_PRIORITY_NORMAL
#define TASK_PRIORITY_USB_COMM      TASK_PRIORITY_LOW
#define TASK_PRIORITY_MONITOR       TASK_PRIORITY_LOW

/* 任务堆栈大小 (字, 32位) */
#define TASK_STACK_SIZE_SMALL       128
#define TASK_STACK_SIZE_NORMAL      256
#define TASK_STACK_SIZE_LARGE       512
#define TASK_STACK_SIZE_XLARGE      1024

/* 应用任务堆栈大小 */
#define TASK_STACK_SIZE_CLEANBOT_APP    TASK_STACK_SIZE_LARGE
#define TASK_STACK_SIZE_MOTOR_CTRL      TASK_STACK_SIZE_NORMAL
#define TASK_STACK_SIZE_PID_CTRL        TASK_STACK_SIZE_NORMAL
#define TASK_STACK_SIZE_SENSOR          TASK_STACK_SIZE_NORMAL
#define TASK_STACK_SIZE_USB_COMM        TASK_STACK_SIZE_NORMAL
#define TASK_STACK_SIZE_MONITOR         TASK_STACK_SIZE_SMALL

/* ============================================
   任务周期配置 (ms)
   ============================================ */

#define TASK_PERIOD_CLEANBOT_APP    10      /* 主应用任务周期 */
#define TASK_PERIOD_MOTOR_CTRL      5       /* 电机控制任务周期 */
#define TASK_PERIOD_PID_CTRL        10      /* PID控制任务周期 */
#define TASK_PERIOD_SENSOR          50      /* 传感器读取任务周期 */
#define TASK_PERIOD_USB_COMM        20      /* USB通信任务周期 */
#define TASK_PERIOD_MONITOR         1000    /* 监控任务周期 */

/* ============================================
   调试配置
   ============================================ */

/* 调试开关 */
#define DEBUG_ENABLE                1
#define DEBUG_MOTOR                 1
#define DEBUG_ENCODER               1
#define DEBUG_PID                   1
#define DEBUG_SENSOR                1
#define DEBUG_USB_COMM              1

/* 调试输出方式 */
#define DEBUG_USE_USB               1
#define DEBUG_USE_UART              0

/* ============================================
   错误处理配置
   ============================================ */

/* 错误代码定义 */
typedef enum {
    ERROR_NONE = 0,
    ERROR_MOTOR_FAULT,
    ERROR_ENCODER_FAULT,
    ERROR_SENSOR_FAULT,
    ERROR_COMM_FAULT,
    ERROR_OVER_CURRENT,
    ERROR_OVER_TEMPERATURE,
    ERROR_BATTERY_LOW,
    ERROR_UNKNOWN
} ErrorCode_t;

/* 错误处理策略 */
#define ERROR_HANDLER_ENABLE        1
#define ERROR_AUTO_RECOVERY         1
#define ERROR_MAX_RETRY             3

/* ============================================
   看门狗配置
   ============================================ */

#define WATCHDOG_ENABLE             1
#define WATCHDOG_TIMEOUT_MS          5000

/* ============================================
   内存配置
   ============================================ */

/* 堆内存配置 */
#define HEAP_SIZE                   8192    /* FreeRTOS堆大小 (字节) */

/* 缓冲区大小 */
#define RING_BUFFER_SIZE            256
#define COMM_BUFFER_SIZE            512

#ifdef __cplusplus
}
#endif

#endif /* __SYSTEM_CONFIG_H__ */

