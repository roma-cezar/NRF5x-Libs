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
#include "esp8266.h"

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
	#define LONG_TIME 	0xffff
	SemaphoreHandle_t 	m_uart_data_ready = NULL;
#else
#include "nrf_delay.h"
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
	bool ret = true;
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
			while(!rx_data_rdy)
			#endif
			{
				if(strstr((const char*)rx_buf, "ERROR"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, "AT command error!\r\n");
					#endif
					ret = false;
					break;
				}
				#ifndef FREERTOS
				rx_data_rdy = false;
				nrf_delay_ms(1);
				if(err_cnt==15000)
				{
					err_cnt = 0;
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, "AT answer timeout!\r\n");
					#endif
					ret = false;
					break;
				}
				err_cnt++;
				#endif
			}
			#ifdef FREERTOS
			else
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, "Semaphore timeout!\r\n");
				#endif
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
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, "Error answer cmd or null back pointer!\r\n");
			#endif
		}
	}
	memset(rx_buf, 0 , BUF_MAX_SIZE);
	rx_data_rdy = false;
	bytes_rx = 0;
	err_cnt = 0;
	return ret;
}

/**/
char *ESP8266_Wlan_Start(const char* SSID, const char* PASS)
{
	char answer[BUF_MAX_SIZE];
	char cmd[32];
	char ip[16];
		
	#if defined (FREERTOS)
		vTaskDelay(100);
	#else
		nrf_delay_ms(100);
	#endif
	
	if(ESP8266_AtCmd(answer, "AT+RST", "OK"))
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, "Reseted succesfuly!\r\n");
		#endif
	}
	
	#if defined (FREERTOS)
		vTaskDelay(1200);
	#else
		nrf_delay_ms(1200);
	#endif
	
	
	if(ESP8266_AtCmd(answer, "AT", "OK"))
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, "ESP8266 is OK!\r\n");
		#endif
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
	
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, (const char*)"Connecting to AP...\r\n");
	#endif
	
	memset(cmd, 0 , 32);
	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", SSID, PASS); //"AT+CWJAP=\"dd-wrt\",\"bora-bora04\""
	if(ESP8266_AtCmd(answer, cmd, "OK")){
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"Connected to AP succesfuly!\r\n");
		#endif
	}	
	
	memset(ip, 0 , 16);
	if(ESP8266_AtCmd(answer, "AT+CIFSR", "OK")){
		int ip_start_pos = strstr((char*)answer, "+CIFSR:STAIP" ) - answer + 1;
		int ip_end_pos = strstr((char*)answer, "+CIFSR:STAMAC" ) - answer + 1;
		strncpy(ip, answer + ip_start_pos + 13, ip_end_pos - ip_start_pos - 17);
	}	
	return ip;
}
/**/
bool ESP8266_Wlan_Stop(void)
{
	bool ret = false;
	if(ESP8266_AtCmd(NULL, "AT+CWQAP", "OK")){
		ret = true;
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"Disconnected from AP succesfuly!\r\n");
		#endif
	}	
	return ret;
}
/**/
bool ESP8266_Session_Open(const char* type, const char* ip, uint16_t port)
{
	char answer[BUF_MAX_SIZE];
	char cmd[64];
	bool ret = false;
	memset(cmd, 0 , 64);
	
	sprintf(cmd, "AT+PING=\"%s\"", ip);
	if(ESP8266_AtCmd(answer, cmd, "OK"))
	{
		
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"Ping OK!\r\n");	
		memset(cmd, 0 , 64);
		sprintf(cmd, "Connecting to %s:%d...\r\n", ip, port);
		SEGGER_RTT_WriteString(0, (const char*)cmd);
		#endif
		
		memset(cmd, 0 , 64);
		sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%d", type, ip, port);
		if(ESP8266_AtCmd(answer, cmd, "OK"))
		{
			if(strstr((const char*)answer, "CONNECT"))
			{	
				if(ESP8266_AtCmd(answer, "AT+CIPMODE=0", "OK"))
				{
					#ifdef ESP_DEBUG
					memset(cmd, 0 , 64);
					sprintf(cmd, "Connected to %s:%d succesfuly\r\n", ip, port);
					SEGGER_RTT_WriteString(0, (const char*)cmd);
					#endif
					ret = true;
				}
			}
			else
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"Connecting error!\r\n");
				#endif
				ret = false;
			}
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"Server not avaliable!\r\n");
			#endif
			ret = false;
		}
	}	
	else
	{
		#ifdef ESP_DEBUG
		memset(cmd, 0 , 64);
		sprintf(cmd, "IP: %s is not avaliable!\r\n", ip);
		SEGGER_RTT_WriteString(0, (const char*)cmd);
		#endif
		ret = false;
	}
	return ret;
}
/**/
bool ESP8266_Session_Close(void)
{
	char answer[BUF_MAX_SIZE];
	bool ret = false;	
	if(ESP8266_AtCmd(answer, "AT+CIPCLOSE", "OK"))
	{
		if(strstr((const char*)answer, "CLOSED"))
		{
			#ifdef ESP_DEBUG		
			SEGGER_RTT_WriteString(0, (const char*)"Disconnected from server!\r\n");
			#endif
			ret = true;	
		}
		else
		{
			#ifdef ESP_DEBUG	
			SEGGER_RTT_WriteString(0, (const char*)"Disconnecting error!\r\n");
			#endif
			ret = false;	
		}
	}
	else
	{
		#ifdef ESP_DEBUG	
		SEGGER_RTT_WriteString(0, (const char*)"Disconnect, AT error!\r\n");
		#endif
	}
	return ret;
}
/**/
bool ESP8266_Session_Send(const char* msg)
{
	char answer[BUF_MAX_SIZE];
	bool ret = false;	
	char cmd[64];
	memset(cmd, 0 , 64);
	sprintf(cmd, "AT+CIPSEND=%d", strlen(msg));
	if(ESP8266_AtCmd(answer, cmd, ">"))
	{
		ESP8266_Serial_Print((uint8_t*)msg);
		if(ESP8266_AtCmd(NULL, NULL, "SEND OK"))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"Data sended succesfuly!\r\n");
			#endif
			ret = true;
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"Error while sending data!\r\n");
			#endif
			ret = false;
		}
	}
	else
	{
		ret = false;
	}
	return ret;
}
/**/
uint8_t ESP8266_Session_Status(void)
{
	char answer[BUF_MAX_SIZE];
	uint8_t ret = 0;	
	char cmd[64];
	memset(cmd, 0 , 64);
	if(ESP8266_AtCmd(answer, "AT+CIPSTATUS", "OK"))
	{
		if(strstr((const char*)answer, "STATUS:2"))
		{
			ret = 2; // Station is connected to an AP and its IP is obtained
		}
		else if(strstr((const char*)answer, "STATUS:3"))
		{
			ret = 3; // TCP or UDP session is open
		}
		else if(strstr((const char*)answer, "STATUS:4"))
		{
			ret = 4; //  TCP or UDP session is close
		}
		else if(strstr((const char*)answer, "STATUS:5"))
		{
			ret = 5; //  NOT connect to an AP
		}
	}
	else
	{
		ret = 0;
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"Error: unknown status!\r\n");
		#endif
	}
	#ifdef ESP_DEBUG
	sprintf(cmd, "Connection status: %d\r\n", ret);
	SEGGER_RTT_WriteString(0, (const char*)cmd);
	#endif
	return ret;
}
/**/
bool ESP8266_GET_Req(const char* uri)
{
	char answer[BUF_MAX_SIZE];
	bool ret = false;	
	char msg[256];
	char cmd[64];
	memset(msg, 0 , 256);
	memset(cmd, 0 , 64);
	// /22dd4423c4a34bf5b989a6342cc691d6/update/V0?value=2.0
	sprintf(msg, "GET %s HTTP/1.1\r\n\r\n", uri);
	sprintf(cmd, "AT+CIPSEND=%d", strlen(msg));
	if(ESP8266_AtCmd(answer, cmd, ">"))
	{
		ESP8266_Serial_Print((uint8_t*)msg);
		if(ESP8266_AtCmd(NULL, NULL, "SEND OK"))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"Message sended!\r\n");
			#endif
			if(ESP8266_AtCmd(NULL, NULL, "+IPD"))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"Server answer recieved!\r\n");
				#endif
				if(ESP8266_AtCmd(answer, NULL, "\r\n\r\n"))
				{
					#ifdef ESP_DEBUG
					//SEGGER_RTT_WriteString(0, (const char*)answer);
					//SEGGER_RTT_WriteString(0, (const char*)"\r\n");
					#endif
					if(strstr((const char*)answer, "200"))
					{
						#ifdef ESP_DEBUG
						SEGGER_RTT_WriteString(0, (const char*)"200\r\n");
						#endif
						ret = true;
					}
					else if(strstr((const char*)answer, "400"))
					{
						#ifdef ESP_DEBUG
						SEGGER_RTT_WriteString(0, (const char*)"400\r\n");
						#endif
						ret = false;
					}
					else if(strstr((const char*)answer, "500"))
					{
						#ifdef ESP_DEBUG
						SEGGER_RTT_WriteString(0, (const char*)"500\r\n");
						#endif
						ret = false;
					}
					else
					{
						#ifdef ESP_DEBUG
						SEGGER_RTT_WriteString(0, (const char*)answer);
					  SEGGER_RTT_WriteString(0, (const char*)"\r\n");
						#endif
						ret = false;
					}
				}
			}
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"Message send error!\r\n");
			#endif
		}
	}
	return ret;	
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
	NVIC_SetPriority(UART0_IRQn, 4);
	NVIC_EnableIRQ(UART0_IRQn);
#elif defined (NRF52832)
  NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
	NVIC_SetPriority(UARTE0_UART0_IRQn, 4);
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