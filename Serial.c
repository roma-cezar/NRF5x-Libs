#if defined (NRF51822)
		#include "nrf51.h"
		#include "nrf51_bitfields.h"
#elif defined (NRF52832)
		#include "nrf52.h"
		#include "nrf52_bitfields.h"
#endif
#include "boards.h"
#include "nrf.h"
#include "Serial.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

uint8_t databuf[256];
uint8_t bytes_rx=0;
uint8_t c;
bool rx_data_rdy = false;

#if defined (FREERTOS)
	#include "FreeRTOS.h"
	#include "task.h"
	#include "queue.h"
	#include "semphr.h"
	#define LONG_TIME 0xffff
	SemaphoreHandle_t xUARTSemaphore = NULL;
#endif

#if defined (NRF51822)
void UART0_IRQHandler(void)
#elif defined (NRF52832)
void UARTE0_UART0_IRQHandler(void)
#endif
{
		#if defined (FREERTOS)
			static long xHigherPriorityTaskWoken = pdFALSE;
		#endif
		if(NRF_UART0->EVENTS_RXDRDY == 1)
		{
			NRF_UART0->EVENTS_RXDRDY = 0;
			c = (uint8_t)(NRF_UART0->RXD);
			if(bytes_rx++ > 255) bytes_rx=0;
			if(c != 0x0A)
			{
				databuf[bytes_rx-1] = c;
				rx_data_rdy = false;
			}
			else
			{
				databuf[bytes_rx-1] = c;
#ifdef FREERTOS
					xSemaphoreGiveFromISR( xUARTSemaphore, &xHigherPriorityTaskWoken );
					portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
#else
					rx_data_rdy = true;
#endif
			}
		}
}
/**/

void FlushRx(void)
{
	memset(databuf, 0, 64);
	rx_data_rdy = false;
	bytes_rx = 0;
}
/**/
bool CheckCmd(const char* cmd)
{
	bool flag = false;
#ifndef FREERTOS
	NRF_UART0->TASKS_STARTRX = 1;
	if(rx_data_rdy)
	{
		NRF_UART0->TASKS_STOPRX = 1;
#endif
		if(strstr((const char*)databuf, cmd))
		{
			flag = true;
		}
		else
		{
			flag = false;
		}
#ifndef FREERTOS
	}
#endif
	return flag;
}
/**/
void getstring(uint8_t* data)
{
	NRF_UART0->TASKS_STARTRX = 1;
#ifndef FREERTOS
  while(!rx_data_rdy);
#endif
	NRF_UART0->TASKS_STOPRX = 1;
	memcpy(data, databuf, bytes_rx);
	memset(databuf, 0, 64);
	rx_data_rdy = false;
	bytes_rx = 0;
}
/**/
void Serial_Config(uint32_t baud)
{
	
	NRF_UART0->PSELRXD = RX_PIN_NUMBER; 
	NRF_UART0->PSELTXD = TX_PIN_NUMBER;
	
	NRF_UART0->CONFIG = UART_CONFIG_HWFC_Disabled | UART_CONFIG_PARITY_Excluded;
	switch(baud){
		case 1200:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud1200; 
			break;
		case 2400:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud2400; 
			break;
		case 4800:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud4800; 
			break;
		case 9600:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud9600; 
			break;
		case 19200:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud19200; 
			break;
		case 38400:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud38400; 
			break;
		case 57600:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud57600; 
			break;
		case 115200:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud115200;
			break;
		case 230400:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud230400; 
			break;
		case 460800:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud460800; 
			break;
		case 921600:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud921600; 
			break;
		case 1000000:
			NRF_UART0->BAUDRATE = UART_BAUDRATE_BAUDRATE_Baud1M; 
			break;
	}
	
	
	NRF_UART0->INTENCLR = 0xFFFFFFFF;
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos;
	NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
	
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
/**/
void Serial_Send(uint8_t data)
{
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TXD = data;
	while(!(NRF_UART0->EVENTS_TXDRDY));
	NRF_UART0->EVENTS_TXDRDY = 0;
	NRF_UART0->TASKS_STOPTX = 1;
}
/**/
void Serial_Send_Buff(uint8_t *data, uint16_t size)
{
	for(uint16_t i=0; i<size; i++)
	{
		Serial_Send(data[i]);
	}
}
/**/
void Serial_Print(uint8_t *data)
{
	uint16_t i = 0;
	while(data[i])
	{
		Serial_Send(data[i]);
		i++;
	}
}
/**/
void Serial_PrintChar(char ch, FILE *stream)
{
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TXD = ch;
	while(!(NRF_UART0->EVENTS_TXDRDY));
	NRF_UART0->EVENTS_TXDRDY = 0;
	NRF_UART0->TASKS_STOPTX = 1;
}