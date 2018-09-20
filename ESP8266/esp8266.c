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
extern uint8_t timestamp[32];

uint8_t rx_buf[BUF_MAX_SIZE];
uint8_t my_ip[16];
uint16_t bytes_rx=0;
bool rx_data_rdy = false;
bool uart_err_flag = false;

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
			#ifdef FREERTOS
			xSemaphoreGiveFromISR( m_uart_data_ready, &xHigherPriorityTaskWoken );
			portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
			#else
			rx_data_rdy = true;
			#endif
		}
		else if(NRF_UART0->EVENTS_RXTO == 1)
		{
			NRF_UART0->EVENTS_RXTO = 0;
			#ifdef ESP_DEBUG_LVL2
			SEGGER_RTT_WriteString(0, (const char*)"\r\nUART: RXTO EVENT!\r\n");
			#endif
		}
		else if(NRF_UART0->EVENTS_ERROR == 1)
		{
			#ifdef ESP_DEBUG_LVL2
			SEGGER_RTT_WriteString(0, (const char*)"\r\nUART: ERROR EVENT!\r\n");
			#endif
			uint8_t err=0;
			NRF_UART0->EVENTS_ERROR = 0;
			NRF_UART0->EVENTS_RXDRDY = 0;
			uart_err_flag = true;
		}
}
/**/
bool ESP8266_AtCmd(uint8_t* databack, const char* cmd, const char* answer, uint16_t fuse_time_ms)
{
	bool ret = true;
	uint8_t err_cnt = 0;
	uint32_t answer_timeout = 0;
	uint32_t rx_timeout = 0;
	bytes_rx = 0;
	memset(databack, 0, BUF_MAX_SIZE);
	memset(rx_buf, 0, BUF_MAX_SIZE);
	
	if(cmd!=NULL)
	{
		// Отправляем АТ комманду
		#ifdef ESP_DEBUG_LVL2
		SEGGER_RTT_WriteString(0, (const char*)"\r\nAT >> ");
		SEGGER_RTT_WriteString(0, (const char*)cmd);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
		#endif
	}
	if(answer!=NULL)
	{
		#if defined (NRF51822)	
			NVIC_EnableIRQ(UART0_IRQn);
		#elif defined (NRF52832)
			NVIC_EnableIRQ(UARTE0_UART0_IRQn);
		#endif
		// Запускаем прием данных
		NRF_UART0->TASKS_STARTRX = 1;
	}
	
	if(cmd!=NULL)
	{
		  // Отправляем АТ комманду
			ESP8266_Serial_Print((uint8_t*)cmd);
			ESP8266_Serial_Print("\r\n");
	}
	// Если ждем ответ
	if(answer!=NULL)
	{
		// Ждем нужного ответа
		answer_timeout = 2000000;
		while(!strstr((const char*)rx_buf, answer) && --answer_timeout && !uart_err_flag)
		{
			#ifdef FREERTOS
			if(xSemaphoreTake(m_uart_data_ready, ( TickType_t )15000) == pdTRUE)
			{
				
			}
			else
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"WLAN: Semaphore timeout!\r\n");
				#endif
				//if(err_cnt == 6) NVIC_SystemReset();
			}
			#endif
			
			// Если приняли АТ ошибку
			if(strstr((const char*)rx_buf, "ERROR"))
			{
				
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"\r\nWLAN: AT command error!\r\n");
				#endif
				ret = false;
				break;
			}

		}
		
		// Останавливаем прием данных через 200 мс для приема оставшихся байт данных
		if(fuse_time_ms != NULL)
		{
			#if defined (FREERTOS)
				vTaskDelay(fuse_time_ms);
			#else
				nrf_delay_ms(fuse_time_ms);
			#endif
		}
		NRF_UART0->TASKS_STOPRX = 1;
		#if defined (NRF51822)	
			NVIC_DisableIRQ(UART0_IRQn);
		#elif defined (NRF52832)
			NVIC_DisableIRQ(UARTE0_UART0_IRQn);
		#endif
		
		#ifndef FREERTOS
		// Если время ожидания ответа превышено
		if(answer_timeout==0) 
		{
			ret = false;
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"\r\nWLAN: AT timeout!\r\n");
			#endif
		}
		#endif
	}
	
	// если ошибка УАРТ
	if(uart_err_flag) 
	{
		#ifdef ESP_DEBUG_LVL2
			err = (uint8_t)(NRF_UART0->ERRORSRC);
			char cmd[32];
			memset(cmd, 0 , 32);
			sprintf(cmd, "UART ERROR: %d\r\n", err);
			SEGGER_RTT_WriteString(0, (const char*)cmd);
		#endif
		uart_err_flag=!uart_err_flag;
		ret = false;
	}
	// Копируем данные в буффер обмена
	if(databack!=NULL){
		// Проверяем эхо - если ответ содержит отправленную команду
		memcpy(databack, rx_buf, BUF_MAX_SIZE);
		#ifdef ESP_DEBUG_LVL2
		SEGGER_RTT_WriteString(0, (const char*)"\r\nAT << ");
		SEGGER_RTT_WriteString(0, (const char*)databack);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
		#endif
	}
	memset(rx_buf, 0 , BUF_MAX_SIZE);
	rx_data_rdy = false;
	bytes_rx = 0;
	err_cnt = 0;
	return ret;
}
/**/
bool ESP8266_Preinit(void)
{
	bool ret = false;
	char answer[BUF_MAX_SIZE];
	memset(answer, 0 , BUF_MAX_SIZE);
	uint8_t tries = 3;
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, (const char*)"\r\n");
	SEGGER_RTT_WriteString(0, timestamp);
	SEGGER_RTT_WriteString(0, (const char*)" WLAN: Init");
	#endif
	#if defined (FREERTOS)
		vTaskDelay(200);
	#else
		nrf_delay_ms(200);
	#endif
	
	/*if(ESP8266_AtCmd(answer, "AT+RST", "OK", 50))
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Reseted succesfuly!\r\n");
		#endif
		ret = true;
	}*/
	
	nrf_gpio_pin_clear(29);
	nrf_delay_ms(10);
	nrf_gpio_pin_set(29);
	#if defined (FREERTOS)
		vTaskDelay(2000);
	#else
		nrf_delay_ms(2000);
	#endif
		
	while(--tries && !ret)
	{	
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)".");
		#endif
		if(ESP8266_AtCmd(answer, "AT", "OK", 50))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"\r\n");
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" WLAN: ESP8266 is OK!\r\n");
			#endif
			if(ESP8266_AtCmd(answer, "ATE0", "OK", 50))
			{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" WLAN: Echo off!\r\n");
			#endif
			}
			ret = true;
		}
		
	}
	/*if(ESP8266_AtCmd(answer, "AT+GMR", "OK", 50))
	{
		SEGGER_RTT_WriteString(0, answer);
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: \r\n");
	}

	if(ESP8266_AtCmd(answer, "AT+CWLAP", "OK", 50)){
		SEGGER_RTT_WriteString(0, (const char*)answer);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
	}	*/
	
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, timestamp);
	SEGGER_RTT_WriteString(0, (const char*)" WLAN: Init complete!\r\n");
	#endif
	return ret;
}
/**/
bool ESP8266_Wlan_Start(const char* SSID, const char* PASS)
{
	bool ret = false;
	char answer[BUF_MAX_SIZE];
	char cmd[32];
	uint8_t tries = 3;	
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, timestamp);
	SEGGER_RTT_WriteString(0, (const char*)" WLAN: WLAN start...\r\n");
	#endif
	
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, timestamp);
	SEGGER_RTT_WriteString(0, (const char*)" WLAN: Connecting to AP");
	#endif
	
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(cmd, 0 , 32);
	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", SSID, PASS); //"AT+CWJAP=\"dd-wrt\",\"bora-bora04\""
	
	while(--tries && !ret)
	{	
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)".");
		#endif
		if(ESP8266_AtCmd(answer, cmd, "OK", 50)){
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"\r\n");
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" WLAN: Connected to AP SSID: "); 
			SEGGER_RTT_WriteString(0, (const char*)SSID);
			SEGGER_RTT_WriteString(0, (const char*)" PSWD: ");
			SEGGER_RTT_WriteString(0, (const char*)PASS);
			SEGGER_RTT_WriteString(0, (const char*)" succesfuly!\r\n");
			
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" WLAN: IP "); 
			SEGGER_RTT_WriteString(0, ESP8266_Get_IP());
			SEGGER_RTT_WriteString(0, (const char*)"\r\n");
			
			#endif
			ret = true;
		}	
	}
	
	if(!ret)
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
		#endif
	}
	return ret;
}
/**/
char *ESP8266_Get_IP(void)
{	
	char answer[BUF_MAX_SIZE];
	char ip[16];
	
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(ip, 0 , 16);
	if(ESP8266_AtCmd(answer, "AT+CIFSR", "OK", 50)){
		int ip_start_pos = strstr((char*)answer, "+CIFSR:STAIP" ) - answer + 1;
		int ip_end_pos = strstr((char*)answer, "+CIFSR:STAMAC" ) - answer + 1;
		strncpy(ip, answer + ip_start_pos + 13, ip_end_pos - ip_start_pos - 17);
		return ip;
	}	
	else
	{
		return NULL;
	}
}
/**/
bool ESP8266_Wlan_Stop(void)
{
	char answer[BUF_MAX_SIZE];
	bool ret = false;
	
	memset(answer, 0 , BUF_MAX_SIZE);
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, timestamp);
	SEGGER_RTT_WriteString(0, (const char*)" WLAN: Disconnecting...\r\n");
	#endif
	if(ESP8266_AtCmd(answer, "AT+CWQAP", "OK", 50)){
		ret = true;
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, timestamp);
		SEGGER_RTT_WriteString(0, (const char*)" WLAN: Disconnected from AP succesfuly!\r\n");
		#endif
	}	
	else
	{
		ret = false;
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, timestamp);
		SEGGER_RTT_WriteString(0, (const char*)" WLAN: Disconnecting error!\r\n");
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
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(cmd, 0 , 64);
	
	sprintf(cmd, "AT+PING=\"%s\"", ip);
	if(ESP8266_AtCmd(answer, cmd, "OK", 50))
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, timestamp);
		SEGGER_RTT_WriteString(0, (const char*)" TCP:  Ping OK!\r\n");
		memset(cmd, 0 , 64);
		sprintf(cmd, " TCP:  Connecting to %s:%d...\r\n", ip, port);
		SEGGER_RTT_WriteString(0, timestamp);
		SEGGER_RTT_WriteString(0, (const char*)cmd);
		#endif
		
		memset(cmd, 0 , 64);
		sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%d", type, ip, port);
		if(ESP8266_AtCmd(answer, cmd, "OK", 50))
		{
			if(strstr((const char*)answer, "CONNECT"))
			{	
				if(ESP8266_AtCmd(answer, "AT+CIPMODE=0", "OK", 50))
				{
					#ifdef ESP_DEBUG
					memset(cmd, 0 , 64);
					sprintf(cmd, " TCP:  Connected to %s:%d succesfuly\r\n", ip, port);
					SEGGER_RTT_WriteString(0, timestamp);
					SEGGER_RTT_WriteString(0, (const char*)cmd);
					#endif
					ret = true;
				}
			}
			else
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, timestamp);
				SEGGER_RTT_WriteString(0, (const char*)" TCP:  Connecting error!\r\n");
				#endif
				ret = false;
			}
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" TCP:  Server not avaliable!\r\n");
			#endif
			ret = false;
		}
	}	
	else
	{
		#ifdef ESP_DEBUG
		memset(cmd, 0 , 64);
		sprintf(cmd, " WLAN: IP: %s is not avaliable!\r\n", ip);
		SEGGER_RTT_WriteString(0, timestamp);
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
	memset(answer, 0 , BUF_MAX_SIZE);
	if(ESP8266_AtCmd(answer, "AT+CIPCLOSE", "OK", 50))
	{
		if(strstr((const char*)answer, "CLOSED"))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);		
			SEGGER_RTT_WriteString(0, (const char*)" TCP:  Disconnected from server!\r\n");
			#endif
			ret = true;	
		}
		else
		{
			#ifdef ESP_DEBUG	
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" TCP:  Disconnecting error!\r\n");
			#endif
			ret = false;	
		}
	}
	else
	{
		#ifdef ESP_DEBUG	
		SEGGER_RTT_WriteString(0, timestamp);
		SEGGER_RTT_WriteString(0, (const char*)" TCP:  Disconnect, AT error!\r\n");
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
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(cmd, 0 , 64);
	sprintf(cmd, "AT+CIPSEND=%d", strlen(msg));
	if(ESP8266_AtCmd(answer, cmd, ">", 50))
	{
		ESP8266_Serial_Print((uint8_t*)msg);
		if(ESP8266_AtCmd(answer, NULL, "SEND OK", 50))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" TCP:  Data sended succesfuly!\r\n");
			#endif
			ret = true;
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" TCP:  Error while sending data!\r\n");
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
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(cmd, 0 , 64);
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, timestamp);
	SEGGER_RTT_WriteString(0, (const char*)" WLAN: Get status...\r\n");
	#endif
	if(ESP8266_AtCmd(answer, "AT+CIPSTATUS", "OK", 100))
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
		SEGGER_RTT_WriteString(0, timestamp);
		SEGGER_RTT_WriteString(0, (const char*)" WLAN: Unknown status!\r\n");
		#endif
	}
	#ifdef ESP_DEBUG
	sprintf(cmd, " WLAN: Connection status: %d\r\n", ret);
	SEGGER_RTT_WriteString(0, timestamp);
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
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(msg, 0 , 256);
	memset(cmd, 0 , 64);
	// /22dd4423c4a34bf5b989a6342cc691d6/update/V0?value=2.0
	sprintf(msg, "GET %s HTTP/1.1\r\n\r\n", uri);
	sprintf(cmd, "AT+CIPSEND=%d", strlen(msg));
	if(ESP8266_AtCmd(answer, cmd, ">", NULL))
	{
		ESP8266_Serial_Print((uint8_t*)msg);
		if(ESP8266_AtCmd(answer, NULL, "SEND OK", 1000))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)" GET:  Message sended!\r\n");
			#endif
			if(strstr(answer, "+IPD"))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, timestamp);
				SEGGER_RTT_WriteString(0, (const char*)" GET:  Server answer recieved!\r\n");
				#endif
				if(strstr((const char*)answer, "200"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, timestamp);
					SEGGER_RTT_WriteString(0, (const char*)" GET:  200\r\n");
					#endif
					ret = true;
				}
				else if(strstr((const char*)answer, "400"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, timestamp);
					SEGGER_RTT_WriteString(0, (const char*)" GET:  400\r\n");
					#endif
					ret = false;
				}
				else if(strstr((const char*)answer, "500"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, timestamp);
					SEGGER_RTT_WriteString(0, (const char*)" GET:  500\r\n");
					#endif
					ret = false;
				}
				else
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, timestamp);
					SEGGER_RTT_WriteString(0, (const char*)" ");
					SEGGER_RTT_WriteString(0, (const char*)answer);
					SEGGER_RTT_WriteString(0, (const char*)"\r\n");
					#endif
					ret = false;
				}
			}
			else
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, timestamp);
				SEGGER_RTT_WriteString(0, (const char*)" GET:  No answer from server\r\n");
				#endif
				ret = false;
			}
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, timestamp);
			SEGGER_RTT_WriteString(0, (const char*)"GET:  Message send error!\r\n");
			#endif
		}
	}
	return ret;	
}
/**/
void ESP8266_Serial_Config(uint32_t baud)
{
	// Reset pin
	nrf_gpio_cfg_output(29);
	nrf_gpio_pin_clear(29);
	// UART pins
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
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos |
												UART_INTENSET_RXTO_Enabled << UART_INTENSET_RXTO_Pos |
												UART_INTENSET_ERROR_Enabled << UART_INTENSET_ERROR_Pos;
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