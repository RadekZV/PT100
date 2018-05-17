#include "adc.h"
#include <stdio.h>
#include <stdlib.h>

#define ADC_SPI_TIMEOUT 0xFFFF

#define ADC_BUFFER_SIZE 2
uint8_t adc_rx_data[ADC_BUFFER_SIZE];
uint8_t adc_tx_data[ADC_BUFFER_SIZE];
uint8_t stav = 0;
double uref = 1;

extern SPI_HandleTypeDef hspi1;

// setting one register
HAL_StatusTypeDef adc_set_reg(uint8_t reg, uint8_t value)
{
    uint8_t config[2];
    config[0] = ADC_CMD_WREG | (reg << 2); // nn + 1
    config[1] = value;
    return HAL_SPI_Transmit(&hspi1, config, 2, ADC_SPI_TIMEOUT);
}

//setting more register
HAL_StatusTypeDef adc_set_regs(uint8_t start_reg, uint8_t number_reg, uint8_t values[])
{
    uint8_t config[5];
    uint8_t i;

    config[0] = ADC_CMD_WREG | (start_reg << 2) | (number_reg - 1); // nn + 1

    for (i = 0; i < number_reg; i++)
    {
        config[i + 1] = values[i];
    }

    return HAL_SPI_Transmit(&hspi1, config, number_reg + 1, ADC_SPI_TIMEOUT);   //delay
}

//reset ADC
HAL_StatusTypeDef adc_reset(void)
{
    uint8_t config[] = { ADC_CMD_RESET };
    HAL_Delay(1);
    HAL_NVIC_DisableIRQ(EXTI0_1_IRQn);  // disable interruption
    return HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
}

//start conversion
HAL_StatusTypeDef adc_start(void)
{
    uint8_t config[] = { ADC_CMD_START };
    HAL_StatusTypeDef status;
    status = HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
    HAL_Delay(100);
    HAL_NVIC_EnableIRQ(EXTI0_1_IRQn);   // enable interruption
    return status;
}

// function for read measurement data
HAL_StatusTypeDef adc_read_data(void)
{
    uint8_t config[] = { ADC_CMD_RDATA };
    return HAL_SPI_Transmit(&hspi1, config, 1, ADC_SPI_TIMEOUT);
}

//function for conversion impedance to temperature
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

//function for calculate average temperature from X sample
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

//function for calculate temperature from measurement sample
double adc_calculate_temp(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION1 20      // number for reduction speed of print temperature
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage = 0, resistance = 0, temperature = 0;
    double t = 0, r = 0, v = 0;
    int16_t sample = (~((((uint16_t) msb) << 8) + lsb) + 1);//13108;# 100 ohm
    message_reduction++;

    if (sample > ADC_LIMIT_MIN && sample < ADC_LIMIT_MAX)       //range for calculate temperature from
                                                                //measurement sample
    {
        
        resistance = (((ADC_R_REF * ((double)sample))/ADC_PRECISION)/4);
        if (resistance < 100.00001 && resistance > 99.995)
        {
            temperature = 0.0000;
            voltage = resistance * ADC_I_REF;
        }
        else
        {
            temperature = adc_res_to_temp(resistance);//adc_average_temp(adc_res_to_temp(resistance));
            voltage = resistance * ADC_I_REF;
        }
        
        r = resistance;
        t = temperature;
        v = voltage;
        
        if (message_reduction == ADC_MESSAGE_REDUCTION1)
        {
            led_green_on();
            message_reduction = 0;
            snprintf(buffer, 80, "\t%4.4f Ohm", r);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.4f dC", t);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.4f V", v);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r\r", (sample));
            debug(buffer);
        }
            
        }
    
    else    // if range is outside setting temperature (-50 to +250 dC)
            // is evaluated short or open circuit
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION1)
        {
            message_reduction = 0;
            debug("Sample value error: ");
            debug("open or short circuit\n\r");
        }
        led_red_off();
    }

    return t;
}


//function for calculate voltage from measurement sample
double adc_calculate_voltage(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION 10      // number for reduction speed of print voltage
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage;
    double v = 0;
    int16_t sample = ((((uint16_t) msb) << 8) + lsb);

    message_reduction++;

    if (sample > ADC_EXTREF_MIN && sample < ADC_EXTREF_MAX)       //range for calculate temperature from
                                                                //measurement sample
    {
        voltage     = ((ADC_U_REF / ADC_PRECISION) * ((double) sample)) / ADC_GAIN;
        
        v = voltage;
        
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            led_green_on();
            message_reduction = 0;
            
            snprintf(buffer, 80, "\t Extern reference 0.21 V =  %+4.4f V", v);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r\r", (sample));
            debug(buffer);
            
            
        }
    }
    else    // if range is outside setting temperature (0.18 to 0.23 V)
            // is evaluated error
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            voltage = ((ADC_U_REF / ADC_PRECISION) * ((double) sample)) / ADC_GAIN;
            v = voltage;
            
            message_reduction = 0;
            debug("Sample value error: ");
            snprintf(buffer, 80, "\t EXTERN REFERENCE =  %+4.4f V", v);
            debug(buffer);
            debug("\tERROR!!! HIGH OR LOW VOLTAGE\n\r\r");
        }
        led_red_off();
    }

    return v;
}

double adc_calculate_voltage_ain1(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION 10      // number for reduction speed of print voltage
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage;
    double v = 0;
    int16_t sample = ((((uint16_t) msb) << 8) + lsb);

    message_reduction++;

    if (sample > ADC_AIN1_MIN && sample < ADC_AIN1_MAX)       //range for calculate voltage from
                                                              //measurement sample
    {
        voltage     = ((ADC_U_REF_INTER / ADC_PRECISION) * ((double) sample));
        
        v = voltage;
        
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            led_green_on();
            message_reduction = 0;
            
            snprintf(buffer, 80, "\t AIN1 to AVSS\t\t =  %+4.4f V", v);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r\r", (sample));
            debug(buffer);
            
            
        }
    }
    else    // if range is outside setting (1.18 to 1,56)
            // is evaluated short or open circuit
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            voltage     = ((ADC_U_REF_INTER / ADC_PRECISION) * ((double) sample));
            v = voltage;
            
            message_reduction = 0;
            debug("Sample value error: ");
            snprintf(buffer, 80, "\t AIN1 to AVSS =  %+4.4f V", v);
            debug(buffer);
            debug("\tERROR!!! HIGH OR LOW VOLTAGE\n\r");
        }
        led_red_off();
    }

    return v;
}

double adc_calculate_voltage_ain0(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION 10      // number for reduction speed of print temperature
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage;
    double v = 0;
    int16_t sample = ((((uint16_t) msb) << 8) + lsb);

    message_reduction++;

    if (sample > ADC_AIN0_MIN && sample < ADC_AIN0_MAX)       //range for calculate voltage AIN0 to AVSS
                                                              //measurement sample
    {
        voltage     = ((ADC_U_REF_INTER / ADC_PRECISION) * ((double) sample));
        
        v = voltage;
        
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            led_green_on();
            message_reduction = 0;
            
            snprintf(buffer, 80, "\t AIN0 to AVSS\t\t =  %+4.4f V", v);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
            debug(buffer);
            
            
        }
    }
    else    // if range is outside setting temperature (1.2 V)
            // is evaluated ERROR
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            voltage     = ((ADC_U_REF_INTER / ADC_PRECISION) * ((double) sample));
            v = voltage;
            
            message_reduction = 0;
            debug("Sample value error: ");
            snprintf(buffer, 80, "\t AIN0 to AVSS =  %+4.4f V", v);
            debug(buffer);
            debug("\tERROR!!! HIGH OR LOW VOLTAGE\n\r");
        }
        led_red_off();
    }

    return v;
}

double adc_calculate_internal_temperature(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION2 10      // number for reduction speed of print temperature
    static uint16_t message_reduction = 0;

    char buffer[80];
    double i_temp;
    double t = 0;
    uint16_t sample = ((((uint16_t) msb) << 6) + lsb);

    message_reduction++;

    if (sample <= TEMP_BORDER)               //border between positive and negative temperature
                                            // here is positive temperature            
    {
        i_temp     = ((TEMP_STEP) * ((double) sample));
        
        t = i_temp;
        
        if (sample >= 3200) //temperature is high then 100 dC
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                led_green_off();
                led_red_on();
                message_reduction = 0;
                
                snprintf(buffer, 80, "\t Internal temperature =  %+4.4f dC", t);
                debug(buffer);
                debug("\tTEMPERATURE IS VERY HIGH!!!\n\r");
                led_red_off();
            }
        }
        else    //temperature is between 0 - 100 dC
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                led_green_on();
                message_reduction = 0;
                
                snprintf(buffer, 80, "\t Internal temperature\t =  %+4.4f dC", t);
                debug(buffer);
                snprintf(buffer, 80, "\t%+4.0f  sample in DEC", ((double)sample));
                debug(buffer);
                snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
                debug(buffer);
                
                
            }
        }
    }
    else if ( sample > TEMP_BORDER && sample < TEMP_LOW) // here is negative temperature
    {   
        sample = ((~(sample)) + 1) - TEMP_CORRECT ;
        i_temp     = ((- TEMP_STEP) * ((double) sample));
        t = i_temp;
        
        if (sample > TEMP_MINUS_SIXTY) // temperature is low then -60 dC
        {
            if (message_reduction == ADC_MESSAGE_REDUCTION2)
            {
                led_green_off();
                led_red_on();
                message_reduction = 0;
                
                snprintf(buffer, 80, "\t Internal temperature =  %+4.4f dC", t);
                debug(buffer);
                debug("\tTEMPERATURE IS VERY LOW!!!\n\r");
                led_red_off();
            }
        }
        else
        {
           if (message_reduction == ADC_MESSAGE_REDUCTION2)
        {
            led_green_on();
            message_reduction = 0;
            
            snprintf(buffer, 80, "\t Internal temperature\t =  %+4.4f dC", t);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
            debug(buffer);
               
        } 
        }
        
        
    }
    else    
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION2)
        {
            message_reduction = 0;
            debug("Sample value error: ");
            debug("TEMPERATURE ERROR\n\r");
        }
        led_red_off();
    }

    return t;
}

double adc_calculate_unap(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION 10      // number for reduction speed of print voltage
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage;
    double v = 0;
    int16_t sample = ((((uint16_t) msb) << 8) + lsb);

    message_reduction++;

    if (sample > ADC_UNAP_MIN && sample < ADC_UNAP_MAX)       //range for calculate voltage from
                                                                //measurement sample
    {
        voltage     = (((ADC_U_REF_INTER) / ADC_PRECISION) * (((double) sample) )) * 4;
        
        v = voltage;
        
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            led_green_on();
            message_reduction = 0;
            
            snprintf(buffer, 80, "\t Supply voltage\t\t =  %+4.4f V", v);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
            debug(buffer);
            
            
        }
    }
    else    // if range is outside setting temperature (3.0 to 3.5)
            // is evaluated ERROR
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            voltage     = (((ADC_U_REF_INTER) / ADC_PRECISION) * (((double) sample) )) * 4;
            v = voltage;
            
            message_reduction = 0;
            debug("Sample value error: ");
            snprintf(buffer, 80, "\t Supply voltage =  %+4.4f V", v);
            debug(buffer);
            debug("\tERROR!!! HIGH OR LOW VOLTAGE\n\r");
        }
        led_red_off();
    }

    return v;
}

double adc_calculate_uref(uint8_t msb, uint8_t lsb)
{
    #define ADC_MESSAGE_REDUCTION 10      // number for reduction speed of print voltage
    static uint16_t message_reduction = 0;

    char buffer[80];
    double voltage;
    double v = 0;
    int16_t sample = ((((uint16_t) msb) << 8) + lsb);

    message_reduction++;

    if (sample > ADC_UREF_MIN && sample < ADC_UREF_MAX)       //range for calculate voltage from
                                                                //measurement sample
    {
        voltage     = (((ADC_U_REF_INTER) / ADC_PRECISION) * (((double) sample) )) * 4;
        
        v = voltage;
        uref = voltage;
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            led_green_on();
            message_reduction = 0;
            
            snprintf(buffer, 80, "\t Reference voltage\t =  %+4.4f V", v);
            debug(buffer);
            snprintf(buffer, 80, "\t%+4.0f  sample in DEC", ((double)sample));
            debug(buffer);
            snprintf(buffer, 80, "\t%4x sample in HEX\n\r", (sample));
            debug(buffer);
            
            
        }
    }
    else    // if range is outside setting temperature (-50 to +250 dC)
            // is evaluated short or open circuit
    {   led_green_off();
        led_red_on();
        if (message_reduction == ADC_MESSAGE_REDUCTION)
        {
            voltage     = (((ADC_U_REF_INTER) / ADC_PRECISION) * (((double) sample) )) * 4;
            v = voltage;
            
            message_reduction = 0;
            debug("Sample value error: ");
            snprintf(buffer, 80, "\t Reference voltage =  %+4.4f V", v);
            debug(buffer);
            debug("\tERROR!!! HIGH OR LOW VOLTAGE\n\r");
        }
        led_red_off();
    }

    return v;
}


//function for receive sample from ADC
void adc_get_sample(void)
{
    //debug("SPI start receive\n\r");
    if (HAL_SPI_TransmitReceive(&hspi1, adc_tx_data, adc_rx_data, 2, ADC_SPI_TIMEOUT) == HAL_OK)
    {
        if (stav == 1)
        {
            //debug("SPI start receive ok\n\r");
            adc_calculate_temp(adc_rx_data[0], adc_rx_data[1]);
        }
        else if (stav == 2)
        {
            adc_calculate_voltage(adc_rx_data[0], adc_rx_data[1]);
        }
        else if (stav == 3)
        {
            adc_calculate_voltage_ain1(adc_rx_data[0], adc_rx_data[1]);
        }
        else if (stav == 4)
        {
            adc_calculate_voltage_ain0(adc_rx_data[0], adc_rx_data[1]);
        }
        else if (stav == 5)
        {
            adc_calculate_internal_temperature(adc_rx_data[0], adc_rx_data[1]);
        }
        else if (stav == 6)
        {
            adc_calculate_unap(adc_rx_data[0], adc_rx_data[1]);
        }
        else if (stav == 7)
        {
            adc_calculate_uref(adc_rx_data[0], adc_rx_data[1]);
        }
        
        
        //char buffer[80];
        //snprintf(buffer, 80, "\t%+4.0f stav/n", ((double)stav));
        //debug(buffer);
        
        
    }
    else
    {
        debug("SPI start receive error\n\r");
    }
    //led_green_off();
}


void adc_buffer_clear(uint8_t buffer[])
{
    for (uint16_t i = 0; i < ADC_BUFFER_SIZE; i++)
        buffer[i] = 0;
}

//function for initialization ADC
void adc_init(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 1;
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN0_AIN1 | ADC_REG0_GAIN4 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_EXTERNAL_REFP0_REFN0 | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset error\n\r\r");
    }
    else
    {
        //debug("SPI reset ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config error\n\r\r");
    }
    else
    {
        //debug("SPI config ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
       // debug("SPI start error\n\r\r");
    }
    else
    {
        debug("Measurement temperature:\n\r\r");
    }
}

void adc_init_extref(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 2;
    
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN2_AVSS | ADC_REG0_GAIN4 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_EXTERNAL_REFP0_REFN0 | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset2 error\n\r\r");
    }
    else
    {
        //debug("SPI reset2 ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config2 error\n\r\r");
    }
    else
    {
        //debug("SPI config2 ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
        //debug("SPI start2 error\n\r\r");
    }
    else
    {
        debug("Values of voltage and temperature for control function:\n\r\r");
    }
}

void adc_init_ain1(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 3;
    
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN1_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset2 error\n\r\r");
    }
    else
    {
        //debug("SPI reset2 ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config2 error\n\r\r");
    }
    else
    {
        //debug("SPI config2 ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
        //debug("SPI start2 error\n\r\r");
    }
    else
    {
        //debug("SPI start2 ok\n\r\r");
    }
}

void adc_init_ain0(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 4;
    
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN0_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset2 error\n\r\r");
    }
    else
    {
        //debug("SPI reset2 ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config2 error\n\r\r");
    }
    else
    {
        //debug("SPI config2 ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
        //debug("SPI start2 error\n\r\r");
    }
    else
    {
        //debug("SPI start2 ok\n\r\r");
    }
}

void adc_init_internal_temperature(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 5;
    
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_AIN2_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_ENABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset2 error\n\r\r");
    }
    else
    {
        //debug("SPI reset2 ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config2 error\n\r\r");
    }
    else
    {
        //debug("SPI config2 ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
        //debug("SPI start2 error\n\r\r");
    }
    else
    {
        //debug("SPI start2 ok\n\r\r");
    }
}

void adc_init_unap(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 6;
    
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_MODE_AVDD_AVSS | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_INTERNAL | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset2 error\n\r\r");
    }
    else
    {
        //debug("SPI reset2 ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config2 error\n\r\r");
    }
    else
    {
        //debug("SPI config2 ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
        //debug("SPI start2 error\n\r\r");
    }
    else
    {
        //debug("SPI start2 ok\n\r\r");
    }
}


void adc_init_uref(void)
{
    adc_buffer_clear(adc_rx_data);
    adc_buffer_clear(adc_tx_data);
    stav = 7;
    
    //setting registers
    uint8_t config[] = {
        ADC_REG0_MUX_MODE_VREF | ADC_REG0_GAIN1 | ADC_REG0_PGA_BYPASS_DISABLE,
        ADC_REG1_DR_NORM_MODE_20SPS | ADC_REG1_MODE_NORMAL | ADC_REG1_CM_CONTINUOUS | ADC_REG1_TS_DISABLE | ADC_REG1_BCS_OFF, //0x04, // 0000 0100
        ADC_REG2_VREF_EXTERNAL_REFP0_REFN0 | ADC_REG2_FIR_NO | ADC_REG2_PSW_OPEN | ADC_REG2_IDAC_1000u, //0x46, // 0100 0110
        ADC_REG3_I1MUX_AIN3_REFN1 | ADC_REG3_I2MUX_DISABLED | ADC_REG3_DRDYM_ON | ADC_REG3_RESERVED, //0x80  // 1000 0000
    };


    if (adc_reset() != HAL_OK)
    {
        //debug("SPI reset2 error\n\r\r");
    }
    else
    {
        //debug("SPI reset2 ok\n\r\r");
    }

    if (adc_set_regs(ADC_REG0, 4, config) != HAL_OK)
    {
        //debug("SPI config2 error\n\r\r");
    }
    else
    {
        //debug("SPI config2 ok\n\r\r");
    }

    if (adc_start() != HAL_OK)
    {
        //debug("SPI start2 error\n\r\r");
    }
    else
    {
        //debug("SPI start2 ok\n\r\r");
    }
}

