/**
  ******************************************************************************
  * @file    nec_decode.c
  * @brief   NEC红外解码实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "nec_decode.h"
#include <stddef.h>  /* 定义NULL */
#include <stdint.h>
#include <stdbool.h>

/**
  * @brief  初始化NEC解码器
  * @param  decoder: 解码器对象指针
  * @retval None
  */
void NEC_Decoder_Init(NEC_Decoder_t *decoder)
{
    if (decoder == NULL) return;
    
    decoder->state = NEC_STATE_IDLE;
    decoder->data = 0;
    decoder->bitCount = 0;
    decoder->lastEdgeTime = 0;
    decoder->leaderStartTime = 0;
    decoder->result.valid = false;
    decoder->enabled = true;
}

/**
  * @brief  复位NEC解码器
  * @param  decoder: 解码器对象指针
  * @retval None
  */
void NEC_Decoder_Reset(NEC_Decoder_t *decoder)
{
    if (decoder == NULL) return;
    
    decoder->state = NEC_STATE_IDLE;
    decoder->data = 0;
    decoder->bitCount = 0;
    decoder->result.valid = false;
}

/**
  * @brief  处理边沿信号
  * @param  decoder: 解码器对象指针
  * @param  time: 当前时间（微秒）
  * @param  level: 电平（true=高电平，false=低电平）
  * @retval 是否解码完成
  */
bool NEC_Decoder_ProcessEdge(NEC_Decoder_t *decoder, uint32_t time, bool level)
{
    if (decoder == NULL || !decoder->enabled) return false;
    
    uint32_t period = time - decoder->lastEdgeTime;
    decoder->lastEdgeTime = time;
    
    switch (decoder->state) {
        case NEC_STATE_IDLE:
            /* 等待引导码 */
            if (level) {
                /* 检测到高电平，可能是引导码开始 */
                decoder->leaderStartTime = time;
                decoder->state = NEC_STATE_LEADER;
            }
            break;
            
        case NEC_STATE_LEADER:
            if (!level) {
                /* 引导码低电平 */
                if (period >= (NEC_LEADER_HIGH_TIME - NEC_TIME_TOLERANCE) &&
                    period <= (NEC_LEADER_HIGH_TIME + NEC_TIME_TOLERANCE)) {
                    /* 引导码高电平正确，等待低电平 */
                    decoder->state = NEC_STATE_LEADER;
                } else {
                    /* 不符合引导码，重新开始 */
                    decoder->state = NEC_STATE_IDLE;
                }
            } else {
                /* 引导码低电平后的高电平 */
                if (period >= (NEC_LEADER_LOW_TIME - NEC_TIME_TOLERANCE) &&
                    period <= (NEC_LEADER_LOW_TIME + NEC_TIME_TOLERANCE)) {
                    /* 引导码正确，开始接收数据 */
                    decoder->state = NEC_STATE_DATA;
                    decoder->data = 0;
                    decoder->bitCount = 0;
                } else {
                    /* 不符合引导码，重新开始 */
                    decoder->state = NEC_STATE_IDLE;
                }
            }
            break;
            
        case NEC_STATE_DATA:
            if (!level) {
                /* 数据位低电平，计算数据位 */
                uint8_t bit;
                if (period >= (NEC_BIT_0_TIME - NEC_TIME_TOLERANCE) &&
                    period <= (NEC_BIT_0_TIME + NEC_TIME_TOLERANCE)) {
                    bit = 0;
                } else if (period >= (NEC_BIT_1_TIME - NEC_TIME_TOLERANCE) &&
                          period <= (NEC_BIT_1_TIME + NEC_TIME_TOLERANCE)) {
                    bit = 1;
                } else {
                    /* 时序错误 */
                    decoder->state = NEC_STATE_ERROR;
                    return false;
                }
                
                /* 存储数据位（NEC协议：LSB first） */
                decoder->data |= (bit << decoder->bitCount);
                decoder->bitCount++;
                
                /* 检查是否接收完32位数据 */
                if (decoder->bitCount >= 32) {
                    /* 解析数据 */
                    decoder->result.address = (decoder->data >> 0) & 0xFF;
                    decoder->result.command = (decoder->data >> 8) & 0xFF;
                    decoder->result.addressInv = (decoder->data >> 16) & 0xFF;
                    decoder->result.commandInv = (decoder->data >> 24) & 0xFF;
                    
                    /* 验证数据（地址和命令的反码校验） */
                    if ((decoder->result.address ^ decoder->result.addressInv) == 0xFF &&
                        (decoder->result.command ^ decoder->result.commandInv) == 0xFF) {
                        decoder->result.valid = true;
                        decoder->state = NEC_STATE_COMPLETE;
                        return true;
                    } else {
                        decoder->state = NEC_STATE_ERROR;
                        return false;
                    }
                }
            }
            break;
            
        case NEC_STATE_COMPLETE:
        case NEC_STATE_ERROR:
            /* 等待下一个信号 */
            decoder->state = NEC_STATE_IDLE;
            decoder->result.valid = false;
            break;
            
        default:
            decoder->state = NEC_STATE_IDLE;
            break;
    }
    
    return false;
}

/**
  * @brief  获取解码数据
  * @param  decoder: 解码器对象指针
  * @retval 解码结果
  */
NEC_Data_t NEC_Decoder_GetData(NEC_Decoder_t *decoder)
{
    NEC_Data_t result = {0};
    if (decoder != NULL) {
        result = decoder->result;
        decoder->result.valid = false;  /* 清除标志，准备接收下一次数据 */
        decoder->state = NEC_STATE_IDLE;
    }
    return result;
}

/**
  * @brief  检查数据是否就绪
  * @param  decoder: 解码器对象指针
  * @retval 数据是否就绪
  */
bool NEC_Decoder_IsDataReady(NEC_Decoder_t *decoder)
{
    if (decoder == NULL) return false;
    return decoder->result.valid;
}

