#include "adc.h"
#include <stdio.h>
#include <stdlib.h>

#define ADC_SPI_TIMEOUT 0xFFFF

#define ADC_BUFFER_SIZE 2
uint8_t adc_rx_data[ADC_BUFFER_SIZE];
uint8_t adc_tx_data[ADC_BUFFER_SIZE];

extern SPI_HandleTypeDef hspi1;

HAL_StatusTypeDef adc_set_reg(uint8_t reg, uint8_t value)
{
    uint8_t config[2];
    config[0] = ADC_CMD_WREG | (reg << 2); // nn + 1
    config[1] = value;
    return HAL_SPI_Transmit(&hspi1, config, 2, ADC_SPI_TIMEOUT);
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

    return HAL_SPI_Transmit(&hspi1, config, number_reg + 1, ADC_SPI_TIMEOUT);
}

HAL_StatusTypeDef adc_reset(void)
{
    uint8_t config[] = { ADC_CMD_RESET };
    HAL_Delay(1);
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);
    return HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
}

HAL_StatusTypeDef adc_start(void)
{
    uint8_t config[] = { ADC_CMD_START };
    HAL_StatusTypeDef status;
    status = HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
    HAL_Delay(100);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    return status;
}

HAL_StatusTypeDef adc_read_data(void)
{
    uint8_t config[] = { ADC_CMD_RDATA };
    return HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
}

double adc_res_to_temp(double res)
{
    double a, b;

    a = (res * ADC_PT100_CONST_C7) + ADC_PT100_CONST_C6;
    a = (a * res) + ADC_PT100_CONST_C5;
    a = (a * res) + 1;
    b = (res * ADC_PT100_CONST_C4) + ADC_PT100_CONST_C3;
    b = (b * res) + ADC_PT100_CONST_C2;
    b = (b * res) + ADC_PT100_CONST_C1;
    b = b * res;

    return (b / a) + ADC_PT100_CONST_C0;
}

double adc_average_temp(double temperature)
{
    #define ADC_AVERAGE_BUFFER_SIZE 20

    static double average_buffer[ADC_AVERAGE_BUFFER_SIZE];
    static uint8_t average_index = 0;
    static uint8_t average_start = 1;

    uint8_t sum_index;
    double sum;

    if (average_start)
    {
        average_start = 0;
        for (average_index = 0; average_index < ADC_AVERAGE_BUFFER_SIZE; average_index++)
            average_buffer[average_index] = 0;
        average_index = 0;
    }

    average_buffer[average_index++] = temperature;

    if (average_index >= ADC_AVERAGE_BUFFER_SIZE)
        average_index = 0;

    for (sum_index = 0, sum = 0; sum_index < ADC_AVERAGE_BUFFER_SIZE; sum_index++)
        sum += average_buffer[sum_index];

    return sum / ADC_AVERAGE_BUFFER_SIZE;
}

double adc_calculate_temp(uint8_t msb, uint8_t lsb)
{
    char buffer[80];
    double voltage, resistance, temperature;
    int16_t t      = 0, r = 0;
    int16_t sample = ~((((uint16_t) msb) << 8) + lsb) + 1;

    if (sample > ADC_LIMIT_MIN && sample < ADC_LIMIT_MAX)
    {
        voltage     = ((ADC_U_REF / ADC_PRECISION) * ((double) sample)) / ADC_GAIN;
        resistance  = voltage / ADC_I_REF;
        temperature = adc_average_temp(adc_res_to_temp(resistance));

        r = resistance;
        t = (int16_t) temperature;

        snprintf(buffer, 80, "\t\t\t\t%4d Ohm", r);
        debug(buffer);
        snprintf(buffer, 80, "\t%+4d dC\n", t);
        debug(buffer);
    }
    else
    {
        debug("Sample value error:\n");
        debug("\topen or short circuit\n");
    }

    return t;
}

void adc_get_sample(void)
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
    debug("SPI start receive\n");
    if (HAL_SPI_TransmitReceive(&hspi1, adc_tx_data, adc_rx_data, 2, ADC_SPI_TIMEOUT) == HAL_OK)
    {
        debug("SPI start receive ok\n");
        adc_calculate_temp(adc_rx_data[0], adc_rx_data[1]);
    }
    else
    {
        debug("SPI start receive error\n");
    }
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0);
}

void adc_buffer_clear(uint8_t buffer[])
{
    for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++)
        buffer[i] = 0;
}

void adc_init(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);

    uint8_t config[] = {
        ADC_REG0_MUX_AIN0_AIN1 | ADC_REG0_GAIN4 | ADC_REG0_PGA_BYPASS_DISABLE,
        0x04, // 0000 0100
        0x46, // 0100 0110
        0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        debug("SPI reset error\n\r");
    }
    else
    {
        debug("SPI reset ok\n\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        debug("SPI config error\n\r");
    }
    else
    {
        debug("SPI config ok\n\r");
    }

    if (adc_start() != HAL_OK)
    {
        debug("SPI start error\n\r");
    }
    else
    {
        debug("SPI start ok\n\r");
    }
} /* adc_init */
