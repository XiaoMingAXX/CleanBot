/**
  ******************************************************************************
  * @file    imu_task.h
  * @brief   IMU任务（WIT9011，USART3+DMA+空闲中断）头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __IMU_TASK_H__
#define __IMU_TASK_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* 任务入口 */
void IMUTask_Run(void *argument);

/* 获取最近一次解算后的姿态/角速度/加速度（单位：deg、deg/s、g） */
void IMUTask_GetEuler(float *roll, float *pitch, float *yaw);
void IMUTask_GetGyro(float *gx, float *gy, float *gz);
void IMUTask_GetAccel(float *ax, float *ay, float *az);

#ifdef __cplusplus
}
#endif

#endif /* __IMU_TASK_H__ */


