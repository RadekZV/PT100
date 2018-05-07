/*
 * Library for ADS1120
 */

#ifndef __ADC_INCLUDED__
#define __ADC_INCLUDED__

#include "stm32f0xx_hal.h"
#include "utils.h"

// main commands for ADS1120
#define ADC_CMD_RESET               0x06 // 0000 011x - reset device
#define ADC_CMD_START               0x08 // 0000 100x - start or restart conversion
#define ADC_CMD_POWERDOWN           0x02 // 0000 001x - power-down
#define ADC_CMD_RDATA               0x10 // 0001 xxxx - read data be command
#define ADC_CMD_RREG                0x20 // 0010 rrnn - read nn register starting at address rr
#define ADC_CMD_WREG                0x40 // 0100 rrnn - write nn register starting at address rr

//designation registers
#define ADC_REG0                    0x00 // 0000 0000
#define ADC_REG1                    0x01 // 0000 0001
#define ADC_REG2                    0x02 // 0000 0010
#define ADC_REG3                    0x03 // 0000 0011

//setings for REG0
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
#define ADC_REG0_MUX_MODE_VREF      0xC0 // 1100 0000
#define ADC_REG0_MUX_MODE_AVDD_AVSS 0xD0 // 1101 0000
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

//setings for REG1
#define ADC_REG1_DR_NORM_MODE_20SPS	0x00 // 0000 0000

#define ADC_REG1_MODE_NORMAL		0x00 // 0000 0000
#define ADC_REG1_MODE_DUTY_CYCLE	0x08 // 0000 1000
#define ADC_REG1_MODE_TURBO			0x10 // 0001 0000
#define ADC_REG1_MODE_RESERVED		0x18 // 0001 1000

#define ADC_REG1_CM_SINGLE			0x00 // 0000 0000
#define ADC_REG1_CM_CONTINUOUS		0x04 // 0000 0100

#define ADC_REG1_TS_DISABLE			0x00 // 0000 0000
#define ADC_REG1_TS_ENABLE			0x02 // 0000 0010

#define ADC_REG1_BCS_OFF			0x00 // 0000 0000
#define ADC_REG1_BCS_ON				0x01 // 0000 0001

//setings for REG2
#define ADC_REG2_VREF_INTERNAL               0x00 // 0000 0000
#define ADC_REG2_VREF_EXTERNAL_REFP0_REFN0   0x40 // 0100 0000
#define ADC_REG2_VREF_EXTERNAL_REFP1_REFN1   0x80 // 1000 0000
#define ADC_REG2_VREF_ANALOG                 0xC0 // 1100 0000

#define ADC_REG2_FIR_NO             0x00 // 0000 0000
#define ADC_REG2_FIR_SIMULTANEOUS   0x10 // 0001 0000
#define ADC_REG2_FIR_50             0x20 // 0010 0000
#define ADC_REG2_FIR_60             0x30 // 0011 0000

#define ADC_REG2_PSW_OPEN           0x00 // 0000 0000
#define ADC_REG2_PSW_CLOSE          0x08 // 0000 1000

#define ADC_REG2_IDAC_OFF           0x00 // 0000 0000
#define ADC_REG2_IDAC_RESERVED      0x01 // 0000 0001
#define ADC_REG2_IDAC_50u           0x02 // 0000 0010
#define ADC_REG2_IDAC_100u          0x03 // 0000 0011
#define ADC_REG2_IDAC_250u          0x04 // 0000 0100
#define ADC_REG2_IDAC_500u          0x05 // 0000 0101
#define ADC_REG2_IDAC_1000u         0x06 // 0000 0110
#define ADC_REG2_IDAC_1500u         0x07 // 0000 0111

//setings for REG3
#define ADC_REG3_I1MUX_DISABLED     0x00 // 0000 0000
#define ADC_REG3_I1MUX_AIN0_REFP1   0x20 // 0010 0000
#define ADC_REG3_I1MUX_AIN1         0x40 // 0100 0000
#define ADC_REG3_I1MUX_AIN2         0x60 // 0110 0000
#define ADC_REG3_I1MUX_AIN3_REFN1   0x80 // 1000 0000
#define ADC_REG3_I1MUX_REFP0        0xA0 // 1010 0000
#define ADC_REG3_I1MUX_REFN0        0xC0 // 1100 0000
#define ADC_REG3_I1MUX_RESERVED     0xE0 // 1110 0000

#define ADC_REG3_I2MUX_DISABLED     0x00 // 0000 0000
#define ADC_REG3_I2MUX_AIN0_REFP1   0x04 // 0000 0100
#define ADC_REG3_I2MUX_AIN1         0x08 // 0000 1000
#define ADC_REG3_I2MUX_AIN2         0x0C // 0000 1100
#define ADC_REG3_I2MUX_AIN3_REFN1   0x10 // 0001 0000
#define ADC_REG3_I2MUX_REFP0        0x14 // 0001 0100
#define ADC_REG3_I2MUX_REFN0        0x18 // 0001 1000
#define ADC_REG3_I2MUX_RESERVED     0x1C // 0001 1100

#define ADC_REG3_DRDYM_ON           0x00 // 0000 0000
#define ADC_REG3_DRDYM_Off          0xo2 // 0000 0010

#define ADC_REG3_RESERVED           0x00 // 0000 0000

//constant for calculate impedance
#define ADC_U_REF     1.0
#define ADC_U_REF_INTER     2.048
#define ADC_I_REF     1e-3
#define ADC_PRECISION 32768
#define ADC_GAIN      4
#define ADC_LIMIT_MIN      9000
#define ADC_LIMIT_MAX      27600
#define ADC_EXTREF_MIN     23600
#define ADC_EXTREF_MAX     30150
#define ADC_AIN0_MIN     17000
#define ADC_AIN0_MAX     22000
#define ADC_AIN1_MIN     19000
#define ADC_AIN1_MAX     25000
#define ADC_UNAP_MIN     12000
#define ADC_UNAP_MAX     14000
#define ADC_UREF_MIN     3200
#define ADC_UREF_MAX     4800
#define TEMP_STEP     0.03125
#define TEMP_BORDER     8192

//constant for calculate temperature
#define ADC_PT100_CONST_C0 -245.19
#define ADC_PT100_CONST_C1 2.5293
#define ADC_PT100_CONST_C2 -0.066046
#define ADC_PT100_CONST_C3 4.0422E-3
#define ADC_PT100_CONST_C4 -2.0697E-6
#define ADC_PT100_CONST_C5 -0.025422
#define ADC_PT100_CONST_C6 1.6883E-3
#define ADC_PT100_CONST_C7 -1.3601E-6



void adc_init(void);
void adc_init_extref(void);
void adc_init_ain1(void);
void adc_init_ain0(void);
void adc_init_internal_temperature(void);
void adc_init_unap(void);
void adc_init_uref(void);
HAL_StatusTypeDef adc_reset(void);
HAL_StatusTypeDef adc_start(void);
void adc_get_sample(void);
void adc_buffer_clear(uint8_t buffer[]);
double adc_calculate_temp(uint8_t msb, uint8_t lsb);
double adc_calculate_voltage(uint8_t msb, uint8_t lsb);
double adc_calculate_voltage2(uint8_t msb, uint8_t lsb);
double adc_calculate_voltage3(uint8_t msb, uint8_t lsb);
double adc_average_temp(double temperature);
double adc_calculate_unap(uint8_t msb, uint8_t lsb);

#endif // __ADC_INCLUDED__
