#include "utils.h"
#include "adc.h"

extern UART_HandleTypeDef huart1;

uint8_t adc_main_loop_flag = 0;
uint8_t uart_rx_buffer[2];

void debug(char str[])
{
    for (uint8_t i = 0; str[i]; i++)
        HAL_UART_Transmit(&huart1, (uint8_t *) &str[i], 1, 0xFFFF);
}

void uart_rx_it_start(void)
{
    HAL_UART_Receive_IT(&huart1, uart_rx_buffer, 1);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (uart_rx_buffer[0] == 'r')
    {
        adc_main_loop_flag = 'r';
    }
    else if (uart_rx_buffer[0] == 's')
    {
        adc_main_loop_flag = 's';
    }
    uart_rx_it_start();
}
