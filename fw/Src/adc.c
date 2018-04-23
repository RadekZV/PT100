#include "adc.h"

extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart1;

void adc_init(void)
{
  // send WREG
	// send 4 bytes
	//HAL_SPI_Transmit() vrací HAL_OK kontrolovat a vypisovat chyby na uart

	uint8_t config[5];
	config[0] = ADC_CMD_WREG | (ADC_REG0 << 2) | 4;
	config[1] = 0x06;  //0b00000110; // zápis do reg 0
	config[2] = 0x04;  //0b00000100;
	config[3] = 0x46;  //0b01000110;
	config[4] = 0x80;  //0b10000000;
	if ( HAL_SPI_Transmit(&hspi1, config, 5, 0xFFFF) != HAL_OK)
        HAL_UART_Transmit(&huart1, (uint8_t *) "SPI config error\n", 17, 0xFFFF);
    else
        HAL_UART_Transmit(&huart1, (uint8_t *) "SPI config ok\n", 14, 0xFFFF);

}
