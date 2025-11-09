/**
  ******************************************************************************
  * @file    test_example.c
  * @brief   测试示例代码
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  * @attention
  * 此文件包含测试示例代码，展示如何使用各个电机控制接口
  ******************************************************************************
  */

#include "CleanBotApp.h"
#include "motor_ctrl_task.h"
#include "cmsis_os.h"

/**
 * @brief  测试示例：轮电机控制
 */
void Test_Example_WheelMotor(void)
{
    /* 设置左右轮速度都为0.5 m/s */
    CleanBotApp_Test_SetWheelSpeedMs(0.5f, 0.5f);
    
    /* 或者直接使用MotorCtrlTask接口 */
    MotorCtrlTask_SetWheelSpeed(0.5f, 0.5f);
    
    /* 等待1秒 */
    osDelay(1000);
    
    /* 停止 */
    MotorCtrlTask_SetWheelSpeed(0.0f, 0.0f);
}

/**
 * @brief  测试示例：边刷电机控制
 */
void Test_Example_BrushMotor(void)
{
    /* 左边刷低速，右边刷高速 */
    CleanBotApp_Test_SetBrushMotor(1, 2);  /* 1=低速, 2=高速 */
    
    /* 或者直接使用MotorCtrlTask接口 */
    MotorCtrlTask_SetBrushMotor(BRUSH_MOTOR_LEVEL_LOW, BRUSH_MOTOR_LEVEL_HIGH);
    
    /* 等待2秒 */
    osDelay(2000);
    
    /* 关闭 */
    MotorCtrlTask_SetBrushMotor(BRUSH_MOTOR_LEVEL_OFF, BRUSH_MOTOR_LEVEL_OFF);
}

/**
 * @brief  测试示例：水泵电机控制
 */
void Test_Example_PumpMotor(void)
{
    /* 设置为中速 */
    CleanBotApp_Test_SetPumpMotor(2);  /* 0=关闭, 1=低速, 2=中速, 3=高速 */
    
    /* 或者直接使用MotorCtrlTask接口 */
    MotorCtrlTask_SetPumpMotor(PUMP_MOTOR_LEVEL_MEDIUM);
    
    /* 等待2秒 */
    osDelay(2000);
    
    /* 关闭 */
    MotorCtrlTask_SetPumpMotor(PUMP_MOTOR_LEVEL_OFF);
}

/**
 * @brief  测试示例：风机控制
 */
void Test_Example_FanMotor(void)
{
    /* 设置为档位3 */
    CleanBotApp_Test_SetFanMotor(3);  /* 0=关闭, 1-5=档位1-5 */
    
    /* 或者直接使用MotorCtrlTask接口 */
    MotorCtrlTask_SetFanMotor(FAN_MOTOR_LEVEL_3);
    
    /* 等待2秒 */
    osDelay(2000);
    
    /* 关闭 */
    MotorCtrlTask_SetFanMotor(FAN_MOTOR_LEVEL_OFF);
}

/**
 * @brief  测试示例：综合测试
 */
void Test_Example_Comprehensive(void)
{
    /* 1. 启动边刷电机（低速） */
    MotorCtrlTask_SetBrushMotor(BRUSH_MOTOR_LEVEL_LOW, BRUSH_MOTOR_LEVEL_LOW);
    osDelay(500);
    
    /* 2. 启动风机（档位2） */
    MotorCtrlTask_SetFanMotor(FAN_MOTOR_LEVEL_2);
    osDelay(500);
    
    /* 3. 启动轮电机（前进0.3 m/s） */
    MotorCtrlTask_SetWheelSpeed(0.3f, 0.3f);
    osDelay(2000);
    
    /* 4. 转向（左转） */
    MotorCtrlTask_SetWheelSpeed(0.2f, 0.4f);
    osDelay(1000);
    
    /* 5. 停止所有电机 */
    MotorCtrlTask_SetWheelSpeed(0.0f, 0.0f);
    MotorCtrlTask_SetBrushMotor(BRUSH_MOTOR_LEVEL_OFF, BRUSH_MOTOR_LEVEL_OFF);
    MotorCtrlTask_SetFanMotor(FAN_MOTOR_LEVEL_OFF);
    MotorCtrlTask_SetPumpMotor(PUMP_MOTOR_LEVEL_OFF);
}

/**
 * @brief  测试示例：获取轮电机速度
 */
void Test_Example_GetWheelSpeed(void)
{
    float leftSpeedMs, rightSpeedMs;
    
    /* 设置目标速度 */
    MotorCtrlTask_SetWheelSpeed(0.5f, 0.5f);
    
    /* 等待稳定 */
    osDelay(1000);
    
    /* 获取当前速度 */
    MotorCtrlTask_GetWheelSpeed(&leftSpeedMs, &rightSpeedMs);
    
    /* 可以在这里添加打印或上传到上位机的代码 */
    /* 例如：通过USB通信发送速度数据 */
}

