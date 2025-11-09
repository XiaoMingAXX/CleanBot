/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LEDCONTROL3_Pin GPIO_PIN_2
#define LEDCONTROL3_GPIO_Port GPIOE
#define R_RECEIVE_Pin GPIO_PIN_3
#define R_RECEIVE_GPIO_Port GPIOE
#define R_RECEIVE_EXTI_IRQn EXTI3_IRQn
#define IFHIT_R_Pin GPIO_PIN_4
#define IFHIT_R_GPIO_Port GPIOE
#define IFHIT_R_EXTI_IRQn EXTI4_IRQn
#define S_FOLLOW_CHECK_SIGNAL_Pin GPIO_PIN_5
#define S_FOLLOW_CHECK_SIGNAL_GPIO_Port GPIOF
#define S_FOLLOW_CHECK_SIGNAL_EXTI_IRQn EXTI9_5_IRQn
#define CONTROLBUZZER_Pin GPIO_PIN_6
#define CONTROLBUZZER_GPIO_Port GPIOF
#define FANFG_Pin GPIO_PIN_0
#define FANFG_GPIO_Port GPIOA
#define R_FOLLOW_CHECK_SIGNAL_Pin GPIO_PIN_2
#define R_FOLLOW_CHECK_SIGNAL_GPIO_Port GPIOA
#define R_FOLLOW_CHECK_SIGNAL_EXTI_IRQn EXTI2_IRQn
#define MOTOR2__Pin GPIO_PIN_5
#define MOTOR2__GPIO_Port GPIOA
#define L_RECEIVE_Pin GPIO_PIN_12
#define L_RECEIVE_GPIO_Port GPIOF
#define L_RECEIVE_EXTI_IRQn EXTI15_10_IRQn
#define LEDCONTROL4_Pin GPIO_PIN_14
#define LEDCONTROL4_GPIO_Port GPIOF
#define LEDCONTROL2_Pin GPIO_PIN_1
#define LEDCONTROL2_GPIO_Port GPIOG
#define MOTOR1__Pin GPIO_PIN_9
#define MOTOR1__GPIO_Port GPIOE
#define MOTOR1_E11_Pin GPIO_PIN_11
#define MOTOR1_E11_GPIO_Port GPIOE
#define L_FOLLOW_CHECK_SIGNAL_Pin GPIO_PIN_9
#define L_FOLLOW_CHECK_SIGNAL_GPIO_Port GPIOD
#define L_FOLLOW_CHECK_SIGNAL_EXTI_IRQn EXTI9_5_IRQn
#define MOTOR2DRC_Pin GPIO_PIN_14
#define MOTOR2DRC_GPIO_Port GPIOD
#define MOTOR2PWM_Pin GPIO_PIN_15
#define MOTOR2PWM_GPIO_Port GPIOD
#define PUMP_PWM_Pin GPIO_PIN_6
#define PUMP_PWM_GPIO_Port GPIOC
#define FAN_PWM_Pin GPIO_PIN_7
#define FAN_PWM_GPIO_Port GPIOC
#define CleanMotor1_PWM_Pin GPIO_PIN_8
#define CleanMotor1_PWM_GPIO_Port GPIOC
#define CleanMotor2_PWM_Pin GPIO_PIN_9
#define CleanMotor2_PWM_GPIO_Port GPIOC
#define IFHIT_L_Pin GPIO_PIN_15
#define IFHIT_L_GPIO_Port GPIOA
#define IFHIT_L_EXTI_IRQn EXTI15_10_IRQn
#define LEDCONTROL1_Pin GPIO_PIN_12
#define LEDCONTROL1_GPIO_Port GPIOC
#define BUTTON1_Pin GPIO_PIN_6
#define BUTTON1_GPIO_Port GPIOD
#define BUTTON1_EXTI_IRQn EXTI9_5_IRQn
#define BUTTON2_Pin GPIO_PIN_7
#define BUTTON2_GPIO_Port GPIOD
#define BUTTON2_EXTI_IRQn EXTI9_5_IRQn
#define SIGNAL_1_Pin GPIO_PIN_10
#define SIGNAL_1_GPIO_Port GPIOG
#define SIGNAL_1_EXTI_IRQn EXTI15_10_IRQn
#define SIGNAL_2_Pin GPIO_PIN_11
#define SIGNAL_2_GPIO_Port GPIOG
#define SIGNAL_2_EXTI_IRQn EXTI15_10_IRQn
#define MOTOR2_B3_Pin GPIO_PIN_3
#define MOTOR2_B3_GPIO_Port GPIOB
#define MOTOR1DRC_Pin GPIO_PIN_6
#define MOTOR1DRC_GPIO_Port GPIOB
#define MOTOR1PWM_Pin GPIO_PIN_7
#define MOTOR1PWM_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
