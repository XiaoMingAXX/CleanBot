/**
  ******************************************************************************
  * @file    photo_gate.c
  * @brief   光电门驱动实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "photo_gate.h"
#include "cmsis_os.h"

/**
  * @brief  初始化光电门
  * @param  gate: 光电门对象指针
  * @param  type: 光电门类型
  * @param  port: GPIO端口
  * @param  pin: GPIO引脚
  * @retval None
  */
void PhotoGate_Init(PhotoGate_t *gate, PhotoGateType_t type, 
                    GPIO_TypeDef *port, uint16_t pin)
{
    if (gate == NULL) return;
    
    gate->type = type;
    gate->port = port;
    gate->pin = pin;
    gate->state = PHOTO_GATE_CLEAR;
    gate->lastState = PHOTO_GATE_CLEAR;
    gate->triggerTime = 0;
    gate->enabled = false;
}

/**
  * @brief  使能光电门
  * @param  gate: 光电门对象指针
  * @retval None
  */
void PhotoGate_Enable(PhotoGate_t *gate)
{
    if (gate == NULL) return;
    gate->enabled = true;
}

/**
  * @brief  禁用光电门
  * @param  gate: 光电门对象指针
  * @retval None
  */
void PhotoGate_Disable(PhotoGate_t *gate)
{
    if (gate == NULL) return;
    gate->enabled = false;
}

/**
  * @brief  更新光电门状态
  * @param  gate: 光电门对象指针
  * @retval None
  */
void PhotoGate_Update(PhotoGate_t *gate)
{
    if (gate == NULL || !gate->enabled || gate->port == NULL) return;
    
    gate->lastState = gate->state;
    
    /* 读取GPIO状态（根据实际硬件连接，可能需要取反） */
    GPIO_PinState pinState = HAL_GPIO_ReadPin(gate->port, gate->pin);
    
    /* 假设低电平表示被遮挡，高电平表示畅通（可根据实际硬件调整） */
    if (pinState == GPIO_PIN_RESET) {
        gate->state = PHOTO_GATE_BLOCKED;
    } else {
        gate->state = PHOTO_GATE_CLEAR;
    }
    
    /* 检测状态变化 */
    if (gate->state != gate->lastState) {
        gate->triggerTime = osKernelGetTickCount();
    }
}

/**
  * @brief  获取光电门状态
  * @param  gate: 光电门对象指针
  * @retval 光电门状态
  */
PhotoGateState_t PhotoGate_GetState(PhotoGate_t *gate)
{
    if (gate == NULL) return PHOTO_GATE_CLEAR;
    return gate->state;
}

/**
  * @brief  检查是否被遮挡
  * @param  gate: 光电门对象指针
  * @retval 是否被遮挡
  */
bool PhotoGate_IsBlocked(PhotoGate_t *gate)
{
    if (gate == NULL) return false;
    return (gate->state == PHOTO_GATE_BLOCKED);
}

/**
  * @brief  检查是否有触发（状态变化）
  * @param  gate: 光电门对象指针
  * @retval 是否有触发
  */
bool PhotoGate_IsTriggered(PhotoGate_t *gate)
{
    if (gate == NULL) return false;
    return (gate->state != gate->lastState);
}

