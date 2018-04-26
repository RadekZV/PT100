#ifndef __UTILS_INCLUDED__
#define __UTILS_INCLUDED__

#include "stm32f0xx_hal.h"

#ifdef __GNUC__
# define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
# define PUTCHAR_PROTOTYPE int fputc(int ch, FILE * f)
#endif /* __GNUC__ */

PUTCHAR_PROTOTYPE;

void debug(char str[]);
void uart_rx_it_start(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
//void adc_average_sample(uint8_t msb, uint8_t lsb;

extern uint8_t adc_main_loop_flag;

#endif // __UTILS_INCLUDED__
