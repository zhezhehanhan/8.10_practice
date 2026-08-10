/**
 * @file    led.c
 * @brief   LED 驱动实现文件。
 */
#include "led.h"

/*
 * 点亮 LED，led_mask 支持按位或组合多颗 LED。
 * 例如 led_on(LED1_PIN | LED3_PIN) 同时点亮 LED1 和 LED3。
 */
void led_on(uint8_t led_mask)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, led_mask, GPIO_PIN_SET);
}

/*
 * 熄灭 LED，led_mask 支持按位或组合多颗 LED。
 * 例如 led_off(LED2_PIN | LED4_PIN) 同时熄灭 LED2 和 LED4。
 */
void led_off(uint8_t led_mask)
{
    HAL_GPIO_WritePin(LED_GPIO_PORT, led_mask, GPIO_PIN_RESET);
}
