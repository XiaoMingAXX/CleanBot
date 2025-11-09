/**
  ******************************************************************************
  * @file    nec_decode.h
  * @brief   NEC红外解码头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __NEC_DECODE_H__
#define __NEC_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* NEC解码状态 */
typedef enum {
    NEC_STATE_IDLE = 0,         /* 空闲 */
    NEC_STATE_LEADER,           /* 引导码 */
    NEC_STATE_DATA,             /* 数据 */
    NEC_STATE_COMPLETE,         /* 完成 */
    NEC_STATE_ERROR             /* 错误 */
} NEC_State_t;

/* NEC解码结果 */
typedef struct {
    uint8_t address;            /* 地址码 */
    uint8_t command;            /* 命令码 */
    uint8_t addressInv;         /* 地址反码 */
    uint8_t commandInv;         /* 命令反码 */
    bool valid;                 /* 数据有效 */
} NEC_Data_t;

/* NEC解码器结构体 */
typedef struct {
    NEC_State_t state;          /* 当前状态 */
    uint32_t data;              /* 接收到的数据 */
    uint8_t bitCount;           /* 位计数 */
    uint32_t lastEdgeTime;      /* 上次边沿时间 */
    uint32_t leaderStartTime;   /* 引导码开始时间 */
    NEC_Data_t result;          /* 解码结果 */
    bool enabled;               /* 使能标志 */
} NEC_Decoder_t;

/* NEC时序定义（微秒） */
#define NEC_LEADER_HIGH_TIME       9000    /* 引导码高电平时间 */
#define NEC_LEADER_LOW_TIME        4500    /* 引导码低电平时间 */
#define NEC_BIT_0_TIME             1125    /* 数据0时间 */
#define NEC_BIT_1_TIME             2250    /* 数据1时间 */
#define NEC_TIME_TOLERANCE         500     /* 时间容差 */

/* 函数声明 */
void NEC_Decoder_Init(NEC_Decoder_t *decoder);
void NEC_Decoder_Reset(NEC_Decoder_t *decoder);
bool NEC_Decoder_ProcessEdge(NEC_Decoder_t *decoder, uint32_t time, bool level);
NEC_Data_t NEC_Decoder_GetData(NEC_Decoder_t *decoder);
bool NEC_Decoder_IsDataReady(NEC_Decoder_t *decoder);

#ifdef __cplusplus
}
#endif

#endif /* __NEC_DECODE_H__ */

