#include "nrf_gpio.h"      
#include "nrf_delay.h"
#include "NRF5x_SPI.h"

#if defined  (NRF51)
	#include "nrf.h"
#elif defined  (NRF52)	
	#include "nrf52.h"
	#include "nrf52_bitfields.h"
#endif

void SPI_Init(void) {
	
#if defined  (NRF51)
	
	nrf_gpio_cfg_input(SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);  //MISO	
	nrf_gpio_cfg_output(SPI_MOSI_PIN); 	//MOSI
	nrf_gpio_cfg_output(SPI_SCK_PIN); 	//SCK
	nrf_gpio_cfg_output(SPI_RST_PIN);	//RST
	nrf_gpio_cfg_output(SPI_CS_PIN); 	//CS
	
	nrf_gpio_pin_set(SPI_MISO_PIN);  	//MISI
	nrf_gpio_pin_set(SPI_MOSI_PIN); 	//MOSI
	nrf_gpio_pin_set(SPI_SCK_PIN); 	//SCK
	nrf_gpio_pin_set(SPI_RST_PIN);		//RST
	nrf_gpio_pin_set(SPI_CS_PIN);  	//CS
	
	NRF_SPI0->POWER = SPI_POWER_POWER_Enabled;	
	NRF_SPI0->PSELMOSI = 0;
	NRF_SPI0->PSELMISO = 1;
	NRF_SPI0->PSELSCK = 2;
	NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_K125;
	NRF_SPI0->CONFIG = 	4;
	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Enabled;
	NRF_SPI0->EVENTS_READY = 0;
	
#elif defined  (NRF52)	
	nrf_gpio_cfg_default(SPI_MISO_PIN); //MISO
	nrf_gpio_cfg_output(SPI_MOSI_PIN); 	//MOSI
	nrf_gpio_cfg_output(SPI_SCK_PIN); 	//SCK
	nrf_gpio_cfg_output(SPI_RST_PIN);	//RST
	nrf_gpio_cfg_output(SPI_CS_PIN); 	//CS
	nrf_gpio_cfg_output(SPI_DC_PIN); 	//DC
	nrf_gpio_cfg_input(SPI_BUSY_PIN, GPIO_PIN_CNF_PULL_Disabled << GPIO_PIN_CNF_PULL_Pos); //BUSY

	nrf_gpio_pin_set(SPI_MISO_PIN);  	//MISO
	nrf_gpio_pin_set(SPI_MOSI_PIN); 	//MOSI
	nrf_gpio_pin_set(SPI_SCK_PIN); 	//SCK
	nrf_gpio_pin_set(SPI_RST_PIN);		//RST
	nrf_gpio_pin_set(SPI_CS_PIN);  	//CS
	
	NRF_SPI0->PSEL.MISO = SPI_MISO_PIN;
	NRF_SPI0->PSEL.MOSI = SPI_MOSI_PIN;
	NRF_SPI0->PSEL.SCK  =	SPI_SCK_PIN;
	NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M8 << SPI_FREQUENCY_FREQUENCY_Pos;
	NRF_SPI0->CONFIG = (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos) |
											(SPI_CONFIG_CPHA_Leading << SPI_CONFIG_CPHA_Pos) |	
											(SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);	
	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos;			
	NRF_SPI0->EVENTS_READY = 0;
#endif
}
/**/
uint8_t SPI_RW(uint8_t data) {
	uint8_t ret;
	if(NRF_SPI0->EVENTS_READY == 0)
	{
		NRF_SPI0->TXD = (uint8_t)data;
	}
	while(NRF_SPI0->EVENTS_READY != 1);
	ret = (uint8_t)(NRF_SPI0->RXD);
	NRF_SPI0->EVENTS_READY = 0;
	return ret;
}
/**/
uint8_t SPI_RW_Buff(uint8_t* rx_buff, uint8_t* tx_buff, uint8_t len)
{
	uint8_t i=0;
	while(len--){
		rx_buff[i] = SPI_RW(tx_buff[i]);
		i++;
	}
	return 0;
}/**/






