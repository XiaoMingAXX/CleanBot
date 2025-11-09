/**
  ******************************************************************************
  * @file    led.c
  * @brief   LED驱动实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "led.h"

/**
  * @brief  初始化LED
  * @param  led: LED对象指针
  * @param  type: LED类型
  * @param  port: GPIO端口
  * @param  pin: GPIO引脚
  * @retval None
  */
void LED_Init(LED_t *led, LEDType_t type, GPIO_TypeDef *port, uint16_t pin)
{
    if (led == NULL) return;
    
    led->type = type;
    led->port = port;
    led->pin = pin;
    led->state = LED_OFF;
    led->enabled = true;
    
    /* 初始状态：关闭 */
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
}

/**
  * @brief  打开LED
  * @param  led: LED对象指针
  * @retval None
  */
void LED_On(LED_t *led)
{
    if (led == NULL || led->port == NULL) return;
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_SET);
    led->state = LED_ON;
}

/**
  * @brief  关闭LED
  * @param  led: LED对象指针
  * @retval None
  */
void LED_Off(LED_t *led)
{
    if (led == NULL || led->port == NULL) return;
    HAL_GPIO_WritePin(led->port, led->pin, GPIO_PIN_RESET);
    led->state = LED_OFF;
}

/**
  * @brief  切换LED状态
  * @param  led: LED对象指针
  * @retval None
  */
void LED_Toggle(LED_t *led)
{
    if (led == NULL || led->port == NULL) return;
    HAL_GPIO_TogglePin(led->port, led->pin);
    if (led->state == LED_ON) {
        led->state = LED_OFF;
    } else {
        led->state = LED_ON;
    }
}

/**
  * @brief  设置LED状态
  * @param  led: LED对象指针
  * @param  state: LED状态
  * @retval None
  */
void LED_SetState(LED_t *led, LEDState_t state)
{
    if (led == NULL) return;
    
    switch (state) {
        case LED_ON:
            LED_On(led);
            break;
        case LED_OFF:
            LED_Off(led);
            break;
        case LED_TOGGLE:
            LED_Toggle(led);
            break;
        default:
            break;
    }
}

/**
  * @brief  获取LED状态
  * @param  led: LED对象指针
  * @retval LED状态
  */
LEDState_t LED_GetState(LED_t *led)
{
    if (led == NULL) return LED_OFF;
    return led->state;
}

