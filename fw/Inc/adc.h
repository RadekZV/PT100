/*
 * Library for ADS1120
 */

#ifndef __ADC_INCLUDED__
#define __ADC_INCLUDED__

#include "stm32f0xx_hal.h"
#include "utils.h"

#define ADC_CMD_RESET          0x06 // 0000 011x - reset device
#define ADC_CMD_START          0x08 // 0000 100x - start or restart conversion
#define ADC_CMD_POWERDOWN      0x02 // 0000 001x - power-down
#define ADC_CMD_RDATA          0x10 // 0001 xxxx - read data be command
#define ADC_CMD_RREG           0x20 // 0010 rrnn - read nn register starting at address rr
#define ADC_CMD_WREG           0x40 // 0100 rrnn - write nn register starting at address rr

#define ADC_REG0               0x00
#define ADC_REG1               0x01
#define ADC_REG2               0x02
#define ADC_REG3               0x03

#define ADC_REG0_MUX_AIN0_AIN1 0x00 //0b00000000
#define ADC_REG0_MUX_AIN0_AIN2 0x10 //0b00010000
#define ADC_REG0_MUX_AIN0_AIN3 0x20 //0b00100000
#define ADC_REG0_MUX_AIN1_AIN2 0x30 //0b00110000
#define ADC_REG0_MUX_AIN1_AIN3 0x40 //0b01000000
#define ADC_REG0_MUX_AIN2_AIN3 0x50 //0b01010000
#define ADC_REG0_MUX_AIN1_AIN0 0x60 //0b01100000
#define ADC_REG0_MUX_AIN3_AIN2 0x70 //0b01110000
#define ADC_REG0_MUX_AIN0_AVSS 0x80 //0b10000000
#define ADC_REG0_MUX_AIN1_AVSS 0x90 //0b10010000
#define ADC_REG0_MUX_AIN2_AVSS 0xA0 //0b10100000
#define ADC_REG0_MUX_AIN3_AVSS 0xB0 //0b10110000

/*
 * #define ADC_REG0_MUX_AIN0_AIN3 0b11000000
 * #define ADC_REG0_MUX_AIN0_AIN3 0b11010000
 * #define ADC_REG0_MUX_AIN0_AIN3 0b11100000
 * #define ADC_REG0_MUX_AIN0_AIN3 0b11110000
 */

//#define ADC_REG0_GAIN1   0b00000000
//#define ADC_REG0_GAIN2   0b00000010
#define ADC_REG0_GAIN4   0x04 //0b00000100
//#define ADC_REG0_GAIN8   0b00000110
//#define ADC_REG0_GAIN16  0b00001000
//#define ADC_REG0_GAIN32  0b00001010
//#define ADC_REG0_GAIN64  0b00001100
//#define ADC_REG0_GAIN128 0b00001110

//#define ADC_REG0_PGA_BYPASS_ENABLE 0
#define ADC_REG0_PGA_BYPASS_DISABLE 1


#define ADC_U_REF        1.0
#define ADC_I_REF        1e-3
#define ADC_PRECISION    32768
#define ADC_GAIN         4


#define ADC_PT100_CONST_C0 -245.19
#define ADC_PT100_CONST_C1 2.5293
#define ADC_PT100_CONST_C2 -0.066046
#define ADC_PT100_CONST_C3 4.0422E-3
#define ADC_PT100_CONST_C4 -2.0697E-6
#define ADC_PT100_CONST_C5 -0.025422
#define ADC_PT100_CONST_C6 1.6883E-3
#define ADC_PT100_CONST_C7 -1.3601E-6

#define ADC_LIMIT_MIN    9948
#define ADC_LIMIT_MAX    24593

void adc_init(void);
HAL_StatusTypeDef adc_reset(void);
HAL_StatusTypeDef adc_start(void);
void adc_get_sample(void);
void adc_buffer_clear(uint8_t buffer[]);

#endif // __ADC_INCLUDED__
