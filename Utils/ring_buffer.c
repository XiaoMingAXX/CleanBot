/**
  ******************************************************************************
  * @file    ring_buffer.c
  * @brief   环形缓冲区实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "ring_buffer.h"

/**
  * @brief  初始化环形缓冲区
  * @param  rb: 环形缓冲区对象指针
  * @param  buffer: 缓冲区指针
  * @param  size: 缓冲区大小
  * @retval None
  */
void RingBuffer_Init(RingBuffer_t *rb, uint8_t *buffer, uint32_t size)
{
    if (rb == NULL || buffer == NULL || size == 0) return;
    
    rb->buffer = buffer;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

/**
  * @brief  复位环形缓冲区
  * @param  rb: 环形缓冲区对象指针
  * @retval None
  */
void RingBuffer_Reset(RingBuffer_t *rb)
{
    if (rb == NULL) return;
    
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
}

/**
  * @brief  写入一个字节
  * @param  rb: 环形缓冲区对象指针
  * @param  data: 数据
  * @retval 是否成功
  */
bool RingBuffer_Put(RingBuffer_t *rb, uint8_t data)
{
    if (rb == NULL || rb->buffer == NULL) return false;
    
    if (RingBuffer_IsFull(rb)) {
        return false;  /* 缓冲区满 */
    }
    
    rb->buffer[rb->head] = data;
    rb->head = (rb->head + 1) % rb->size;
    rb->count++;
    
    return true;
}

/**
  * @brief  读取一个字节
  * @param  rb: 环形缓冲区对象指针
  * @param  data: 数据指针
  * @retval 是否成功
  */
bool RingBuffer_Get(RingBuffer_t *rb, uint8_t *data)
{
    if (rb == NULL || rb->buffer == NULL || data == NULL) return false;
    
    if (RingBuffer_IsEmpty(rb)) {
        return false;  /* 缓冲区空 */
    }
    
    *data = rb->buffer[rb->tail];
    rb->tail = (rb->tail + 1) % rb->size;
    rb->count--;
    
    return true;
}

/**
  * @brief  获取当前数据数量
  * @param  rb: 环形缓冲区对象指针
  * @retval 数据数量
  */
uint32_t RingBuffer_GetCount(RingBuffer_t *rb)
{
    if (rb == NULL) return 0;
    return rb->count;
}

/**
  * @brief  获取空闲空间
  * @param  rb: 环形缓冲区对象指针
  * @retval 空闲空间大小
  */
uint32_t RingBuffer_GetFree(RingBuffer_t *rb)
{
    if (rb == NULL) return 0;
    return rb->size - rb->count;
}

/**
  * @brief  检查是否满
  * @param  rb: 环形缓冲区对象指针
  * @retval 是否满
  */
bool RingBuffer_IsFull(RingBuffer_t *rb)
{
    if (rb == NULL) return true;
    return (rb->count >= rb->size);
}

/**
  * @brief  检查是否空
  * @param  rb: 环形缓冲区对象指针
  * @retval 是否空
  */
bool RingBuffer_IsEmpty(RingBuffer_t *rb)
{
    if (rb == NULL) return true;
    return (rb->count == 0);
}

/**
  * @brief  写入多个字节
  * @param  rb: 环形缓冲区对象指针
  * @param  data: 数据指针
  * @param  len: 数据长度
  * @retval 实际写入的字节数
  */
uint32_t RingBuffer_PutData(RingBuffer_t *rb, const uint8_t *data, uint32_t len)
{
    if (rb == NULL || data == NULL || len == 0) return 0;
    
    uint32_t written = 0;
    for (uint32_t i = 0; i < len; i++) {
        if (RingBuffer_Put(rb, data[i])) {
            written++;
        } else {
            break;  /* 缓冲区满 */
        }
    }
    
    return written;
}

/**
  * @brief  读取多个字节
  * @param  rb: 环形缓冲区对象指针
  * @param  data: 数据指针
  * @param  len: 数据长度
  * @retval 实际读取的字节数
  */
uint32_t RingBuffer_GetData(RingBuffer_t *rb, uint8_t *data, uint32_t len)
{
    if (rb == NULL || data == NULL || len == 0) return 0;
    
    uint32_t read = 0;
    for (uint32_t i = 0; i < len; i++) {
        if (RingBuffer_Get(rb, &data[i])) {
            read++;
        } else {
            break;  /* 缓冲区空 */
        }
    }
    
    return read;
}

