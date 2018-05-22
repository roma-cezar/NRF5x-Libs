#if defined (NRF51822)
		#include "nrf51.h"
		#include "nrf51_bitfields.h"
#elif defined (NRF52832)
		#include "nrf52.h"
		#include "nrf52_bitfields.h"
#endif
#include "boards.h"
#include "nrf.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "SEGGER_RTT.h"
#include "ESP8266.h"

uint8_t rx_buf[BUF_MAX_SIZE];

uint8_t my_ip[16];
uint16_t bytes_rx=0;
bool rx_data_rdy = false;

#if defined (FREERTOS)
	#include "SEGGER_RTT_Conf.h"
	#include "FreeRTOS.h"
	#include "task.h"
	#include "queue.h"
	#include "semphr.h"
	#define LONG_TIME 0xffff
	SemaphoreHandle_t m_uart_data_ready = NULL;
#endif

#if defined (NRF51822)
void UART0_IRQHandler(void)
#elif defined (NRF52832)
void UARTE0_UART0_IRQHandler(void)
#endif
{
		#ifdef FREERTOS
			static long xHigherPriorityTaskWoken = pdFALSE;
		#endif
		if(NRF_UART0->EVENTS_RXDRDY == 1)
		{
			NRF_UART0->EVENTS_RXDRDY = 0;
			rx_buf[bytes_rx] = (uint8_t)(NRF_UART0->RXD);
			bytes_rx++;
			//if(bytes_rx++ > BUF_MAX_SIZE) bytes_rx=0;
			#ifdef FREERTOS
			xSemaphoreGiveFromISR( m_uart_data_ready, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
			#else
			rx_data_rdy = true;
			#endif
		}
//		if(NRF_UART0->EVENTS_RXTO == 1)
//		{
//			NRF_UART0->EVENTS_RXTO = 0;
//			__NOP();
//		}
}
/**/

bool ESP8266_AtCmd(uint8_t* databack, const char* cmd, const char* answer)
{
	bool ret = false;
	uint8_t err_cnt = 0;
	bytes_rx = 0;
	memset(databack, 0, BUF_MAX_SIZE);
	// Отправляем АТ комманду
	if(cmd!=NULL){
			ESP8266_Serial_Print((uint8_t*)cmd);
			ESP8266_Serial_Print("\r\n");
	}
	// Если ждем ответ
	if(answer!=NULL)
	{
		// Запускаем прием данных
		NRF_UART0->TASKS_STARTRX = 1;
		// Ждем нужного ответа
		while(!strstr((const char*)rx_buf, answer))
		{
			#ifdef FREERTOS
			if(xSemaphoreTake(m_uart_data_ready, ( TickType_t )15000) == pdTRUE)
			#else
			err_cnt++;
			if(rx_data_rdy)
			#endif
			{
				#ifndef FREERTOS
				rx_data_rdy = false;
				#endif
				if(strstr((const char*)rx_buf, "ERROR"))
				{
					SEGGER_RTT_WriteString(0, "AT command error!\r\n");
					ret = false;
					break;
				}
			}
			#ifdef FREERTOS
			else
			{
				SEGGER_RTT_WriteString(0, "Semaphore timeout!\r\n");
				//if(err_cnt == 6) NVIC_SystemReset();
			}
			#endif
		}
		// Останавливаем прием данных
		NRF_UART0->TASKS_STOPRX = 1;
	}
	// Копируем данные в буффер обмена
	if(databack!=NULL){
		// Проверяем эхо - если ответ содержит отправленную команду
		if(strstr((const char*)rx_buf, cmd))
		{
			memcpy(databack, rx_buf, BUF_MAX_SIZE);
		}
		else
		{
			SEGGER_RTT_WriteString(0, "Error answer cmd or null back pointer!\r\n");
		}
	}
	memset(rx_buf, 0 , BUF_MAX_SIZE);
	rx_data_rdy = false;
	bytes_rx = 0;
	err_cnt = 0;
	ret = true;
	return ret;
}

/**/
char *ESP8266_Init(const char* SSID, const char* PASS)
{
	char answer[BUF_MAX_SIZE];
	char text[32];
	char ip[16];
	
	memset(answer, 0 , BUF_MAX_SIZE);
	
	vTaskDelay(1200);
	if(ESP8266_AtCmd(answer, "AT+RST", "OK"))
	{
		SEGGER_RTT_WriteString(0, answer);
		SEGGER_RTT_WriteString(0, "\r\n");
	}
	vTaskDelay(1200);
	
	if(ESP8266_AtCmd(answer, "AT", "OK"))
	{
		SEGGER_RTT_WriteString(0, answer);
		SEGGER_RTT_WriteString(0, "\r\n");
	}
	
	/*if(ESP8266_AtCmd(answer, "AT+GMR", "OK"))
	{
		SEGGER_RTT_WriteString(0, answer);
		SEGGER_RTT_WriteString(0, "\r\n");
	}

	if(ESP8266_AtCmd(answer, "AT+CWLAP", "OK")){
		SEGGER_RTT_WriteString(0, (const char*)answer);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
	}	*/
	
	memset(text, 0 , 32);
	sprintf(text, "AT+CWJAP=\"%s\",\"%s\"", SSID, PASS); //"AT+CWJAP=\"dd-wrt\",\"bora-bora04\""
	if(ESP8266_AtCmd(answer, text, "OK")){
		SEGGER_RTT_WriteString(0, (const char*)answer);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
	}	
	
	memset(ip, 0 , 16);
	if(ESP8266_AtCmd(answer, "AT+CIFSR", "OK")){
		SEGGER_RTT_WriteString(0, (const char*)answer);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
		
		int ip_start_pos = strstr((char*)answer, "+CIFSR:STAIP" ) - answer + 1;
		int ip_end_pos = strstr((char*)answer, "+CIFSR:STAMAC" ) - answer + 1;
		strncpy(ip, answer + ip_start_pos + 13, ip_end_pos - ip_start_pos - 17);
	}	
	
	/*if(ESP8266_AtCmd(answer, "AT+PING=\"192.168.1.1\"", "OK")){
		SEGGER_RTT_WriteString(0, (const char*)answer);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
	}	*/
	return ip;
}
/**/
void ESP8266_Serial_Config(uint32_t baud)
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

void ESP8266_Serial_Send(uint8_t data)
{
	NRF_UART0->TASKS_STARTTX = 1;
	NRF_UART0->TXD = data;
	while(!(NRF_UART0->EVENTS_TXDRDY));
	NRF_UART0->EVENTS_TXDRDY = 0;
	NRF_UART0->TASKS_STOPTX = 1;
}

void ESP8266_Serial_Print(uint8_t *data)
{
	uint16_t i = 0;
	while(data[i])
	{
		ESP8266_Serial_Send(data[i]);
		i++;
	}
}

void FlushRx(void)
{
	memset(rx_buf, 0, BUF_MAX_SIZE);
	rx_data_rdy = false;
	bytes_rx = 0;
}