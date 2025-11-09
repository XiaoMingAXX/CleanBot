/**
  ******************************************************************************
  * @file    bsp_gpio.h
  * @brief   GPIO板级支持包头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __BSP_GPIO_H__
#define __BSP_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* GPIO操作函数 */
#define BSP_GPIO_SetPin(port, pin)      HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET)
#define BSP_GPIO_ResetPin(port, pin)    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET)
#define BSP_GPIO_TogglePin(port, pin)   HAL_GPIO_TogglePin(port, pin)
#define BSP_GPIO_ReadPin(port, pin)     HAL_GPIO_ReadPin(port, pin)

/* 函数声明 */
void BSP_GPIO_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __BSP_GPIO_H__ */

