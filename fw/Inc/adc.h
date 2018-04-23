/*
 * Library for ADS1120
 */

#ifndef __ADC_INCLUDED__
#define __ADC_INCLUDED__

#include "stm32f0xx_hal.h"

#define ADC_CMD_RESET     0x06 // 0000 011x - reset device
#define ADC_CMD_START     0x08 // 0000 100x - start or restart conversion
#define ADC_CMD_POWERDOWN 0x02 // 0000 001x - power-down
#define ADC_CMD_RDATA     0x10 // 0001 xxxx - read data be command
#define ADC_CMD_RREG      0x20 // 0010 rrnn - read nn register starting at address rr
#define ADC_CMD_WREG      0x40 // 0100 rrnn - write nn register starting at address rr

#define ADC_REG0 0x00
#define ADC_REG1 0x01
#define ADC_REG2 0x02
#define ADC_REG3 0x03

void adc_init(void);
HAL_StatusTypeDef adc_start(void);

#endif // __ADC_INCLUDED__
