#include "utils.h"

extern UART_HandleTypeDef huart1;

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart1, (uint8_t *) &ch, 1, 0xFFFF);
    return ch;
}


void debug(char str[])
{
    for (uint8_t i=0; str[i]; i++)
        HAL_UART_Transmit(&huart1, (uint8_t *) &str[i], 1, 0xFFFF);
}
