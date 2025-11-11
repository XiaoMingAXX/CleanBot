/**
  ******************************************************************************
  * @file    sensor_manager.h
  * @brief   传感器管理模块头文件（中断回调+任务处理）
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __SENSOR_MANAGER_H__
#define __SENSOR_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cleanbot_config.h"
#include <stdint.h>
#include <stdbool.h>

/* 传感器事件类型 */
typedef enum {
    SENSOR_EVENT_IR_LEFT = 0,          /* 左侧红外传感器事件 */
    SENSOR_EVENT_IR_RIGHT,             /* 右侧红外传感器事件 */
    SENSOR_EVENT_IR_FRONT_LEFT,        /* 左前红外传感器事件 */
    SENSOR_EVENT_IR_FRONT_RIGHT,       /* 右前红外传感器事件 */
    SENSOR_EVENT_PHOTO_GATE_LEFT,      /* 左侧光电门事件 */
    SENSOR_EVENT_PHOTO_GATE_RIGHT,     /* 右侧光电门事件 */
    SENSOR_EVENT_BUTTON1_PRESS,        /* 按钮1按下事件 */
    SENSOR_EVENT_BUTTON1_RELEASE,      /* 按钮1释放事件 */
    SENSOR_EVENT_BUTTON2_PRESS,        /* 按钮2按下事件 */
    SENSOR_EVENT_BUTTON2_RELEASE,      /* 按钮2释放事件 */
    SENSOR_EVENT_BUTTON1_CLICK,        /* 按钮1单击事件 */
    SENSOR_EVENT_BUTTON1_DOUBLE_CLICK, /* 按钮1双击事件 */
    SENSOR_EVENT_BUTTON2_CLICK,        /* 按钮2单击事件 */
    SENSOR_EVENT_BUTTON2_DOUBLE_CLICK  /* 按钮2双击事件 */
} SensorEventType_t;

/* 传感器事件 */
typedef struct {
    SensorEventType_t type;    /* 事件类型 */
    uint32_t timestamp;        /* 时间戳 */
    uint32_t data;             /* 事件数据（如NEC命令码） */
} SensorEvent_t;

/* 传感器管理器结构体 */
typedef struct {
    /* 事件队列 */
    QueueHandle_t eventQueue;
    
    /* 按钮状态 */
    struct {
        uint32_t pressTime;        /* 按下时间 */
        uint32_t releaseTime;      /* 释放时间 */
        bool isPressed;            /* 是否按下 */
        bool lastState;            /* 上次状态 */
        uint32_t clickCount;       /* 单击计数 */
        uint32_t lastClickTime;    /* 上次单击时间 */
        bool debouncePending;      /* 滤波待检测标志 */
        uint32_t debounceTime;     /* 滤波触发时间 */
        bool debounceState;        /* 滤波待检测状态 */
    } button1, button2;
    
    /* 传感器状态 */
    bool photoGateLeftBlocked;     /* 左侧光电门被遮挡 */
    bool photoGateRightBlocked;    /* 右侧光电门被遮挡 */
    
    /* NEC解码数据 */
    struct {
        bool dataReady;            /* 数据就绪 */
        NEC_Data_t necData;        /* NEC数据 */
    } irSensors[4];
    
    bool enabled;                  /* 使能标志 */
} SensorManager_t;

/* 函数声明 */
void SensorManager_Init(SensorManager_t *manager);
void SensorManager_Start(SensorManager_t *manager);
void SensorManager_Stop(SensorManager_t *manager);

/* 中断回调函数（在stm32f4xx_it.c中调用） */
void SensorManager_IRQHandler_IR_Left(void);
void SensorManager_IRQHandler_IR_Right(void);
void SensorManager_IRQHandler_IR_FrontLeft(void);
void SensorManager_IRQHandler_IR_FrontRight(void);
void SensorManager_IRQHandler_PhotoGate_Left(void);
void SensorManager_IRQHandler_PhotoGate_Right(void);
void SensorManager_IRQHandler_Button1(void);
void SensorManager_IRQHandler_Button2(void);

/* 获取事件（在Sensor任务中调用） */
bool SensorManager_GetEvent(SensorManager_t *manager, SensorEvent_t *event, uint32_t timeout);

/* 按钮滤波检测（在Sensor任务中定期调用） */
void SensorManager_CheckButtonDebounce(SensorManager_t *manager);

/* 获取全局传感器管理器实例 */
SensorManager_t* SensorManager_GetInstance(void);

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_MANAGER_H__ */

