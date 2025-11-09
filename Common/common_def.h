/**
  ******************************************************************************
  * @file    common_def.h
  * @brief   公共定义头文件
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  * @attention
  * 此文件包含项目中使用的公共定义、类型定义和宏定义
  ******************************************************************************
  */

#ifndef __COMMON_DEF_H__
#define __COMMON_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ============================================
   类型定义
   ============================================ */

/* 状态枚举 */
typedef enum {
    STATUS_OK = 0,
    STATUS_ERROR,
    STATUS_BUSY,
    STATUS_TIMEOUT,
    STATUS_INVALID_PARAM,
    STATUS_NOT_INITIALIZED
} Status_t;

/* 方向枚举 */
typedef enum {
    DIRECTION_FORWARD = 0,
    DIRECTION_BACKWARD,
    DIRECTION_LEFT,
    DIRECTION_RIGHT,
    DIRECTION_STOP
} Direction_t;

/* ============================================
   常用宏定义
   ============================================ */

/* 数组大小宏 */
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))

/* 最小值/最大值宏 */
#define MIN(a, b)               ((a) < (b) ? (a) : (b))
#define MAX(a, b)               ((a) > (b) ? (a) : (b))
#define CLAMP(x, min, max)      ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

/* 位操作宏 */
#define BIT(n)                  (1UL << (n))
#define SET_BIT(reg, bit)       ((reg) |= (bit))
#define CLEAR_BIT(reg, bit)     ((reg) &= ~(bit))
#define READ_BIT(reg, bit)      ((reg) & (bit))
#define TOGGLE_BIT(reg, bit)    ((reg) ^= (bit))

/* 断言宏 */
#ifdef DEBUG
    #define ASSERT(expr)        ((expr) ? (void)0 : Error_Handler())
#else
    #define ASSERT(expr)        ((void)0)
#endif

/* 空指针检查 */
#define CHECK_PTR(ptr)          do { if ((ptr) == NULL) return STATUS_INVALID_PARAM; } while(0)

/* ============================================
   时间相关宏
   ============================================ */

/* 毫秒转系统时钟周期 */
#define MS_TO_TICKS(ms)          ((ms) * configTICK_RATE_HZ / 1000)
#define TICKS_TO_MS(ticks)      ((ticks) * 1000 / configTICK_RATE_HZ)

/* 微秒转系统时钟周期 (假设系统时钟为168MHz) */
#define US_TO_TICKS(us)          ((us) * 168)
#define TICKS_TO_US(ticks)      ((ticks) / 168)

/* ============================================
   数学相关宏
   ============================================ */

/* 角度转换 */
#define DEG_TO_RAD(deg)         ((deg) * 3.14159265359f / 180.0f)
#define RAD_TO_DEG(rad)         ((rad) * 180.0f / 3.14159265359f)

/* 圆周率 */
#ifndef PI
    #define PI                  3.14159265359f
#endif

/* ============================================
   字符串操作宏
   ============================================ */

/* 字符串长度 */
#define STR_LEN(str)            (sizeof(str) - 1)

/* ============================================
   模块版本信息
   ============================================ */

#define VERSION_MAJOR           1
#define VERSION_MINOR           0
#define VERSION_PATCH           0

#define VERSION_STRING          "1.0.0"

/* ============================================
   编译器相关
   ============================================ */

/* 内联函数 */
#ifndef INLINE
    #ifdef __GNUC__
        #define INLINE          __attribute__((always_inline)) inline
    #elif defined(__ICCARM__)
        #define INLINE          __inline
    #else
        #define INLINE          inline
    #endif
#endif

/* 弱函数 */
#ifndef WEAK
    #ifdef __GNUC__
        #define WEAK           __attribute__((weak))
    #elif defined(__ICCARM__)
        #define WEAK           __weak
    #else
        #define WEAK
    #endif
#endif

/* 打包结构体 */
#ifndef PACKED
    #ifdef __GNUC__
        #define PACKED         __attribute__((packed))
    #elif defined(__ICCARM__)
        #define PACKED         __packed
    #else
        #define PACKED
    #endif
#endif

/* 对齐 */
#ifndef ALIGNED
    #ifdef __GNUC__
        #define ALIGNED(n)     __attribute__((aligned(n)))
    #elif defined(__ICCARM__)
        #define ALIGNED(n)     __align(n)
    #else
        #define ALIGNED(n)
    #endif
#endif

/* ============================================
   错误处理函数声明
   ============================================ */

void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_DEF_H__ */

