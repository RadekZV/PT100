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

uint16_t adc_calculate_temp(uint16_t prumer)
{
    char buffer[80];
    double voltage, resistance, temperature;
    uint8_t i;
    int16_t t      = 0, r = 0;
    //int16_t sample = ~((((uint16_t) msb) << 8) + lsb) + 1;  //získaný vzorek měření v rozsahu 0 až 2e15
                                                            //nutno provést dvojkový doplněk, jelikož
                                                            //měření probíhá pro "záporné hodnoty napětí"
    //for (i = 0, i  ) napsat funkci na počítání teploty z průměru x hodnot, aby to nebylo v terminálu tak rychlé
    if (prumer > ADC_LIMIT_MIN && prumer < ADC_LIMIT_MAX)
    {
        voltage     = ((ADC_U_REF / ADC_PRECISION) * ((double) prumer)) / ADC_GAIN; // max U = 0,188 V
        resistance  = voltage / ADC_I_REF;
        temperature = adc_res_to_temp(resistance);

        r = resistance; // max R = 250ohm >> max temperature = 240 °C
        t = (int16_t) temperature;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 0); // Red
        /*HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 1); // Green
        HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, 0); // Green
        HAL_Delay(200);
        */

        snprintf(buffer, 80, "\t\t\t\t%4d Ohm", r);
        debug(buffer);
        snprintf(buffer, 80, "\t%+4d dC\n", t);
        debug(buffer);
    }
    else
    {
        debug("Chyba!!!\n");
        debug("Rozpojeni Zkrat Porucha\n");
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 1); // Red
        /*HAL_Delay(200);
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, 0); // Red
        HAL_Delay(200); */
        
    }

    return t;
}

void adc_get_sample(void)
{
    //HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 1); //Blik pri kazdem novem vzorku
    debug("SPI start receive\n");
    if (HAL_SPI_TransmitReceive(&hspi1, adc_tx_data, adc_rx_data, 2, ADC_SPI_TIMEOUT) == HAL_OK)
    {
        debug("SPI start receive ok\n");
        adc_average_sample(adc_rx_data[0], adc_rx_data[1]);
    }
    else
    {
        debug("SPI start receive error\n");
    }
    //HAL_GPIO_WritePin(LED_GREEN_GPIO_Port, LED_GREEN_Pin, 0); //zhasnout led
}

void adc_average_sample(uint8_t msb, uint8_t lsb)
{
    int16_t sample = ~((((uint16_t) msb) << 8) + lsb) + 1;  //získaný vzorek měření v rozsahu 0 až 2e15
                                                            //nutno provést dvojkový doplněk, jelikož
                                                            //měření probíhá pro "záporné hodnoty napětí"
    uint16_t sample_average ;
    uint16_t sample_average_stop ;
    int8_t i ;

    if (i < 20)
    {
        sample_average = sample_average + sample;
        i++;
    }
    else if (i=20)
    {
        sample_average_stop = sample_average / 21;
        sample_average = 0;
        i = 0;
        adc_calculate_temp(sample_average_stop);
    }
    else
    {
        i = 0;
        sample_average = 0;
        sample_average_stop = 0;
    }
    
    //return sample_average_stop;
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
