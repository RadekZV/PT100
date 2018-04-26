/*
 * Library for ADS1120
 */

#ifndef __ADC_INCLUDED__
#define __ADC_INCLUDED__

#include "stm32f0xx_hal.h"
#include "utils.h"

#define ADC_CMD_RESET               0x06 // 0000 011x - reset device
#define ADC_CMD_START               0x08 // 0000 100x - start or restart conversion
#define ADC_CMD_POWERDOWN           0x02 // 0000 001x - power-down
#define ADC_CMD_RDATA               0x10 // 0001 xxxx - read data be command
#define ADC_CMD_RREG                0x20 // 0010 rrnn - read nn register starting at address rr
#define ADC_CMD_WREG                0x40 // 0100 rrnn - write nn register starting at address rr

#define ADC_REG0                    0x00 // 0000 0000
#define ADC_REG1                    0x01 // 0000 0001
#define ADC_REG2                    0x02 // 0000 0010
#define ADC_REG3                    0x03 // 0000 0011

#define ADC_REG0_MUX_AIN0_AIN1      0x00 // 0000 0000
#define ADC_REG0_MUX_AIN0_AIN2      0x10 // 0001 0000
#define ADC_REG0_MUX_AIN0_AIN3      0x20 // 0010 0000
#define ADC_REG0_MUX_AIN1_AIN2      0x30 // 0011 0000
#define ADC_REG0_MUX_AIN1_AIN3      0x40 // 0100 0000
#define ADC_REG0_MUX_AIN2_AIN3      0x50 // 0101 0000
#define ADC_REG0_MUX_AIN1_AIN0      0x60 // 0110 0000
#define ADC_REG0_MUX_AIN3_AIN2      0x70 // 0111 0000
#define ADC_REG0_MUX_AIN0_AVSS      0x80 // 1000 0000
#define ADC_REG0_MUX_AIN1_AVSS      0x90 // 1001 0000
#define ADC_REG0_MUX_AIN2_AVSS      0xA0 // 1010 0000
#define ADC_REG0_MUX_AIN3_AVSS      0xB0 // 1011 0000
#define ADC_REG0_MUX_MODE_12        0xC0 // 1100 0000
#define ADC_REG0_MUX_MODE_13        0xD0 // 1101 0000
#define ADC_REG0_MUX_MODE_14        0xE0 // 1110 0000
#define ADC_REG0_MUX_RESERVED       0xF0 // 1111 0000

#define ADC_REG0_GAIN1              0x00 // 0000 0000
#define ADC_REG0_GAIN2              0x02 // 0000 0010
#define ADC_REG0_GAIN4              0x04 // 0000 0100
#define ADC_REG0_GAIN8              0x06 // 0000 0110
#define ADC_REG0_GAIN16             0x08 // 0000 1000
#define ADC_REG0_GAIN32             0x0A // 0000 1010
#define ADC_REG0_GAIN64             0x0C // 0000 1100
#define ADC_REG0_GAIN128            0x0E // 0000 1110

#define ADC_REG0_PGA_BYPASS_ENABLE  0x00 // 0000 0000
#define ADC_REG0_PGA_BYPASS_DISABLE 0x01 // 0000 0001


#define ADC_U_REF     1.0
#define ADC_I_REF     1e-3
#define ADC_PRECISION 32768
#define ADC_GAIN      4


#define ADC_PT100_CONST_C0 -245.19
#define ADC_PT100_CONST_C1 2.5293
#define ADC_PT100_CONST_C2 -0.066046
#define ADC_PT100_CONST_C3 4.0422E-3
#define ADC_PT100_CONST_C4 -2.0697E-6
#define ADC_PT100_CONST_C5 -0.025422
#define ADC_PT100_CONST_C6 1.6883E-3
#define ADC_PT100_CONST_C7 -1.3601E-6

#define ADC_LIMIT_MIN      9948
#define ADC_LIMIT_MAX      24593

void adc_init(void);
HAL_StatusTypeDef adc_reset(void);
HAL_StatusTypeDef adc_start(void);
void adc_get_sample(void);
void adc_buffer_clear(uint8_t buffer[]);

#endif // __ADC_INCLUDED__
