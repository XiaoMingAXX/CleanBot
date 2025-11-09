/**
  ******************************************************************************
  * @file    ring_buffer.h
  * @brief   环形缓冲区头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __RING_BUFFER_H__
#define __RING_BUFFER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* 环形缓冲区结构体 */
typedef struct {
    uint8_t *buffer;        /* 缓冲区指针 */
    uint32_t size;          /* 缓冲区大小 */
    uint32_t head;          /* 写指针 */
    uint32_t tail;          /* 读指针 */
    uint32_t count;         /* 当前数据数量 */
} RingBuffer_t;

/* 函数声明 */
void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buffer, uint32_t size);
void RingBuffer_Reset(RingBuffer_t *rb);
bool RingBuffer_Put(RingBuffer_t *rb, uint8_t data);
bool RingBuffer_Get(RingBuffer_t *rb, uint8_t *data);
uint32_t RingBuffer_GetCount(RingBuffer_t *rb);
uint32_t RingBuffer_GetFree(RingBuffer_t *rb);
bool RingBuffer_IsFull(RingBuffer_t *rb);
bool RingBuffer_IsEmpty(RingBuffer_t *rb);
uint32_t RingBuffer_PutData(RingBuffer_t *rb, const uint8_t *data, uint32_t len);
uint32_t RingBuffer_GetData(RingBuffer_t *rb, uint8_t *data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* __RING_BUFFER_H__ */

