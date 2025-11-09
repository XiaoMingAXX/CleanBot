/**
  ******************************************************************************
  * @file    bsp_tim.h
  * @brief   定时器板级支持包头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __BSP_TIM_H__
#define __BSP_TIM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "tim.h"
#include <stdint.h>

/* 定时器操作函数 */
#define BSP_TIM_StartPWM(htim, channel)     HAL_TIM_PWM_Start(htim, channel)
#define BSP_TIM_StopPWM(htim, channel)      HAL_TIM_PWM_Stop(htim, channel)
#define BSP_TIM_SetCompare(htim, channel, value)  __HAL_TIM_SET_COMPARE(htim, channel, value)
#define BSP_TIM_GetCompare(htim, channel)   __HAL_TIM_GET_COMPARE(htim, channel)
#define BSP_TIM_GetCounter(htim)            __HAL_TIM_GET_COUNTER(htim)
#define BSP_TIM_SetCounter(htim, value)     __HAL_TIM_SET_COUNTER(htim, value)
#define BSP_TIM_GetAutoreload(htim)         __HAL_TIM_GET_AUTORELOAD(htim)
#define BSP_TIM_SetAutoreload(htim, value)  __HAL_TIM_SET_AUTORELOAD(htim, value)

/* 函数声明 */
void BSP_TIM_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_TIM_H__ */

