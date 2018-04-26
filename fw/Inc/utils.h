#ifndef __UTILS_INCLUDED__
#define __UTILS_INCLUDED__

#include "stm32f0xx_hal.h"

#define led_red_on()       HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET)
#define led_red_off()      HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_RESET)
#define led_red_toggle()   HAL_GPIO_TogglePin(LED_RED_GPIO_Port, LED_RED_Pin)

#define led_green_on()     HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_SET)
#define led_green_off()    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, GPIO_PIN_RESET)
#define led_green_toggle() HAL_GPIO_TogglePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin)

void debug(char str[]);
void uart_rx_it_start(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

extern uint8_t adc_main_loop_flag;

#endif // __UTILS_INCLUDED__
