/**
  ******************************************************************************
  * @file    sensor_task.h
  * @brief   传感器任务头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __SENSOR_TASK_H__
#define __SENSOR_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cleanbot_config.h"

/* 函数声明 */
void SensorTask_Init(void);
void SensorTask_Run(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TASK_H__ */

