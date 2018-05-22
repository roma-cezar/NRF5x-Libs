#ifndef NRF_SPI
#define NRF_SPI

#include <stdint.h>

//#define		SPI_MOSI_PIN			3  // MOSI
//#define   SPI_MISO_PIN			4  // MISO
//#define		SPI_SCK_PIN				28  // SCK
//#define 	SPI_RST_PIN 			29  // RST
//#define		SPI_SCE_PIN				30  // CE

// Arduino shield
#define		SPI_MOSI_PIN			23  // MOSI
#define   SPI_MISO_PIN			24  // MISO
#define		SPI_SCK_PIN				25  // SCK
#define 	SPI_RST_PIN 			29  // RST
#define		SPI_SCE_PIN				22  // CE
/**********************************************************************/
#define 	CS_DESELECT   		nrf_gpio_pin_set(SPI_SCE_PIN)
#define 	CS_SELECT     		nrf_gpio_pin_clear(SPI_SCE_PIN)


void SPI_Init(void);
uint8_t SPI_RW(uint8_t data);
uint8_t SPI_RW_Buff(uint8_t* rx_buff, uint8_t* tx_buff, uint8_t len);


#endif