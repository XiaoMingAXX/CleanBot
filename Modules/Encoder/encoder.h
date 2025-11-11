/**
  ******************************************************************************
  * @file    encoder.h
  * @brief   霍尔编码器驱动头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __ENCODER_H__
#define __ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* 编码器类型枚举 */
typedef enum {
    ENCODER_TYPE_WHEEL_LEFT = 0,   /* 左轮编码器 */
    ENCODER_TYPE_WHEEL_RIGHT,      /* 右轮编码器 */
    ENCODER_TYPE_FAN               /* 风机编码器 */
} EncoderType_t;

/* 编码器结构体 */
typedef struct {
    EncoderType_t type;            /* 编码器类型 */
    TIM_HandleTypeDef *htim;       /* 定时器句柄（编码器模式） */
    int32_t pulseCount;            /* 脉冲计数 */
    int32_t lastPulseCount;        /* 上次脉冲计数 */
    int32_t lastPulseCountISR;     /* 上次脉冲计数（1kHz中断用） */
    int32_t overflowCount;         /* 溢出次数 */
    float speed;                   /* 当前速度 (RPM) */
    float speedMs;                 /* 当前速度 (m/s) - 仅用于轮电机 */
    uint32_t lastUpdateTime;       /* 上次更新时间 */
    uint16_t ppr;                  /* 每转脉冲数 (Pulse Per Revolution) */
    uint16_t gearRatio;            /* 减速比 */
    uint32_t pulsePerMeter;        /* 每米脉冲数 - 用于轮电机速度计算 */
    bool enabled;                  /* 使能标志 */
} Encoder_t;

/* 函数声明 */
void Encoder_Init(Encoder_t *encoder, EncoderType_t type, TIM_HandleTypeDef *htim, 
                  uint16_t ppr, uint16_t gearRatio);
void Encoder_SetPulsePerMeter(Encoder_t *encoder, uint32_t pulsePerMeter);  /* 设置每米脉冲数（轮电机） */
void Encoder_Start(Encoder_t *encoder);
void Encoder_Stop(Encoder_t *encoder);
void Encoder_Reset(Encoder_t *encoder);
int32_t Encoder_GetPulseCount(Encoder_t *encoder);
float Encoder_GetSpeed(Encoder_t *encoder);  /* 获取速度 (RPM) */
float Encoder_GetSpeedMs(Encoder_t *encoder);  /* 获取速度 (m/s) - 仅用于轮电机 */
void Encoder_Update(Encoder_t *encoder);     /* 更新速度计算 */
int32_t Encoder_GetDeltaCount(Encoder_t *encoder);  /* 获取增量脉冲数 */

/* 1kHz定时器中断钩子（由TIM7调用）*/
void Encoder_On1kHzTick(Encoder_t *encoder);

#ifdef __cplusplus
}
#endif

#endif /* __ENCODER_H__ */

