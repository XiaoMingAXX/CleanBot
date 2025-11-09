/**
  ******************************************************************************
  * @file    buzzer.c
  * @brief   蜂鸣器驱动实现
  * @author  CleanBot Team
  * @date    2025-01-XX
  ******************************************************************************
  */

#include "buzzer.h"
#include "cmsis_os.h"

/**
  * @brief  初始化蜂鸣器
  * @param  buzzer: 蜂鸣器对象指针
  * @param  port: GPIO端口
  * @param  pin: GPIO引脚
  * @param  htim: 定时器句柄（用于PWM）
  * @param  channel: PWM通道
  * @retval None
  */
void Buzzer_Init(Buzzer_t *buzzer, GPIO_TypeDef *port, uint16_t pin, 
                 TIM_HandleTypeDef *htim, uint32_t channel)
{
    if (buzzer == NULL) return;
    
    buzzer->port = port;
    buzzer->pin = pin;
    buzzer->htim = htim;
    buzzer->channel = channel;
    buzzer->enabled = true;
    buzzer->isPlaying = false;
    
    /* 初始状态：关闭 */
    if (buzzer->port != NULL) {
        HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_RESET);
    }
    
    /* 如果使用PWM，启动定时器 */
    if (buzzer->htim != NULL) {
        HAL_TIM_PWM_Start(buzzer->htim, buzzer->channel);
        __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, 0);  /* 初始占空比为0 */
    }
}

/**
  * @brief  蜂鸣（使用PWM产生音调）
  * @param  buzzer: 蜂鸣器对象指针
  * @param  frequency: 频率 (Hz)
  * @param  duration: 持续时间 (ms)
  * @retval None
  */
void Buzzer_Beep(Buzzer_t *buzzer, uint16_t frequency, uint16_t duration)
{
    if (buzzer == NULL || !buzzer->enabled) return;
    
    if (buzzer->htim != NULL && frequency > 0) {
        /* 计算PWM频率 */
        /* 假设系统时钟为168MHz，APB1定时器时钟为84MHz */
        /* 定时器频率 = 系统时钟 / (PSC + 1) / (ARR + 1) */
        uint32_t timClock = 84000000;  /* APB1定时器时钟频率 */
        uint32_t arr = timClock / frequency / 2 - 1;  /* 50%占空比 */
        
        if (arr > 0 && arr < 65536) {
            /* 配置定时器 */
            __HAL_TIM_SET_AUTORELOAD(buzzer->htim, arr);
            __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, arr / 2);
            
            buzzer->isPlaying = true;
            
            /* 等待持续时间 */
            osDelay(duration);
            
            /* 停止PWM */
            __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, 0);
            buzzer->isPlaying = false;
        }
    } else {
        /* 如果没有PWM，使用GPIO简单开关 */
        Buzzer_On(buzzer);
        osDelay(duration);
        Buzzer_Off(buzzer);
    }
}

/**
  * @brief  打开蜂鸣器
  * @param  buzzer: 蜂鸣器对象指针
  * @retval None
  */
void Buzzer_On(Buzzer_t *buzzer)
{
    if (buzzer == NULL || !buzzer->enabled) return;
    
    if (buzzer->port != NULL) {
        HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_SET);
    }
    buzzer->isPlaying = true;
}

/**
  * @brief  关闭蜂鸣器
  * @param  buzzer: 蜂鸣器对象指针
  * @retval None
  */
void Buzzer_Off(Buzzer_t *buzzer)
{
    if (buzzer == NULL) return;
    
    if (buzzer->port != NULL) {
        HAL_GPIO_WritePin(buzzer->port, buzzer->pin, GPIO_PIN_RESET);
    }
    
    if (buzzer->htim != NULL) {
        __HAL_TIM_SET_COMPARE(buzzer->htim, buzzer->channel, 0);
    }
    
    buzzer->isPlaying = false;
}

/**
  * @brief  切换蜂鸣器状态
  * @param  buzzer: 蜂鸣器对象指针
  * @retval None
  */
void Buzzer_Toggle(Buzzer_t *buzzer)
{
    if (buzzer == NULL || !buzzer->enabled) return;
    
    if (buzzer->isPlaying) {
        Buzzer_Off(buzzer);
    } else {
        Buzzer_On(buzzer);
    }
}

/**
  * @brief  检查是否正在播放
  * @param  buzzer: 蜂鸣器对象指针
  * @retval 是否正在播放
  */
bool Buzzer_IsPlaying(Buzzer_t *buzzer)
{
    if (buzzer == NULL) return false;
    return buzzer->isPlaying;
}

