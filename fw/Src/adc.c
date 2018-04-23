#include "adc.h"

extern SPI_HandleTypeDef hspi1;

void ADS1120_init(void)
{
  // send WREG
	// send 4 bytes
	//HAL_SPI_Transmit() vrací HAL_OK kontrolovat a vypisovat chyby na uart
	
	uint8_t config[5];
	config[0] = ADC_CMD_WREG | (ADC_REG0 << 2) | 4;
	config[1] = 0b00000110; // zápis do reg 0
	config[2] = 0b00000100;
	config[3] = 0b01000110;
	config[4] = 0b10000000;
	// SPI_HandleTypeDef *hspi, uint8_t *pData, uint16_t Size, uint32_t Timeout
	HAL_SPI_Transmit(&hspi1, config, 5, 0xFFFF);
	
}
