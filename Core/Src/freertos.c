/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "CleanBotApp.h"
#include "sensor_manager.h"
#include "sensor_task.h"
#include "motor_ctrl_task.h"
#include "usb_comm_task.h"
#include "imu_task.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* Definitions for SensorTask */
osThreadId_t sensorTaskHandle;
const osThreadAttr_t sensorTask_attributes = {
  .name = "sensorTask",
  .stack_size = TASK_STACK_SIZE_SENSOR * 4,
  .priority = (osPriority_t) TASK_PRIORITY_SENSOR,
};

/* Definitions for MotorCtrlTask */
osThreadId_t motorCtrlTaskHandle;
const osThreadAttr_t motorCtrlTask_attributes = {
  .name = "motorCtrlTask",
  .stack_size = TASK_STACK_SIZE_MOTOR_CTRL * 4,
  .priority = (osPriority_t) TASK_PRIORITY_MOTOR_CTRL,
};

/* Definitions for USBCommTask */
osThreadId_t usbCommTaskHandle;
const osThreadAttr_t usbCommTask_attributes = {
  .name = "usbCommTask",
  .stack_size = TASK_STACK_SIZE_USB_COMM * 4,
  .priority = (osPriority_t) TASK_PRIORITY_USB_COMM,
};

/* Definitions for IMUTask */
osThreadId_t imuTaskHandle;
const osThreadAttr_t imuTask_attributes = {
  .name = "imuTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
extern void SensorTask_Run(void *argument);
extern void MotorCtrlTask_Run(void *argument);
extern void USBCommTask_Run(void *argument);
extern void IMUTask_Run(void *argument);

extern void MX_USB_DEVICE_Init(void);
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

extern void MX_USB_DEVICE_Init(void);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* 初始化应用层 */
  CleanBotApp_t *app = CleanBotApp_GetInstance();
  CleanBotApp_Init(app);
  
  /* 初始化传感器管理器 */
  SensorManager_Init(SensorManager_GetInstance());
  
  /* 创建传感器任务 */
  sensorTaskHandle = osThreadNew(SensorTask_Run, NULL, &sensorTask_attributes);
  
  /* 创建电机控制任务 */
  motorCtrlTaskHandle = osThreadNew(MotorCtrlTask_Run, NULL, &motorCtrlTask_attributes);
  
  /* 创建USB通信任务 */
  usbCommTaskHandle = osThreadNew(USBCommTask_Run, NULL, &usbCommTask_attributes);

  /* 创建IMU任务（200Hz上报） */
  imuTaskHandle = osThreadNew(IMUTask_Run, NULL, &imuTask_attributes);
  
  /* 启动应用 */
  CleanBotApp_Start(app);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* init code for USB_DEVICE */
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN StartDefaultTask */
  /* 主任务主要用于初始化，实际功能在各专门任务中实现 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

