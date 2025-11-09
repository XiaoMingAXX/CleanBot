/**
  ******************************************************************************
  * @file    usb_comm_task.h
  * @brief   USB通信任务头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __USB_COMM_TASK_H__
#define __USB_COMM_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cleanbot_config.h"

/* 函数声明 */
void USBCommTask_Init(void);
void USBCommTask_Run(void *argument);

#ifdef __cplusplus
}
#endif

#endif /* __USB_COMM_TASK_H__ */

