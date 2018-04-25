#include "adc.h"
#include <stdio.h>
#include <stdlib.h>

#define ADC_SPI_TIMEOUT 0xFFFF

extern SPI_HandleTypeDef hspi1;


#define ADC_BUFFER_SIZE 2
uint8_t adc_rx_data[ADC_BUFFER_SIZE];
uint8_t adc_tx_data[ADC_BUFFER_SIZE];

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
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);
    return status;
}

HAL_StatusTypeDef adc_read_data(void)
{
    uint8_t config[] = { ADC_CMD_RDATA };
    return HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
}

uint16_t adc_calculate_temp(uint8_t msb, uint8_t lsb)
{
    char buffer[10];
    double voltage;
    double resistance;
    uint16_t r = 0;
    uint16_t sample = (((uint16_t) msb) << 8) + lsb;

    if (sample > ADC_LIMIT_MIN && sample < ADC_LIMIT_MAX)
    {
        voltage    = ((ADC_U_REF / ADC_PRECISION) * ((double) sample)) / ADC_GAIN; // max U = 0,188 V
        resistance = voltage / ADC_I_REF;
        r = resistance; // max R = 250ohm >> max temperature = 240 °C

        itoa(sample, buffer, 10);
        debug("                        sample: ");
        debug(buffer);
        itoa(r, buffer, 10);
        debug("      res: ");
        debug(buffer);
        debug("\n");
    }
    else
    {
        debug("Chyba!!!\n");
        debug("Rozpojeni Zkrat Porucha\n");
    }

     return r;
}

void adc_get_sample(void)
{
    HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1);
    debug("SPI start receive\n");
    // adc_read_data();
    // HAL_Delay(1);
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
    //adc_start();
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
        0x04, // 0000 0100 - zápis do reg 0
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
