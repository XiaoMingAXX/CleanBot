/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    gpio.c
  * @brief   This file provides code for the configuration
  *          of all used GPIO pins.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "gpio.h"

/* USER CODE BEGIN 0 */
#include "sensor_manager.h"
/* USER CODE END 0 */

/*----------------------------------------------------------------------------*/
/* Configure GPIO                                                             */
/*----------------------------------------------------------------------------*/
/* USER CODE BEGIN 1 */

/**
  * @brief  EXTI line detection callback.
  * @param  GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    switch (GPIO_Pin) {
        case L_RECEIVE_Pin:
            SensorManager_IRQHandler_IR_Left();
            break;
        case R_RECEIVE_Pin:
            SensorManager_IRQHandler_IR_Right();
            break;
        case L_FOLLOW_CHECK_SIGNAL_Pin:
            SensorManager_IRQHandler_IR_FrontLeft();
            break;
        case R_FOLLOW_CHECK_SIGNAL_Pin:
            SensorManager_IRQHandler_IR_FrontRight();
            break;
        case IFHIT_L_Pin:
            SensorManager_IRQHandler_PhotoGate_Left();
            break;
        case IFHIT_R_Pin:
            SensorManager_IRQHandler_PhotoGate_Right();
            break;
        case BUTTON1_Pin:
            SensorManager_IRQHandler_Button1();
            break;
        case BUTTON2_Pin:
            SensorManager_IRQHandler_Button2();
            break;
        default:
            break;
    }
}

/* USER CODE END 1 */

/** Configure pins as
        * Analog
        * Input
        * Output
        * EVENT_OUT
        * EXTI
*/
void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDCONTROL3_GPIO_Port, LEDCONTROL3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDCONTROL4_GPIO_Port, LEDCONTROL4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDCONTROL2_GPIO_Port, LEDCONTROL2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LEDCONTROL1_GPIO_Port, LEDCONTROL1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LEDCONTROL3_Pin */
  GPIO_InitStruct.Pin = LEDCONTROL3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDCONTROL3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : R_RECEIVE_Pin (红外传感器) */
  GPIO_InitStruct.Pin = R_RECEIVE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 双边沿触发 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
  
  /*Configure GPIO pin : IFHIT_R_Pin (右侧光电门，碰撞高电平有效) */
  GPIO_InitStruct.Pin = IFHIT_R_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 双边沿触发 */
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;  /* 下拉，高电平表示碰撞 */
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : S_FOLLOW_CHECK_SIGNAL_Pin L_RECEIVE_Pin */
  GPIO_InitStruct.Pin = S_FOLLOW_CHECK_SIGNAL_Pin|L_RECEIVE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 改为双边沿触发 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : R_FOLLOW_CHECK_SIGNAL_Pin (红外传感器) */
  GPIO_InitStruct.Pin = R_FOLLOW_CHECK_SIGNAL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 双边沿触发 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /*Configure GPIO pin : IFHIT_L_Pin (左侧光电门，碰撞高电平有效) */
  GPIO_InitStruct.Pin = IFHIT_L_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 双边沿触发 */
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;  /* 下拉，高电平表示碰撞 */
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LEDCONTROL4_Pin */
  GPIO_InitStruct.Pin = LEDCONTROL4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDCONTROL4_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LEDCONTROL2_Pin */
  GPIO_InitStruct.Pin = LEDCONTROL2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDCONTROL2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : L_FOLLOW_CHECK_SIGNAL_Pin (红外传感器) */
  GPIO_InitStruct.Pin = L_FOLLOW_CHECK_SIGNAL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 双边沿触发 */
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  
  /*Configure GPIO pins : BUTTON1_Pin BUTTON2_Pin (按钮，按下低电平) */
  GPIO_InitStruct.Pin = BUTTON1_Pin|BUTTON2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;  /* 双边沿触发，检测按下和释放 */
  GPIO_InitStruct.Pull = GPIO_PULLUP;  /* 上拉，按下时为低电平 */
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : LEDCONTROL1_Pin */
  GPIO_InitStruct.Pin = LEDCONTROL1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LEDCONTROL1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SIGNAL_1_Pin SIGNAL_2_Pin */
  GPIO_InitStruct.Pin = SIGNAL_1_Pin|SIGNAL_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 2 */

/* USER CODE END 2 */
