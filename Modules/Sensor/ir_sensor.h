/**
  ******************************************************************************
  * @file    ir_sensor.h
  * @brief   红外传感器驱动头文件（支持NEC解码）
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __IR_SENSOR_H__
#define __IR_SENSOR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "nec_decode.h"
#include <stdint.h>
#include <stdbool.h>

/* 红外传感器类型 */
typedef enum {
    IR_SENSOR_LEFT = 0,      /* 左侧红外传感器 */
    IR_SENSOR_RIGHT,         /* 右侧红外传感器 */
    IR_SENSOR_FRONT_LEFT,    /* 左前红外传感器 */
    IR_SENSOR_FRONT_RIGHT    /* 右前红外传感器 */
} IR_SensorType_t;

/* 红外传感器结构体 */
typedef struct {
    IR_SensorType_t type;            /* 传感器类型 */
    GPIO_TypeDef *port;              /* GPIO端口 */
    uint16_t pin;                    /* GPIO引脚 */
    NEC_Decoder_t decoder;           /* NEC解码器 */
    uint32_t lastEdgeTime;           /* 上次边沿时间（微秒） */
    bool lastLevel;                  /* 上次电平 */
    bool enabled;                    /* 使能标志 */
} IR_Sensor_t;

/* 函数声明 */
void IR_Sensor_Init(IR_Sensor_t *sensor, IR_SensorType_t type, 
                    GPIO_TypeDef *port, uint16_t pin);
void IR_Sensor_Enable(IR_Sensor_t *sensor);
void IR_Sensor_Disable(IR_Sensor_t *sensor);
void IR_Sensor_Update(IR_Sensor_t *sensor);  /* 需要在中断或定时器中调用 */
bool IR_Sensor_GetLevel(IR_Sensor_t *sensor);  /* 获取当前电平 */
bool IR_Sensor_IsDataReady(IR_Sensor_t *sensor);  /* 检查NEC数据是否就绪 */
NEC_Data_t IR_Sensor_GetNECData(IR_Sensor_t *sensor);  /* 获取NEC解码数据 */

#ifdef __cplusplus
}
#endif

#endif /* __IR_SENSOR_H__ */

