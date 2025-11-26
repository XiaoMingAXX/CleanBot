/**
  ******************************************************************************
  * @file    ir_homing.h
  * @brief   红外回冲定位模块头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  * @attention
  * 
  * 本模块实现扫地机器人的红外回冲定位功能，通过4个红外接收器
  * 检测充电座的信标信号，实现自动寻找并对接充电座。
  * 
  ******************************************************************************
  */

#ifndef __IR_HOMING_H__
#define __IR_HOMING_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "ir_sensor.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================
   充电座信标命令码定义（实际NEC编码）
   ============================================ */

/* 充电站发射的实际信号码 */
#define IR_CODE_LEFT          0x17    /* 最左边传感器接收的信号 */
#define IR_CODE_FRONT_LEFT    0x65    /* 左前传感器（signal1）接收的信号 */
#define IR_CODE_FRONT_RIGHT   0x9A    /* 右前传感器（signal2）接收的信号 */
#define IR_CODE_RIGHT         0xB4    /* 最右边传感器接收的信号 */
#define IR_CODE_EXTRA         0xA3    /* 其他可能出现的信号 */

/* 对准状态：4个传感器分别接收到对应信号 */
/* 左=0x17, 左前=0x65, 右前=0x9A, 右=0xB4 = 已对准 */

/* ============================================
   回冲状态定义
   ============================================ */

/* 回冲导航状态 */
typedef enum {
    HOMING_STATE_IDLE = 0,        /* 空闲，未启动回冲 */
    HOMING_STATE_SEARCHING,       /* 搜索充电座 */
    HOMING_STATE_APPROACHING,     /* 接近充电座 */
    HOMING_STATE_ALIGNING,        /* 对齐中 */
    HOMING_STATE_DOCKING,         /* 对接中 */
    HOMING_STATE_DOCKED,          /* 已对接 */
    HOMING_STATE_FAILED,          /* 回冲失败 */
    HOMING_STATE_TIMEOUT          /* 超时 */
} HomingState_t;

/* 红外接收器位置 */
typedef enum {
    IR_POSITION_LEFT = 0,         /* 左侧 */
    IR_POSITION_RIGHT,            /* 右侧 */
    IR_POSITION_FRONT_LEFT,       /* 左前 */
    IR_POSITION_FRONT_RIGHT       /* 右前 */
} IRPosition_t;

/* 单个红外接收器状态 */
typedef struct {
    bool detected;                /* 是否检测到信号 */
    uint8_t command;              /* 接收到的命令码 */
    uint8_t address;              /* 接收到的地址码 */
    uint32_t lastDetectTime;      /* 最后检测时间（ms） */
    uint16_t detectCount;         /* 连续检测次数 */
} IRReceiverStatus_t;

/* 回冲导航数据 */
typedef struct {
    /* 当前状态 */
    HomingState_t state;          /* 回冲状态 */
    
    /* 4个红外接收器状态 */
    IRReceiverStatus_t left;      /* 左侧接收器 */
    IRReceiverStatus_t right;     /* 右侧接收器 */
    IRReceiverStatus_t frontLeft; /* 左前接收器 */
    IRReceiverStatus_t frontRight;/* 右前接收器 */
    
    /* 导航参数 */
    float targetSpeedLeft;        /* 左轮目标速度（m/s） */
    float targetSpeedRight;       /* 右轮目标速度（m/s） */
    
    /* 统计信息 */
    uint32_t startTime;           /* 回冲开始时间 */
    uint32_t timeout;             /* 超时时间（ms） */
    uint16_t searchRotations;     /* 搜索旋转次数 */
    
    /* 配置参数 */
    bool enabled;                 /* 模块使能标志 */
    bool debug;                   /* 调试模式 */

    /* 传感器/碰撞状态 */
    bool bumperLeftTriggered;     /* 左碰撞触发 */
    bool bumperRightTriggered;    /* 右碰撞触发 */

    /* Docking阶段控制 */
    bool dockingTimerActive;
    uint32_t dockingTimerStart;

    /* Docked后退控制 */
    bool dockBackoffActive;
    uint32_t dockBackoffStart;
} IRHoming_t;

/* ============================================
   函数声明
   ============================================ */

/**
 * @brief  初始化红外回冲模块
 * @param  homing: 回冲对象指针
 * @retval None
 */
void IRHoming_Init(IRHoming_t *homing);

/**
 * @brief  启动红外回冲
 * @param  homing: 回冲对象指针
 * @param  timeout: 超时时间（ms），0表示不限时
 * @retval None
 */
void IRHoming_Start(IRHoming_t *homing, uint32_t timeout);

/**
 * @brief  停止红外回冲
 * @param  homing: 回冲对象指针
 * @retval None
 */
void IRHoming_Stop(IRHoming_t *homing);

/**
 * @brief  更新红外接收器数据
 * @param  homing: 回冲对象指针
 * @param  position: 接收器位置
 * @param  necData: NEC解码数据
 * @retval None
 */
void IRHoming_UpdateReceiver(IRHoming_t *homing, IRPosition_t position, NEC_Data_t *necData);

/**
 * @brief  红外回冲导航算法（需要在任务中周期性调用）
 * @param  homing: 回冲对象指针
 * @retval None
 */
void IRHoming_Process(IRHoming_t *homing);

/**
 * @brief  获取当前回冲状态
 * @param  homing: 回冲对象指针
 * @retval 回冲状态
 */
HomingState_t IRHoming_GetState(IRHoming_t *homing);

/**
 * @brief  检查是否成功对接
 * @param  homing: 回冲对象指针
 * @retval true=已对接，false=未对接
 */
bool IRHoming_IsDocked(IRHoming_t *homing);

/**
 * @brief  检查是否已完美对准（所有4个传感器接收到正确信号）
 * @param  homing: 回冲对象指针
 * @retval true=已对准，false=未对准
 */
bool IRHoming_IsAligned(IRHoming_t *homing);

/**
 * @brief  获取导航速度（供电机控制使用）
 * @param  homing: 回冲对象指针
 * @param  leftSpeed: 输出左轮速度（m/s）
 * @param  rightSpeed: 输出右轮速度（m/s）
 * @retval None
 */
void IRHoming_GetNavigationSpeed(IRHoming_t *homing, float *leftSpeed, float *rightSpeed);

/**
 * @brief  使能调试模式
 * @param  homing: 回冲对象指针
 * @param  enable: true=使能，false=禁用
 * @retval None
 */
void IRHoming_SetDebug(IRHoming_t *homing, bool enable);

/**
 * @brief  重置回冲模块
 * @param  homing: 回冲对象指针
 * @retval None
 */
void IRHoming_Reset(IRHoming_t *homing);

/**
 * @brief  更新碰撞传感器状态
 */
void IRHoming_UpdateBumperState(IRHoming_t *homing, bool leftTriggered, bool rightTriggered);

#ifdef __cplusplus
}
#endif

#endif /* __IR_HOMING_H__ */
