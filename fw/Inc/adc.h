#ifndef __ADC_INCLUDED__
#define __ADC_INCLUDED__

#define ADC_CMD_RESET     0b00000110 // 0000 011x - reset device
#define ADC_CMD_START     0b00001000 // 0000 100x - start or restart conversion
#define ADC_CMD_POWERDOWN 0b00000010 // 0000 001x - power-down
#define ADC_CMD_RDATA     0b00010000 // 0001 xxxx - read data be command
#define ADC_CMD_RREG      0b00100000 // 0010 rrnn - read nn register starting at address rr
#define ADC_CMD_WREG      0b01000000 // 0100 rrnn - write nn register starting at address rr

#define ADC_REG0 0x00
#define ADC_REG1 0x01
#define ADC_REG2 0x02
#define ADC_REG3 0x03

void ADS1120_init(void);

#endif // __ADC_INCLUDED__
