#if defined (NRF51822)
		#include "nrf51.h"
		#include "nrf51_bitfields.h"
#elif defined (NRF52832)
		#include "nrf52.h"
		#include "nrf52_bitfields.h"
#endif
#include "boards.h"
#include "nrf.h"
#include "eSERIAL.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t eSERIAL_TXBUFF[MAX_USART_BUFFER];
uint8_t eSERIAL_RXBUFF[MAX_USART_BUFFER];
	
bool tx_end_flag = false;
bool rx_end_flag = false;

uint16_t rx_len = 0;
uint16_t tx_len = 0;

extern void eSERIAL_RxCallback(uint8_t *data, uint8_t len);

#if defined (NRF51822)
void UART0_IRQHandler(void)
#elif defined (NRF52832)
void UARTE0_UART0_IRQHandler(void)
#endif
{
	if(NRF_UARTE0->EVENTS_ENDTX)
	{
		NRF_UARTE0->EVENTS_ENDTX = 0;
		tx_end_flag = true;
	}
	
	if(NRF_UARTE0->EVENTS_RXDRDY)
	{
		NRF_UARTE0->EVENTS_RXDRDY = 0;
	}
	
	if(NRF_UARTE0->EVENTS_RXTO)
	{
		NRF_UARTE0->EVENTS_RXTO = 0;
		NRF_UARTE0->TASKS_FLUSHRX = 1;
		__NOP();
	}
	
	if(NRF_UARTE0->EVENTS_ENDRX)
	{	 
		NRF_UARTE0->EVENTS_ENDRX = 0;
	  rx_end_flag = true;
		//eSERIAL_Recieve(eSERIAL_RxCallback);
		memset(eSERIAL_RXBUFF, 0, MAX_USART_BUFFER);
		//NRF_UARTE0->SHORTS = UARTE_SHORTS_ENDRX_STARTRX_Msk;
	}
	
	if(NRF_UARTE0->EVENTS_ERROR)
	{
		NRF_UARTE0->EVENTS_ERROR = 0;
		__NOP();
	}
}

void Flush(uint8_t *buff, uint16_t len)
{
	memset(buff, 0, len);
}

void eSERIAL_Init(uint32_t baud)
{
	//NRF_UARTE0->
	NRF_UARTE0->PSEL.RXD = RX_PIN_NUMBER; 
	NRF_UARTE0->PSEL.TXD = TX_PIN_NUMBER;
	

	NRF_UARTE0->CONFIG = UARTE_CONFIG_HWFC_Disabled | UARTE_CONFIG_PARITY_Excluded;

	switch(baud){
		case 1200:
			NRF_UART0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud1200; 
			break;
		case 2400:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud2400; 
			break;
		case 4800:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud4800; 
			break;
		case 9600:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud9600; 
			break;
		case 19200:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud19200; 
			break;
		case 38400:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud38400; 
			break;
		case 57600:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud57600; 
			break;
		case 115200:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud115200;
			break;
		case 230400:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud230400; 
			break;
		case 460800:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud460800; 
			break;
		case 921600:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud921600; 
			break;
		case 1000000:
			NRF_UARTE0->BAUDRATE = UARTE_BAUDRATE_BAUDRATE_Baud1M; 
			break;
	}
	
	
		memset(eSERIAL_RXBUFF, 0, MAX_USART_BUFFER);
		memset(eSERIAL_TXBUFF, 0, MAX_USART_BUFFER);
	
		NRF_UARTE0->TXD.PTR = (uint32_t)(eSERIAL_TXBUFF);
		NRF_UARTE0->TXD.MAXCNT = (uint32_t)MAX_USART_BUFFER - 1;
	
		NRF_UARTE0->RXD.PTR = (uint32_t)(eSERIAL_RXBUFF);
		NRF_UARTE0->RXD.MAXCNT = (uint32_t)MAX_USART_BUFFER - 1;
	
	
		NRF_UARTE0->INTENCLR = 0xFFFFFFFF;
		NRF_UARTE0->INTENSET = UARTE_INTENSET_ENDRX_Enabled << UARTE_INTENSET_ENDRX_Pos;
		NRF_UARTE0->INTENSET = UARTE_INTENSET_RXDRDY_Enabled << UARTE_INTENSET_RXDRDY_Pos;
		NRF_UARTE0->INTENSET = UARTE_INTENSET_ENDTX_Enabled << UARTE_INTENSET_ENDTX_Pos;
	  NRF_UARTE0->INTENSET = UARTE_INTENSET_RXTO_Enabled << UARTE_INTENSET_RXTO_Pos;
	  NRF_UARTE0->INTENSET = UARTE_INTENSET_ERROR_Enabled << UARTE_INTENSET_ERROR_Pos;
		NRF_UARTE0->ENABLE = UARTE_ENABLE_ENABLE_Enabled;
		
	#if defined (NRF51822)	
		NVIC_ClearPendingIRQ(UART0_IRQn);
		NVIC_SetPriority(UART0_IRQn, 3);
		NVIC_EnableIRQ(UART0_IRQn);
	#elif defined (NRF52832)
		NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
		NVIC_SetPriority(UARTE0_UART0_IRQn, 3);
		NVIC_EnableIRQ(UARTE0_UART0_IRQn);
	#endif
}

void eSERIAL_StartRX(void)
{
	rx_end_flag = false;
	NRF_UARTE0->TASKS_STARTRX = 1;
}
	
void eSERIAL_StartTX(void)
{
	tx_end_flag = false;
	NRF_UARTE0->TASKS_STARTTX = 1;
}


void eSERIAL_Transmit(uint8_t* data, uint16_t len)
{
	tx_end_flag = false;
	memset(eSERIAL_TXBUFF, 0, MAX_USART_BUFFER);
	memcpy((char*)eSERIAL_TXBUFF, data, len);
	NRF_UARTE0->TXD.MAXCNT = (uint32_t)len;
	NRF_UARTE0->TASKS_STARTTX = 1;
	while(!tx_end_flag);
	tx_end_flag = false;
}

void eSERIAL_Recieve(void (*callback)(uint8_t*, uint8_t))
{
	//while(!rx_end_flag);
	rx_len = (uint16_t)(NRF_UARTE0->RXD.AMOUNT);
	callback(eSERIAL_RXBUFF, rx_len);
}