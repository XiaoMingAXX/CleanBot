/**
  ******************************************************************************
  * @file    buzzer.h
  * @brief   蜂鸣器驱动头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __BUZZER_H__
#define __BUZZER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* 蜂鸣器结构体 */
typedef struct {
    GPIO_TypeDef *port;      /* GPIO端口 */
    uint16_t pin;            /* GPIO引脚 */
    TIM_HandleTypeDef *htim; /* 定时器句柄（用于产生PWM音调） */
    uint32_t channel;        /* PWM通道 */
    bool enabled;            /* 使能标志 */
    bool isPlaying;          /* 是否正在播放 */
} Buzzer_t;

/* 函数声明 */
void Buzzer_Init(Buzzer_t *buzzer, GPIO_TypeDef *port, uint16_t pin, 
                 TIM_HandleTypeDef *htim, uint32_t channel);
void Buzzer_Beep(Buzzer_t *buzzer, uint16_t frequency, uint16_t duration);  /* 蜂鸣（频率和持续时间） */
void Buzzer_On(Buzzer_t *buzzer);  /* 打开蜂鸣器 */
void Buzzer_Off(Buzzer_t *buzzer);  /* 关闭蜂鸣器 */
void Buzzer_Toggle(Buzzer_t *buzzer);  /* 切换蜂鸣器状态 */
bool Buzzer_IsPlaying(Buzzer_t *buzzer);  /* 检查是否正在播放 */

#ifdef __cplusplus
}
#endif

#endif /* __BUZZER_H__ */

