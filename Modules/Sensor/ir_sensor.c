/**
  ******************************************************************************
  * @file    ir_sensor.c
  * @brief   红外传感器驱动实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "ir_sensor.h"

/**
  * @brief  初始化红外传感器
  * @param  sensor: 传感器对象指针
  * @param  type: 传感器类型
  * @param  port: GPIO端口
  * @param  pin: GPIO引脚
  * @retval None
  */
void IR_Sensor_Init(IR_Sensor_t *sensor, IR_SensorType_t type, 
                    GPIO_TypeDef *port, uint16_t pin)
{
    if (sensor == NULL) return;
    
    sensor->type = type;
    sensor->port = port;
    sensor->pin = pin;
    sensor->lastEdgeTime = 0;
    sensor->lastLevel = false;
    sensor->enabled = false;
    
    NEC_Decoder_Init(&sensor->decoder);
}

/**
  * @brief  使能红外传感器
  * @param  sensor: 传感器对象指针
  * @retval None
  */
void IR_Sensor_Enable(IR_Sensor_t *sensor)
{
    if (sensor == NULL) return;
    sensor->enabled = true;
    NEC_Decoder_Init(&sensor->decoder);
}

/**
  * @brief  禁用红外传感器
  * @param  sensor: 传感器对象指针
  * @retval None
  */
void IR_Sensor_Disable(IR_Sensor_t *sensor)
{
    if (sensor == NULL) return;
    sensor->enabled = false;
}

/**
  * @brief  更新传感器状态（需要在中断或定时器中调用）
  * @param  sensor: 传感器对象指针
  * @retval None
  */
void IR_Sensor_Update(IR_Sensor_t *sensor)
{
    if (sensor == NULL || !sensor->enabled) return;
    
    bool currentLevel = HAL_GPIO_ReadPin(sensor->port, sensor->pin);
    
    /* 检测边沿 */
    if (currentLevel != sensor->lastLevel) {
        /* 计算时间差（微秒） */
        uint32_t currentTime = HAL_GetTick() * 1000;  /* 转换为微秒（近似） */
        uint32_t period = currentTime - sensor->lastEdgeTime;
        
        /* 处理边沿 */
        NEC_Decoder_ProcessEdge(&sensor->decoder, period, currentLevel);
        
        sensor->lastEdgeTime = currentTime;
        sensor->lastLevel = currentLevel;
    }
}

/**
  * @brief  获取当前电平
  * @param  sensor: 传感器对象指针
  * @retval 当前电平（true=高电平，false=低电平）
  */
bool IR_Sensor_GetLevel(IR_Sensor_t *sensor)
{
    if (sensor == NULL || sensor->port == NULL) return false;
    return HAL_GPIO_ReadPin(sensor->port, sensor->pin) == GPIO_PIN_SET;
}

/**
  * @brief  检查NEC数据是否就绪
  * @param  sensor: 传感器对象指针
  * @retval 数据是否就绪
  */
bool IR_Sensor_IsDataReady(IR_Sensor_t *sensor)
{
    if (sensor == NULL) return false;
    return NEC_Decoder_IsDataReady(&sensor->decoder);
}

/**
  * @brief  获取NEC解码数据
  * @param  sensor: 传感器对象指针
  * @retval NEC解码数据
  */
NEC_Data_t IR_Sensor_GetNECData(IR_Sensor_t *sensor)
{
    NEC_Data_t result = {0};
    if (sensor != NULL) {
        result = NEC_Decoder_GetData(&sensor->decoder);
    }
    return result;
}

