/**
  ******************************************************************************
  * @file    ir_homing.c
  * @brief   红外回冲定位模块实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "ir_homing.h"
#include "motor_ctrl_task.h"
#include <string.h>

/* 配置参数 */
#define IR_SIGNAL_TIMEOUT_MS     500    /* 信号超时时间（ms） */
#define IR_DETECT_THRESHOLD      3      /* 连续检测阈值 */
#define DEFAULT_TIMEOUT_MS       120000 /* 默认超时时间（2分钟） */

/* 速度配置（m/s） */
#define SPEED_SEARCH            0.15f   /* 搜索速度 */
#define SPEED_APPROACH          0.20f   /* 接近速度 */
#define SPEED_ALIGN_FAST        0.25f   /* 快速对齐 */
#define SPEED_ALIGN_SLOW        0.15f   /* 慢速对齐 */
#define SPEED_DOCK              0.10f   /* 对接速度 */
#define SPEED_ROTATE            0.12f   /* 旋转速度 */

/**
 * @brief  初始化红外回冲模块
 */
void IRHoming_Init(IRHoming_t *homing)
{
    if (homing == NULL) return;
    
    memset(homing, 0, sizeof(IRHoming_t));
    homing->state = HOMING_STATE_IDLE;
    homing->enabled = false;
    homing->debug = false;
    homing->timeout = DEFAULT_TIMEOUT_MS;
}

/**
 * @brief  启动红外回冲
 */
void IRHoming_Start(IRHoming_t *homing, uint32_t timeout)
{
    if (homing == NULL) return;
    
    /* 重置状态 */
    IRHoming_Reset(homing);
    
    /* 启动回冲 */
    homing->enabled = true;
    homing->state = HOMING_STATE_SEARCHING;
    homing->startTime = HAL_GetTick();
    homing->timeout = (timeout > 0) ? timeout : DEFAULT_TIMEOUT_MS;
    homing->searchRotations = 0;
}

/**
 * @brief  停止红外回冲
 */
void IRHoming_Stop(IRHoming_t *homing)
{
    if (homing == NULL) return;
    
    homing->enabled = false;
    homing->state = HOMING_STATE_IDLE;
    
    /* 停止电机 */
    MotorCtrlTask_SetWheelSpeed(0.0f, 0.0f);
}

/**
 * @brief  更新红外接收器数据
 */
void IRHoming_UpdateReceiver(IRHoming_t *homing, IRPosition_t position, NEC_Data_t *necData)
{
    if (homing == NULL || necData == NULL || !necData->valid) return;
    
    IRReceiverStatus_t *receiver = NULL;
    
    /* 选择对应的接收器 */
    switch (position) {
        case IR_POSITION_LEFT:
            receiver = &homing->left;
            break;
        case IR_POSITION_RIGHT:
            receiver = &homing->right;
            break;
        case IR_POSITION_FRONT_LEFT:
            receiver = &homing->frontLeft;
            break;
        case IR_POSITION_FRONT_RIGHT:
            receiver = &homing->frontRight;
            break;
        default:
            return;
    }
    
    /* 更新接收器状态 */
    receiver->detected = true;
    receiver->command = necData->command;
    receiver->address = necData->address;
    receiver->lastDetectTime = HAL_GetTick();
    receiver->detectCount++;
}

/**
 * @brief  检查信号是否超时
 */
static void IRHoming_CheckSignalTimeout(IRHoming_t *homing)
{
    uint32_t currentTime = HAL_GetTick();
    
    /* 检查各接收器信号是否超时 */
    if ((currentTime - homing->left.lastDetectTime) > IR_SIGNAL_TIMEOUT_MS) {
        homing->left.detected = false;
        homing->left.detectCount = 0;
    }
    if ((currentTime - homing->right.lastDetectTime) > IR_SIGNAL_TIMEOUT_MS) {
        homing->right.detected = false;
        homing->right.detectCount = 0;
    }
    if ((currentTime - homing->frontLeft.lastDetectTime) > IR_SIGNAL_TIMEOUT_MS) {
        homing->frontLeft.detected = false;
        homing->frontLeft.detectCount = 0;
    }
    if ((currentTime - homing->frontRight.lastDetectTime) > IR_SIGNAL_TIMEOUT_MS) {
        homing->frontRight.detected = false;
        homing->frontRight.detectCount = 0;
    }
}

/**
 * @brief  检查是否超时
 */
static bool IRHoming_CheckTimeout(IRHoming_t *homing)
{
    if (homing->timeout == 0) return false;  /* 不限时 */
    
    uint32_t elapsed = HAL_GetTick() - homing->startTime;
    if (elapsed > homing->timeout) {
        homing->state = HOMING_STATE_TIMEOUT;
        return true;
    }
    return false;
}

/**
 * @brief  搜索充电座
 */
static void IRHoming_Search(IRHoming_t *homing)
{
    /* 检查是否检测到任何信号 */
    if (homing->left.detected || homing->right.detected ||
        homing->frontLeft.detected || homing->frontRight.detected) {
        /* 检测到信号，切换到接近模式 */
        homing->state = HOMING_STATE_APPROACHING;
        return;
    }
    
    /* 原地旋转搜索充电座 */
    homing->targetSpeedLeft = SPEED_ROTATE;
    homing->targetSpeedRight = -SPEED_ROTATE;
    
    /* 记录旋转次数（每360度计数） */
    static uint32_t lastRotateTime = 0;
    if (HAL_GetTick() - lastRotateTime > 3000) {  /* 约3秒一圈 */
        homing->searchRotations++;
        lastRotateTime = HAL_GetTick();
    }
}

/**
 * @brief  检查是否已完美对准（所有传感器接收到正确信号）
 * @note   此函数提供给内部和外部调用
 */
bool IRHoming_IsAligned(IRHoming_t *homing)
{
    if (homing == NULL) return false;
    
    return (homing->left.detected && homing->left.command == IR_CODE_LEFT &&
            homing->frontLeft.detected && homing->frontLeft.command == IR_CODE_FRONT_LEFT &&
            homing->frontRight.detected && homing->frontRight.command == IR_CODE_FRONT_RIGHT &&
            homing->right.detected && homing->right.command == IR_CODE_RIGHT);
}

/**
 * @brief  接近充电座
 */
static void IRHoming_Approach(IRHoming_t *homing)
{
    /* 情况1：完美对准 - 所有传感器接收到正确信号 */
    if (IRHoming_IsAligned(homing)) {
        /* 已对准，切换到对接模式 */
        homing->state = HOMING_STATE_ALIGNING;
        homing->targetSpeedLeft = SPEED_APPROACH;
        homing->targetSpeedRight = SPEED_APPROACH;
        return;
    }
    
    /* 情况2：左前和右前都检测到正确信号 - 基本对准 */
    if (homing->frontLeft.detected && homing->frontLeft.command == IR_CODE_FRONT_LEFT &&
        homing->frontRight.detected && homing->frontRight.command == IR_CODE_FRONT_RIGHT) {
        /* 前方已对准，继续前进 */
        homing->targetSpeedLeft = SPEED_APPROACH;
        homing->targetSpeedRight = SPEED_APPROACH;
        return;
    }
    
    /* 情况3：只有左前检测到正确信号 - 需要左转 */
    if (homing->frontLeft.detected && homing->frontLeft.command == IR_CODE_FRONT_LEFT &&
        !homing->frontRight.detected) {
        /* 左转修正 */
        homing->targetSpeedLeft = SPEED_ALIGN_SLOW;
        homing->targetSpeedRight = SPEED_ALIGN_FAST;
        return;
    }
    
    /* 情况4：只有右前检测到正确信号 - 需要右转 */
    if (homing->frontRight.detected && homing->frontRight.command == IR_CODE_FRONT_RIGHT &&
        !homing->frontLeft.detected) {
        /* 右转修正 */
        homing->targetSpeedLeft = SPEED_ALIGN_FAST;
        homing->targetSpeedRight = SPEED_ALIGN_SLOW;
        return;
    }
    
    /* 情况5：只有左侧检测到信号 - 充电座在左侧，原地左转 */
    if (homing->left.detected && (homing->left.command == IR_CODE_LEFT || 
                                   homing->left.command == IR_CODE_EXTRA) &&
        !homing->frontLeft.detected && !homing->frontRight.detected) {
        /* 原地左转 */
        homing->targetSpeedLeft = -SPEED_ROTATE;
        homing->targetSpeedRight = SPEED_ROTATE;
        return;
    }
    
    /* 情况6：只有右侧检测到信号 - 充电座在右侧，原地右转 */
    if (homing->right.detected && (homing->right.command == IR_CODE_RIGHT || 
                                    homing->right.command == IR_CODE_EXTRA) &&
        !homing->frontLeft.detected && !homing->frontRight.detected) {
        /* 原地右转 */
        homing->targetSpeedLeft = SPEED_ROTATE;
        homing->targetSpeedRight = -SPEED_ROTATE;
        return;
    }
    
    /* 情况7：检测到任意信号，但方向不明确 - 慢速前进同时微调 */
    if (homing->left.detected || homing->right.detected ||
        homing->frontLeft.detected || homing->frontRight.detected) {
        /* 慢速前进 */
        homing->targetSpeedLeft = SPEED_SEARCH;
        homing->targetSpeedRight = SPEED_SEARCH;
        return;
    }
    
    /* 情况8：失去所有信号 - 返回搜索模式 */
    homing->state = HOMING_STATE_SEARCHING;
}

/**
 * @brief  对齐充电座（精确对准阶段）
 */
static void IRHoming_Align(IRHoming_t *homing)
{
    if (homing->bumperLeftTriggered && homing->bumperRightTriggered) {
        homing->state = HOMING_STATE_DOCKED;
        homing->dockingTimerActive = false;
        homing->dockBackoffActive = false;
        return;
    }

    /* 检查是否完美对准（4个传感器都接收到正确信号） */
    if (IRHoming_IsAligned(homing)) {
        /* 完美对准，准备对接 */
        homing->state = HOMING_STATE_DOCKING;
        homing->targetSpeedLeft = SPEED_DOCK;
        homing->targetSpeedRight = SPEED_DOCK;
        return;
    }
    
    /* 精确对齐：左前和右前都检测到正确信号 */
    if (homing->frontLeft.detected && homing->frontLeft.command == IR_CODE_FRONT_LEFT &&
        homing->frontRight.detected && homing->frontRight.command == IR_CODE_FRONT_RIGHT) {
        /* 慢速直行，等待左右传感器也对准 */
        homing->targetSpeedLeft = SPEED_DOCK;
        homing->targetSpeedRight = SPEED_DOCK;
        return;
    }
    
    /* 需要微调方向：只有左前检测到 */
    if (homing->frontLeft.detected && homing->frontLeft.command == IR_CODE_FRONT_LEFT &&
        !homing->frontRight.detected) {
        /* 微调左转 */
        homing->targetSpeedLeft = SPEED_DOCK * 0.7f;
        homing->targetSpeedRight = SPEED_DOCK * 1.3f;
        return;
    }
    
    /* 需要微调方向：只有右前检测到 */
    if (homing->frontRight.detected && homing->frontRight.command == IR_CODE_FRONT_RIGHT &&
        !homing->frontLeft.detected) {
        /* 微调右转 */
        homing->targetSpeedLeft = SPEED_DOCK * 1.3f;
        homing->targetSpeedRight = SPEED_DOCK * 0.7f;
        return;
    }
    
    /* 失去精确信号，返回接近模式 */
    homing->state = HOMING_STATE_APPROACHING;
}

/**
 * @brief  对接充电座
 */
static void IRHoming_Dock(IRHoming_t *homing)
{
    if (homing->bumperLeftTriggered && homing->bumperRightTriggered) {
        homing->state = HOMING_STATE_DOCKED;
        homing->dockingTimerActive = false;
        homing->dockBackoffActive = false;
        return;
    }

    /* 极低速前进对接 */
    homing->targetSpeedLeft = SPEED_DOCK * 0.5f;
    homing->targetSpeedRight = SPEED_DOCK * 0.5f;
    
    /* TODO: 这里可以添加充电检测逻辑 */
    /* 如果检测到充电电流，表示对接成功 */
    /* 示例：if (charging_detected) { */
    /*     homing->state = HOMING_STATE_DOCKED; */
    /* } */
    
    /* 暂时：持续对接10秒后认为成功（兜底） */
    if (!homing->dockingTimerActive) {
        homing->dockingTimerActive = true;
        homing->dockingTimerStart = HAL_GetTick();
    }
    if ((HAL_GetTick() - homing->dockingTimerStart) > 10000) {
        homing->state = HOMING_STATE_DOCKED;
        homing->dockingTimerActive = false;
        homing->dockBackoffActive = false;
    }
}

/**
 * @brief  红外回冲导航算法
 */
void IRHoming_Process(IRHoming_t *homing)
{
    if (homing == NULL || !homing->enabled) return;
    
    /* 检查超时 */
    if (IRHoming_CheckTimeout(homing)) {
        IRHoming_Stop(homing);
        return;
    }
    
    /* 检查信号超时 */
    IRHoming_CheckSignalTimeout(homing);
    
    /* 根据当前状态执行相应的导航算法 */
    switch (homing->state) {
        case HOMING_STATE_SEARCHING:
            IRHoming_Search(homing);
            break;
            
        case HOMING_STATE_APPROACHING:
            IRHoming_Approach(homing);
            break;
            
        case HOMING_STATE_ALIGNING:
            IRHoming_Align(homing);
            break;
            
        case HOMING_STATE_DOCKING:
            IRHoming_Dock(homing);
            break;
            
        case HOMING_STATE_DOCKED:
            if (!homing->dockBackoffActive) {
                homing->dockBackoffActive = true;
                homing->dockBackoffStart = HAL_GetTick();
            }
            if ((HAL_GetTick() - homing->dockBackoffStart) < 1000) {
                homing->targetSpeedLeft = -SPEED_DOCK;
                homing->targetSpeedRight = -SPEED_DOCK;
            } else {
                homing->targetSpeedLeft = 0.0f;
                homing->targetSpeedRight = 0.0f;
                homing->dockBackoffActive = false;
                homing->state = HOMING_STATE_IDLE;
                homing->enabled = false;
            }
            break;
            
        case HOMING_STATE_FAILED:
        case HOMING_STATE_TIMEOUT:
            /* 失败或超时，停止 */
            IRHoming_Stop(homing);
            break;
            
        default:
            break;
    }
    
    /* 应用速度到电机控制 */
    MotorCtrlTask_SetWheelSpeed(homing->targetSpeedLeft, homing->targetSpeedRight);
}

/**
 * @brief  获取当前回冲状态
 */
HomingState_t IRHoming_GetState(IRHoming_t *homing)
{
    if (homing == NULL) return HOMING_STATE_IDLE;
    return homing->state;
}

/**
 * @brief  检查是否成功对接
 */
bool IRHoming_IsDocked(IRHoming_t *homing)
{
    if (homing == NULL) return false;
    return (homing->state == HOMING_STATE_DOCKED);
}

/**
 * @brief  获取导航速度
 */
void IRHoming_GetNavigationSpeed(IRHoming_t *homing, float *leftSpeed, float *rightSpeed)
{
    if (homing == NULL || leftSpeed == NULL || rightSpeed == NULL) return;
    
    *leftSpeed = homing->targetSpeedLeft;
    *rightSpeed = homing->targetSpeedRight;
}

/**
 * @brief  使能调试模式
 */
void IRHoming_SetDebug(IRHoming_t *homing, bool enable)
{
    if (homing == NULL) return;
    homing->debug = enable;
}

/**
 * @brief  重置回冲模块
 */
void IRHoming_Reset(IRHoming_t *homing)
{
    if (homing == NULL) return;
    
    /* 清除接收器状态 */
    memset(&homing->left, 0, sizeof(IRReceiverStatus_t));
    memset(&homing->right, 0, sizeof(IRReceiverStatus_t));
    memset(&homing->frontLeft, 0, sizeof(IRReceiverStatus_t));
    memset(&homing->frontRight, 0, sizeof(IRReceiverStatus_t));
    
    /* 重置导航参数 */
    homing->targetSpeedLeft = 0.0f;
    homing->targetSpeedRight = 0.0f;
    homing->searchRotations = 0;
    homing->bumperLeftTriggered = false;
    homing->bumperRightTriggered = false;
    homing->dockingTimerActive = false;
    homing->dockingTimerStart = 0;
    homing->dockBackoffActive = false;
    homing->dockBackoffStart = 0;
}

void IRHoming_UpdateBumperState(IRHoming_t *homing, bool leftTriggered, bool rightTriggered)
{
    if (homing == NULL) {
        return;
    }
    homing->bumperLeftTriggered = leftTriggered;
    homing->bumperRightTriggered = rightTriggered;
}

