/**
 * @file    led.h
 * @brief   LED 驱动头文件。
 *          头文件放声明和宏，具体实现放在 led.c。
 */
#ifndef LED_H
#define LED_H

#include "stm32h7xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 宏定义：给 LED 使用的端口和引脚起名字 */
#define LED_GPIO_PORT GPIOB
#define LED1_PIN      GPIO_PIN_3
#define LED2_PIN      GPIO_PIN_4
#define LED3_PIN      GPIO_PIN_5
#define LED4_PIN      GPIO_PIN_6

/*
 * LED 引脚掩码：每颗 LED 对应一个位，可按位或组合
 * 例如 led_on(LED1_PIN | LED3_PIN) 同时点亮 LED1 和 LED3
 */
void led_on(uint8_t led_mask);
void led_off(uint8_t led_mask);

#ifdef __cplusplus
}
#endif

#endif /* LED_H */
