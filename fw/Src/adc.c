#include "adc.h"

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

HAL_StatusTypeDef adc_set_reg(uint8_t reg, uint8_t value)
{
    uint8_t config[2];
    config[0] = ADC_CMD_WREG | (reg << 2); // nn + 1
    config[1] = value;
    return HAL_SPI_Transmit(&hspi1, config, 2, 0xFFFF);
}

HAL_StatusTypeDef adc_set_regs(uint8_t start_reg, uint8_t number_reg, uint8_t values[])
{
    uint8_t config[5];
    uint8_t i;

    config[0] = ADC_CMD_WREG | (start_reg << 2) | (number_reg - 1); // nn + 1

    for (i = 0; i < number_reg; i++)
    {
        config[i + 1] = values[i];
    }

    return HAL_SPI_Transmit(&hspi1, config, number_reg + 1, 0xFFFF);
}

void adc_init(void)
{
    uint8_t config[] = {
        0x06, // 0000 0110 - zápis do reg 0
        0x04, // 0000 0100
        0x46, // 0100 0110
        0x80  // 1000 0000
    };

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *) "SPI config error\n", 17, 0xFFFF);
    }
    else
    {
        HAL_UART_Transmit(&huart1, (uint8_t *) "SPI config ok\n", 14, 0xFFFF);
    }
}

HAL_StatusTypeDef adc_start(void)
{
    uint8_t config[] = { ADC_CMD_START };
    HAL_Delay(100);
    return HAL_SPI_Transmit(&hspi1, config, 1, 0xFFFF);
}
