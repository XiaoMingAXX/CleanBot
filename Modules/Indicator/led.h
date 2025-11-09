/**
  ******************************************************************************
  * @file    led.h
  * @brief   LED驱动头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __LED_H__
#define __LED_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* LED类型 */
typedef enum {
    LED_1 = 0,
    LED_2,
    LED_3,
    LED_4
} LEDType_t;

/* LED状态 */
typedef enum {
    LED_OFF = 0,
    LED_ON,
    LED_TOGGLE
} LEDState_t;

/* LED结构体 */
typedef struct {
    LEDType_t type;         /* LED类型 */
    GPIO_TypeDef *port;     /* GPIO端口 */
    uint16_t pin;           /* GPIO引脚 */
    LEDState_t state;       /* 当前状态 */
    bool enabled;           /* 使能标志 */
} LED_t;

/* 函数声明 */
void LED_Init(LED_t *led, LEDType_t type, GPIO_TypeDef *port, uint16_t pin);
void LED_On(LED_t *led);
void LED_Off(LED_t *led);
void LED_Toggle(LED_t *led);
void LED_SetState(LED_t *led, LEDState_t state);
LEDState_t LED_GetState(LED_t *led);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H__ */

