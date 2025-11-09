/**
  ******************************************************************************
  * @file    common_utils.h
  * @brief   公共工具函数头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#ifndef __COMMON_UTILS_H__
#define __COMMON_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "common_def.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================
   时间相关函数
   ============================================ */

/**
 * @brief  获取系统运行时间 (毫秒)
 * @return 系统运行时间 (ms)
 */
uint32_t Common_GetTick(void);

/**
 * @brief  获取系统运行时间 (微秒)
 * @return 系统运行时间 (us)
 */
uint64_t Common_GetTickUs(void);

/**
 * @brief  延时函数 (毫秒)
 * @param  ms: 延时时间 (ms)
 */
void Common_Delay(uint32_t ms);

/**
 * @brief  延时函数 (微秒)
 * @param  us: 延时时间 (us)
 */
void Common_DelayUs(uint32_t us);

/**
 * @brief  检查是否超时
 * @param  startTime: 开始时间 (ms)
 * @param  timeout: 超时时间 (ms)
 * @return true: 超时, false: 未超时
 */
bool Common_IsTimeout(uint32_t startTime, uint32_t timeout);

/* ============================================
   数学相关函数
   ============================================ */

/**
 * @brief  限制值在指定范围内
 * @param  value: 输入值
 * @param  min: 最小值
 * @param  max: 最大值
 * @return 限制后的值
 */
int32_t Common_Clamp(int32_t value, int32_t min, int32_t max);

/**
 * @brief  限制浮点值在指定范围内
 * @param  value: 输入值
 * @param  min: 最小值
 * @param  max: 最大值
 * @return 限制后的值
 */
float Common_ClampFloat(float value, float min, float max);

/**
 * @brief  计算绝对值
 * @param  value: 输入值
 * @return 绝对值
 */
int32_t Common_Abs(int32_t value);

/**
 * @brief  计算浮点绝对值
 * @param  value: 输入值
 * @return 绝对值
 */
float Common_AbsFloat(float value);

/**
 * @brief  线性映射
 * @param  value: 输入值
 * @param  inMin: 输入最小值
 * @param  inMax: 输入最大值
 * @param  outMin: 输出最小值
 * @param  outMax: 输出最大值
 * @return 映射后的值
 */
float Common_Map(float value, float inMin, float inMax, float outMin, float outMax);

/* ============================================
   字符串相关函数
   ============================================ */

/**
 * @brief  字符串长度
 * @param  str: 字符串指针
 * @return 字符串长度
 */
uint32_t Common_StrLen(const char *str);

/**
 * @brief  字符串比较
 * @param  str1: 字符串1
 * @param  str2: 字符串2
 * @return 0: 相等, >0: str1>str2, <0: str1<str2
 */
int32_t Common_StrCmp(const char *str1, const char *str2);

/**
 * @brief  字符串复制
 * @param  dest: 目标缓冲区
 * @param  src: 源字符串
 * @param  maxLen: 最大长度
 * @return 复制的字符数
 */
uint32_t Common_StrCpy(char *dest, const char *src, uint32_t maxLen);

/* ============================================
   数据转换函数
   ============================================ */

/**
 * @brief  整数转字符串
 * @param  value: 整数值
 * @param  str: 输出缓冲区
 * @param  base: 进制 (10, 16等)
 * @return 字符串长度
 */
uint32_t Common_IntToStr(int32_t value, char *str, uint32_t base);

/**
 * @brief  浮点数转字符串
 * @param  value: 浮点值
 * @param  str: 输出缓冲区
 * @param  precision: 小数位数
 * @return 字符串长度
 */
uint32_t Common_FloatToStr(float value, char *str, uint32_t precision);

/**
 * @brief  字符串转整数
 * @param  str: 字符串
 * @return 整数值
 */
int32_t Common_StrToInt(const char *str);

/**
 * @brief  字符串转浮点数
 * @param  str: 字符串
 * @return 浮点值
 */
float Common_StrToFloat(const char *str);

/* ============================================
   校验和函数
   ============================================ */

/**
 * @brief  计算校验和 (简单累加)
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return 校验和
 */
uint8_t Common_Checksum8(const uint8_t *data, uint32_t len);

/**
 * @brief  计算CRC16
 * @param  data: 数据指针
 * @param  len: 数据长度
 * @return CRC16值
 */
uint16_t Common_CRC16(const uint8_t *data, uint32_t len);

/* ============================================
   字节序转换
   ============================================ */

/**
 * @brief  字节序转换 (16位)
 * @param  value: 输入值
 * @return 转换后的值
 */
uint16_t Common_Swap16(uint16_t value);

/**
 * @brief  字节序转换 (32位)
 * @param  value: 输入值
 * @return 转换后的值
 */
uint32_t Common_Swap32(uint32_t value);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_UTILS_H__ */

