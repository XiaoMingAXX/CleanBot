/**
  ******************************************************************************
  * @file    encoder.c
  * @brief   霍尔编码器驱动实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "encoder.h"
#include "cmsis_os.h"

#define ENCODER_UPDATE_PERIOD_MS    10  /* 速度更新周期 (ms) */

/**
  * @brief  初始化编码器
  * @param  encoder: 编码器对象指针
  * @param  type: 编码器类型
  * @param  htim: 定时器句柄（编码器模式）
  * @param  ppr: 每转脉冲数
  * @param  gearRatio: 减速比
  * @retval None
  */
void Encoder_Init(Encoder_t *encoder, EncoderType_t type, TIM_HandleTypeDef *htim, 
                  uint16_t ppr, uint16_t gearRatio)
{
    if (encoder == NULL) return;
    
    encoder->type = type;
    encoder->htim = htim;
    encoder->pulseCount = 0;
    encoder->lastPulseCount = 0;
    encoder->lastPulseCountISR = 0;
    encoder->overflowCount = 0;
    encoder->speed = 0.0f;
    encoder->speedMs = 0.0f;
    encoder->angle = 0.0f;
    encoder->lastUpdateTime = 0;
    encoder->ppr = ppr;
    encoder->gearRatio = gearRatio;
    encoder->pulsePerMeter = 0;  /* 默认值，需要后续设置 */
    encoder->enabled = false;
}

/**
 * @brief  设置每米脉冲数（用于轮电机速度计算）
 */
void Encoder_SetPulsePerMeter(Encoder_t *encoder, uint32_t pulsePerMeter)
{
    if (encoder == NULL) return;
    encoder->pulsePerMeter = pulsePerMeter;
}

/**
  * @brief  启动编码器
  * @param  encoder: 编码器对象指针
  * @retval None
  */
void Encoder_Start(Encoder_t *encoder)
{
    if (encoder == NULL || encoder->htim == NULL) return;
    
    encoder->enabled = true;
    encoder->lastUpdateTime = osKernelGetTickCount();
    HAL_TIM_Encoder_Start(encoder->htim, TIM_CHANNEL_ALL);
}

/**
  * @brief  停止编码器
  * @param  encoder: 编码器对象指针
  * @retval None
  */
void Encoder_Stop(Encoder_t *encoder)
{
    if (encoder == NULL || encoder->htim == NULL) return;
    
    encoder->enabled = false;
    HAL_TIM_Encoder_Stop(encoder->htim, TIM_CHANNEL_ALL);
}

/**
  * @brief  复位编码器
  * @param  encoder: 编码器对象指针
  * @retval None
  */
void Encoder_Reset(Encoder_t *encoder)
{
    if (encoder == NULL) return;
    
    encoder->pulseCount = 0;
    encoder->lastPulseCount = 0;
    encoder->lastPulseCountISR = 0;
    encoder->overflowCount = 0;
    encoder->speed = 0.0f;
    encoder->angle = 0.0f;
    
    if (encoder->htim != NULL) {
        __HAL_TIM_SET_COUNTER(encoder->htim, 0);
    }
}

/**
  * @brief  获取脉冲计数
  * @param  encoder: 编码器对象指针
  * @retval 脉冲计数
  */
int32_t Encoder_GetPulseCount(Encoder_t *encoder)
{
    if (encoder == NULL || encoder->htim == NULL) return 0;
    
    int16_t counter = (int16_t)__HAL_TIM_GET_COUNTER(encoder->htim);
    
    /* 检测溢出 */
    static int16_t lastCounter[3] = {0, 0, 0};
    int32_t index = encoder->type;
    
    if (counter - lastCounter[index] > 32768) {
        encoder->overflowCount--;
			 
    } else if (counter - lastCounter[index] < -32768) {
        encoder->overflowCount++;
			 
    }
		 lastCounter[index] = counter;
    
    encoder->pulseCount = (int32_t)counter + encoder->overflowCount * 65536;
    return encoder->pulseCount;
}

/**
  * @brief  获取增量脉冲数
  * @param  encoder: 编码器对象指针
  * @retval 增量脉冲数
  */
int32_t Encoder_GetDeltaCount(Encoder_t *encoder)
{
    if (encoder == NULL) return 0;
    
    int32_t currentCount = Encoder_GetPulseCount(encoder);
    int32_t delta = currentCount - encoder->lastPulseCount;
    encoder->lastPulseCount = currentCount;
    
    return delta;
}

/**
  * @brief  更新速度计算
  * @param  encoder: 编码器对象指针
  * @retval None
  */
void Encoder_Update(Encoder_t *encoder)
{
    if (encoder == NULL || !encoder->enabled) return;
    
    uint32_t currentTime = osKernelGetTickCount();
    uint32_t deltaTime = currentTime - encoder->lastUpdateTime;
    
    if (deltaTime >= ENCODER_UPDATE_PERIOD_MS) {
        int32_t deltaCount = Encoder_GetDeltaCount(encoder);
        
        /* 计算速度 (RPM) */
        /* 速度 = (增量脉冲数 / 每转脉冲数 / 减速比) / (时间(ms) / 60000) */
        if (deltaTime > 0 && encoder->ppr > 0 && encoder->gearRatio > 0) {
            float revolutions = (float)deltaCount / (encoder->ppr * encoder->gearRatio);
            float minutes = (float)deltaTime / 60000.0f;
            if (minutes > 0) {
                encoder->speed = revolutions / minutes;
            } else {
                encoder->speed = 0.0f;
            }
        } else {
            encoder->speed = 0.0f;
        }
        
        /* 计算速度 (m/s) - 仅用于轮电机 */
        if (encoder->type == ENCODER_TYPE_WHEEL_LEFT || encoder->type == ENCODER_TYPE_WHEEL_RIGHT) {
            if (deltaTime > 0 && encoder->pulsePerMeter > 0) {
                float deltaTimeSeconds = (float)deltaTime / 1000.0f;
                float deltaMeters = (float)deltaCount / encoder->pulsePerMeter;
                encoder->speedMs = deltaMeters / deltaTimeSeconds;
            } else {
                encoder->speedMs = 0.0f;
            }
        }
        
        /* 更新角度（弧度制）：角度 = (累计脉冲数 / 每转脉冲数) * 2π */
        if (encoder->ppr > 0) {
            int32_t totalPulses = Encoder_GetPulseCount(encoder);
            encoder->angle = ((float)totalPulses / (float)encoder->ppr) * 6.28318530718f;  /* 2π ≈ 6.28318530718 */
        }
        
        encoder->lastUpdateTime = currentTime;
    }
}

/**
 * @brief  获取速度
 * @param  encoder: 编码器对象指针
 * @retval 速度 (RPM)
 */
float Encoder_GetSpeed(Encoder_t *encoder)
{
    if (encoder == NULL) return 0.0f;
    return encoder->speed;
}

/**
 * @brief  获取速度 (m/s) - 仅用于轮电机
 * @param  encoder: 编码器对象指针
 * @retval 速度 (m/s)
 */
float Encoder_GetSpeedMs(Encoder_t *encoder)
{
    if (encoder == NULL) return 0.0f;
    return encoder->speedMs;
}

/**
 * @brief  获取角度
 * @param  encoder: 编码器对象指针
 * @retval 角度 (弧度制，连续累加，可以是任意实数)
 */
float Encoder_GetAngle(Encoder_t *encoder)
{
    if (encoder == NULL) return 0.0f;
    
    /* 实时计算角度，确保获取最新值 */
    if (encoder->ppr > 0) {
        int32_t totalPulses = Encoder_GetPulseCount(encoder);
        encoder->angle = ((float)totalPulses / (float)encoder->ppr) * 6.28318530718f;  /* 2π ≈ 6.28318530718 */
    }
    
    return encoder->angle;
}
static int32_t watch_count_now;
/**
  * @brief  1kHz中断内更新速度（由TIM7调用）
  * @note   基于1ms时间基，减少与PID频率的耦合，提高速度计算精度
  */
void Encoder_On1kHzTick(Encoder_t *encoder)
{
    if (encoder == NULL || !encoder->enabled || encoder->htim == NULL) return;

    /* 读取当前累计脉冲（含溢出拓展） */
    int32_t currentCount = Encoder_GetPulseCount(encoder);
		watch_count_now = currentCount;
    int32_t delta = currentCount - encoder->lastPulseCountISR;
    encoder->lastPulseCountISR = currentCount;

    /* 1ms周期下的RPM计算：rpm = delta * 60000 / (ppr * gear) */
    if (encoder->ppr > 0 && encoder->gearRatio > 0) {
        float denom = (float)(encoder->ppr * encoder->gearRatio);
        float instRPM = (float)delta * 60000.0f / denom;
        /* 简单一阶IIR滤波，减小抖动（alpha越小越平滑） */
        const float alphaRPM = 0.8f;
        encoder->speed = alphaRPM * instRPM + (1.0f - alphaRPM) * encoder->speed;
    } else {
        encoder->speed = 0.0f;
    }

    /* 轮速 m/s：v = delta * 1000 / pulsePerMeter */
    if ((encoder->type == ENCODER_TYPE_WHEEL_LEFT || encoder->type == ENCODER_TYPE_WHEEL_RIGHT)
        && encoder->pulsePerMeter > 0) {
        float instMs = ((float)delta) * 1000.0f / (float)encoder->pulsePerMeter;
        const float alphaMs = 0.2f;
        encoder->speedMs = alphaMs * instMs + (1.0f - alphaMs) * encoder->speedMs;
    } else if (encoder->type == ENCODER_TYPE_FAN) {
        /* 非轮式不更新m/s */
    } else {
        encoder->speedMs = 0.0f;
    }
    
    /* 更新角度（弧度制）：角度 = (累计脉冲数 / 每转脉冲数) * 2π */
    if (encoder->ppr > 0) {
        encoder->angle = ((float)currentCount / (float)encoder->ppr) * 6.28318530718f;  /* 2π ≈ 6.28318530718 */
    }
}

