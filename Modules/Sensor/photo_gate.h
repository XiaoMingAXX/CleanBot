/**
  ******************************************************************************
  * @file    photo_gate.h
  * @brief   光电门驱动头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __PHOTO_GATE_H__
#define __PHOTO_GATE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* 光电门类型 */
typedef enum {
    PHOTO_GATE_LEFT = 0,     /* 左侧光电门 */
    PHOTO_GATE_RIGHT         /* 右侧光电门 */
} PhotoGateType_t;

/* 光电门状态 */
typedef enum {
    PHOTO_GATE_BLOCKED = 0,  /* 被遮挡 */
    PHOTO_GATE_CLEAR         /* 畅通 */
} PhotoGateState_t;

/* 光电门结构体 */
typedef struct {
    PhotoGateType_t type;        /* 光电门类型 */
    GPIO_TypeDef *port;          /* GPIO端口 */
    uint16_t pin;                /* GPIO引脚 */
    PhotoGateState_t state;      /* 当前状态 */
    PhotoGateState_t lastState;  /* 上次状态 */
    uint32_t triggerTime;        /* 触发时间 */
    bool enabled;                /* 使能标志 */
} PhotoGate_t;

/* 函数声明 */
void PhotoGate_Init(PhotoGate_t *gate, PhotoGateType_t type, 
                    GPIO_TypeDef *port, uint16_t pin);
void PhotoGate_Enable(PhotoGate_t *gate);
void PhotoGate_Disable(PhotoGate_t *gate);
void PhotoGate_Update(PhotoGate_t *gate);  /* 更新状态 */
PhotoGateState_t PhotoGate_GetState(PhotoGate_t *gate);
bool PhotoGate_IsBlocked(PhotoGate_t *gate);
bool PhotoGate_IsTriggered(PhotoGate_t *gate);  /* 检测是否有触发（状态变化） */

#ifdef __cplusplus
}
#endif

#endif /* __PHOTO_GATE_H__ */

