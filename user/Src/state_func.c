#include "led.h"
#include "stm32h7xx_hal.h"
#include "state_func.h"



SystemState current_state = STATE_IDLE;

void state_running_run(void);
void state_alarm_run(void);
void state_idle_run(void);

typedef void (*state_func)(void);

 const state_func state_table[] = 
{
    [STATE_IDLE]    = state_idle_run,
    [STATE_RUNNING] = state_running_run,
    [STATE_ALARM]   = state_alarm_run,       
};

void state_idle_run(void)
{
    led_off(LED1_PIN|LED2_PIN|LED3_PIN|LED4_PIN);
}
void state_running_run(void)
{  
    uint8_t LED_PIN=LED1_PIN;
    while(LED_PIN<=LED4_PIN)
    {
        led_on(LED_PIN);
        HAL_Delay(200U);
        led_off(LED_PIN);
        HAL_Delay(200U);
        LED_PIN=LED_PIN<<1U;
    }
   
}
   

void state_alarm_run(void)
{
    led_on(LED1_PIN|LED2_PIN|LED3_PIN|LED4_PIN);
    HAL_Delay(50U);
    led_off(LED1_PIN|LED2_PIN|LED3_PIN|LED4_PIN);
    HAL_Delay(50U);
}