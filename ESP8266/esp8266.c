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



extern uint8_t timestamp[];

static uint8_t rx_buf[BUF_MAX_SIZE];
uint8_t my_ip[16];
uint8_t my_mac[17];

uint16_t bytes_rx=0;
bool rx_data_rdy = false;
bool rx_bytes = false;
bool uart_err_flag = false;
uint8_t ch;

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
		if(NRF_UART0->EVENTS_RXDRDY)
		{
			NRF_UART0->EVENTS_RXDRDY = 0;
			ch = (uint8_t)(NRF_UART0->RXD);
			if(!rx_bytes)
			{
				if(ch == '\n'){
					rx_buf[bytes_rx++] = ch;
					NRF_UART0->TASKS_STOPRX = 1;
					rx_data_rdy = true;
				}
				else if(ch == '>')
				{
					rx_buf[bytes_rx++] = ch;
					NRF_UART0->TASKS_STOPRX = 1;
					rx_data_rdy = true;
				}
				else
				{
					rx_buf[bytes_rx++] = ch; 
				}
			}
			else
			{
				rx_buf[bytes_rx++] = ch; 
				rx_data_rdy = true;
			}
		}
		else if(NRF_UART0->EVENTS_ERROR)
		{
			NRF_UART0->EVENTS_ERROR = 0;
			uart_err_flag = true;
		}
}
/**/
bool ESP8266_AtCmd(uint8_t* databack, const char* cmd, const char* answer, uint32_t fuse_time_ms)
{
	bool ret = false;
	bool data_ok = false;
	bool at_err = false;
	bool at_timeout = false;
	bool at_busy = false;
	
	uint32_t answer_timeout = 0;
	uint32_t rx_timeout = 0;
	char debug_msg[128];
	
	memset(databack, 0, BUF_MAX_SIZE);
	
	bytes_rx = 0;
	if(cmd!=NULL)
	{
		// ���������� �� ��������
		#ifdef ESP_DEBUG_LVL2
		memset(debug_msg, 0, 128);
		sprintf(debug_msg, "\r\nAT >> %s\r\n", (const char*)cmd); 
		SEGGER_RTT_WriteString(0, (const char*)debug_msg);
		#endif
		// ���������� �� ��������
		ESP8266_Serial_Print((uint8_t*)cmd);
		ESP8266_Serial_Print("\r\n");
	}
	
	// ���� ���� �����
	if(answer!=NULL)
	{
		#if defined (NRF51822)	
			NVIC_EnableIRQ(UART0_IRQn);
		#elif defined (NRF52832)
			NVIC_EnableIRQ(UARTE0_UART0_IRQn);
		#endif
		// ��������� ����� ������
		NRF_UART0->TASKS_STARTRX = 1;
		// ���� ������� ������
		if(fuse_time_ms==NULL)
		{
			answer_timeout = 130000000; // 10sec
		}
		else
		{
			answer_timeout =  13000 * fuse_time_ms;
		}

		while(--answer_timeout && !uart_err_flag)
		{
			if(rx_data_rdy)
			{
				// ���� ������� �� ������
				if(strstr((const char*)rx_buf, answer))
				{		
					data_ok = true;
					break;
				}
				else if(strstr((const char*)rx_buf, "ERROR"))
				{
					at_err = true;
					break;
				}
				else if(strstr((const char*)rx_buf, "busy"))
				{
					at_busy = true;
					//break;
				}
				else if(strstr((const char*)rx_buf, "wdt reset"))
				{
					break;
				}
				
				rx_data_rdy = false;
				NRF_UART0->TASKS_STARTRX = 1;
			}
		}

		NRF_UART0->TASKS_STOPRX = 1;
		//NRF_UART0->EVENTS_RXDRDY = 0;
		#if defined (NRF51822)	
			NVIC_DisableIRQ(UART0_IRQn);
		#elif defined (NRF52832)
			NVIC_DisableIRQ(UARTE0_UART0_IRQn);
		#endif
		
		// ���� ����� �������� ������ ���������
		if(answer_timeout==0) 
		{
			#ifdef ESP_DEBUG_LVL2
			SEGGER_RTT_WriteString(0, (const char*)"\r\nAT: timeout!\r\n");
			#endif
			return ret;
		}
		else if(uart_err_flag) // ���� ������ ����
		{
			#ifdef ESP_DEBUG_LVL2
				uint8_t err = (uint8_t)(NRF_UART0->ERRORSRC);
				sprintf(debug_msg, "\r\nAT: UART ERROR: %d\r\n", err);
				SEGGER_RTT_WriteString(0, (const char*)cmd);
			#endif
			uart_err_flag=false;
		}
		else if(at_err) // ���� ������ AT
		{
			#ifdef ESP_DEBUG_LVL2
			SEGGER_RTT_WriteString(0, (const char*)"\r\nAT:   error!\r\n");
			#endif
		}
		else if(at_busy) // ���� AT busy
		{
			#ifdef ESP_DEBUG_LVL2
			SEGGER_RTT_WriteString(0, (const char*)"\r\nAT:   busy!\r\n");
			#endif
		}
		else if(data_ok) // ���� ������� �������
		{
			#ifdef ESP_DEBUG_LVL2
			SEGGER_RTT_WriteString(0, (const char*)"\r\nAT:   success!\r\n");
			#endif
			if(databack!=NULL){
				// �������� ������ � ������ ������
				memcpy(databack, rx_buf, bytes_rx);
			}
			ret = true;
		}
	}

	#ifdef ESP_DEBUG_LVL2
		memset(debug_msg, 0, 128);
		sprintf(debug_msg, "\r\nAT >> %s\r\n", (const char*)rx_buf); 
		SEGGER_RTT_WriteString(0, (const char*)debug_msg);
	#endif
	
	
	memset(rx_buf, 0, BUF_MAX_SIZE);
	return ret;
}
/**/
void ESP8266_Deinit(void)
{
	char debug_msg[128];
	nrf_gpio_pin_clear(28); // power down esp8266
	nrf_gpio_pin_clear(29); // down reset pin
	NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Disabled;
	NRF_UART0->PSELRXD = RX_PIN_NUMBER; 
	NRF_UART0->PSELTXD = TX_PIN_NUMBER;
	memset(debug_msg, 0, 128);
	#ifdef ESP_DEBUG
	sprintf(debug_msg, "\r\n********************************* %s *****************************\r\n", (const char*)timestamp);
	SEGGER_RTT_WriteString(0, (const char*)"WLAN: Deinitialised\r\n");
	SEGGER_RTT_WriteString(0, (const char*)debug_msg);
	#endif
}

bool ESP8266_Preinit(m_WLAN *wlan)
{
	bool ret = false;
	bool esp_ready = false;
	uint8_t tries;
	char debug_msg[128];
	
	ESP8266_Serial_Config(wlan->baudrate);
	nrf_gpio_pin_set(28); //power up esp8266
	#ifdef ESP_DEBUG
	memset(debug_msg, 0, 128);
	sprintf(debug_msg, "\r\n********************************* %s *****************************\r\n", (const char*)timestamp); 
	SEGGER_RTT_WriteString(0, (const char*)debug_msg);
	SEGGER_RTT_WriteString(0, (const char*)"\r\nWLAN: Initialised");
	#endif
	//nrf_delay_ms(1000);
	nrf_gpio_pin_clear(29);
	nrf_delay_ms(10);
	nrf_gpio_pin_set(29);
	//nrf_delay_ms(3000);
	while(!ESP8266_AtCmd(NULL, NULL, "ready", 10000));
	
	tries = 3;
	while(--tries && !esp_ready)
	{
		esp_ready = ESP8266_AtCmd(NULL, "AT", "OK", 100);
		//	nrf_delay_ms(100);
	}
	if(esp_ready)
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: ESP8266 is OK!\r\n");
		#endif
		if(ESP8266_AtCmd(NULL, "ATE0", "OK", 100))
		{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Echo off!\r\n");
		#endif
		}

		if(wlan->wlan_mode==WLAN_MODE_STA)
		{
			if(ESP8266_AtCmd(NULL, "AT+CWMODE=1", "OK", 100))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"WLAN: Mode STA\r\n");
				#endif
			}
		}
		else if(wlan->wlan_mode==WLAN_MODE_AP)
		{
			if(ESP8266_AtCmd(NULL, "AT+CWMODE=2", "OK", 100))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"WLAN: Mode AP\r\n");
				#endif
			}
		}
		
		if(wlan->ip_mode==IP_MODE_STATIC)
		{
			memset(debug_msg, 0, 128);
			sprintf(debug_msg, "AT+CIPSTA=\"%s\"", (const char*)wlan->ip); 
			ESP8266_AtCmd(NULL, debug_msg, "OK", 100);
			
			if(ESP8266_AtCmd(NULL, "AT+CWDHCP=1,0", "OK", 100))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"WLAN: ST IP Static\r\n");
				#endif
			}
		}
		else if(wlan->ip_mode==IP_MODE_DHCP)
		{
			if(ESP8266_AtCmd(NULL, "AT+CWDHCP=1,1", "OK", 100))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"WLAN: STA IP DHCP\r\n");
				#endif
			}
		}
	
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Init complete!\r\n");
		#endif
		ret = true;
	}
	return ret;
}
/**/
bool ESP8266_Wlan_Start(m_WLAN *wlan)
{
	bool ret = false;
	char cmd[128];
	uint8_t tries ;	
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, (const char*)"WLAN: Connecting to AP...");
	#endif
	memset(cmd, 0 , 128);
	sprintf(cmd, "AT+CWJAP=\"%s\",\"%s\"", wlan->ssid, wlan->psk); //"AT+CWJAP=\"dd-wrt\",\"bora-bora04\""
	tries = 4;
	while(!ret && --tries)
	{
		if(ESP8266_AtCmd(NULL, cmd, "OK", 15000)){
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"\r\n");
			SEGGER_RTT_WriteString(0, (const char*)"WLAN: Connected to AP SSID: "); 
			SEGGER_RTT_WriteString(0, (const char*)wlan->ssid);
			SEGGER_RTT_WriteString(0, (const char*)" PSWD: ");
			SEGGER_RTT_WriteString(0, (const char*)wlan->psk);
			SEGGER_RTT_WriteString(0, (const char*)" succesfuly!\r\n");
			SEGGER_RTT_WriteString(0, (const char*)"WLAN: Local  IP: ");
			SEGGER_RTT_WriteString(0, ESP8266_Get_IP());
			SEGGER_RTT_WriteString(0, (const char*)"\r\n");	
			SEGGER_RTT_WriteString(0, (const char*)"WLAN: Local MAC: ");
			SEGGER_RTT_WriteString(0, ESP8266_Get_MAC());
			SEGGER_RTT_WriteString(0, (const char*)"\r\n");	
			#endif
			ret = true;
		}	
	}
	if(!ret)
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Can't connect\r\n");
		#endif
	}
	return ret;
}
/**/
char *ESP8266_Get_IP(void)
{	
	char answer[BUF_MAX_SIZE];
	char ip[16];
	int ip_start_pos = 0 ;
	int ip_end_pos = 0 ;
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(ip, 0 , 16);
	if(ESP8266_AtCmd(answer, "AT+CIFSR", "OK", 100)){
		ip_start_pos = strstr((char*)answer, "+CIFSR:STAIP" ) - answer + 1;
		ip_end_pos = strstr((char*)answer, "+CIFSR:STAMAC" ) - answer + 1;
		strncpy(ip, answer + ip_start_pos + 13, ip_end_pos - ip_start_pos - 17);
		return ip;
	}	
	else
	{
		return NULL;
	}
}/**/
char *ESP8266_Get_MAC(void)
{	
	int mac_start_pos = 0;
	int mac_end_pos = 0;
	char answer[BUF_MAX_SIZE];
	char mac[17];
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(mac, 0 , 17);
	if(ESP8266_AtCmd(answer, "AT+CIFSR", "OK", 100)){
		mac_start_pos = strstr((char*)answer, "+CIFSR:STAMAC" ) - answer;
		mac_end_pos   = strstr((char*)answer, "\"\r\n\r\nOK" ) - answer;
		strncpy(mac, answer + mac_start_pos + 15, mac_end_pos - mac_start_pos - 15);
		return mac;
	}	
	else
	{
		return NULL;
	}
}
/**/
bool ESP8266_Wlan_Stop(void)
{
	bool ret = false;
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, (const char*)"WLAN: Disconnecting...\r\n");
	#endif
	if(ESP8266_AtCmd(NULL, "AT+CWQAP", "OK", 100)){
		ret = true;
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Disconnected successfuly!\r\n");
		#endif
	}	
	else
	{
		ret = false;
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Disconnecting error!\r\n");
		#endif
	}
	return ret;
}
/**/
bool ESP8266_Ping(unsigned char *host, uint8_t tries)
{
	char cmd[64];
	bool ret = false;
	memset(cmd, 0 , 64);
	sprintf(cmd, "AT+PING=\"%s\"", host);
	for(int i=0; i<tries; i++)
	{
		if(ESP8266_AtCmd(NULL, cmd, "OK", 10000))
		{
			#ifdef ESP_DEBUG
			memset(cmd, 0 , 64);
			sprintf(cmd, "TCP:  Ping host %s is OK!\r\n", host);
			SEGGER_RTT_WriteString(0, (const char*)cmd);
			#endif
			ret = true;
		}
		else
		{
			#ifdef ESP_DEBUG
			memset(cmd, 0 , 64);
			sprintf(cmd, "TCP:  Ping host %s is not avaliable!\r\n", host);
			SEGGER_RTT_WriteString(0, (const char*)cmd);
			#endif
		}
	}
	return ret;
}
/**/
bool ESP8266_Session_Open(m_SOCKET *socket)
{
	char answer[BUF_MAX_SIZE];
	char cmd[64];
	bool ret = false;
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(cmd, 0 , 64);
	
	#ifdef ESP_DEBUG
	memset(cmd, 0 , 64);
	sprintf(cmd, "TCP:  Connecting to %s:%d...\r\n", socket->host, socket->port);
	SEGGER_RTT_WriteString(0, (const char*)cmd);
	#endif
	
	if(strstr(socket->type, "SSL"))
	{
		if(ESP8266_AtCmd(NULL, "AT+CIPSSLSIZE=4096", "OK", 100))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"SSL:  Buffer set up!\r\n");
			#endif
		}
	}
			
	memset(cmd, 0 , 64);
	sprintf(cmd, "AT+CIPSTART=\"%s\",\"%s\",%d", socket->type, socket->host, socket->port);
	if(ESP8266_AtCmd(answer, cmd, "OK", 30000))
	{
		if(strstr((const char*)answer, "CONNECT"))
		{	
			if(ESP8266_AtCmd(NULL, "AT+CIPMODE=0", "OK", 100))
			{
				#ifdef ESP_DEBUG
				memset(cmd, 0 , 64);
				sprintf(cmd, "TCP:  Connected succesfuly!\r\n", socket->host, socket->port);
				SEGGER_RTT_WriteString(0, (const char*)cmd);
				#endif
				ret = true;
			}
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  Connecting error!\r\n");
			#endif
			ret = false;
		}
	}
	else
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"TCP:  Server not avaliable!\r\n");
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
	if(ESP8266_AtCmd(answer, "AT+CIPCLOSE", "OK", 100))
	{
		if(strstr((const char*)answer, "CLOSED"))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  Disconnected from server!\r\n");
			#endif
			ret = true;	
		}
		else
		{
			#ifdef ESP_DEBUG	
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  Disconnecting error!\r\n");
			#endif
			ret = false;	
		}
	}
	else
	{
		#ifdef ESP_DEBUG	
		SEGGER_RTT_WriteString(0, (const char*)"TCP:  Disconnect, AT error!\r\n");
		#endif
	}
	return ret;
}
/**/
bool ESP8266_Session_Send(m_SOCKET *socket)
{
	char answer[BUF_MAX_SIZE];
	bool ret = false;	
	char cmd[64];
	memset(cmd, 0 , 64);
	sprintf(cmd, "AT+CIPSEND=%d", socket->payload.tx.len);
	if(ESP8266_AtCmd(NULL, cmd, ">", 100))
	{
		ESP8266_Serial_SendBytes((uint8_t*)socket->payload.tx.data, socket->payload.tx.len);
		if(ESP8266_AtCmd(answer, NULL, "SEND OK", 1000))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  Data sended successfuly!\r\n");
			#endif
			ret = true;
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  Error while sending data!\r\n");
			#endif
		}
	}
	else
	{
		ret = false;
	}
	memset(socket->payload.tx.data, 0, 2048);
	return ret;
}
/**/
bool ESP8266_Session_ReadBytes(m_SOCKET *socket)
{
		bool ret = false;
		uint32_t timeout;
		char debug_msg[128];
		uint16_t rx_len =0;
		char *p;
	
		memset(rx_buf, 0, BUF_MAX_SIZE);
		memset(socket->payload.rx.data, 0 , BUF_MAX_SIZE);
		
		bytes_rx = 0;
		rx_bytes = true;
		#if defined (NRF51822)	
			NVIC_EnableIRQ(UART0_IRQn);
		#elif defined (NRF52832)
			NVIC_EnableIRQ(UARTE0_UART0_IRQn);
		#endif
		// ��������� ����� ������
		NRF_UART0->TASKS_STARTRX = 1;
	
		// ���� ������� ������
		timeout = socket->rx_timeout_ms * 1000; // 1/64 sec
		while(--timeout)
		{
			if(strstr((char*)rx_buf, "+IPD" ))
			{
				break;
			}
			else if(uart_err_flag)
			{
				break;
			}
		}
		nrf_delay_ms(200);
		NRF_UART0->EVENTS_RXDRDY = 0;
		NRF_UART0->TASKS_STOPRX = 1;
		#if defined (NRF51822)	
			NVIC_DisableIRQ(UART0_IRQn);
		#elif defined (NRF52832)
			NVIC_DisableIRQ(UARTE0_UART0_IRQn);
		#endif
		rx_data_rdy = false;
		if(timeout==0)
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  RX data timeout!\r\n");
			#endif
			ret = false;
			return ret;
		}
		
		if(uart_err_flag) // ���� ������ ����
		{
			#ifdef ESP_DEBUG_LVL2
				uint8_t err = (uint8_t)(NRF_UART0->ERRORSRC);
				sprintf(debug_msg, "\r\nAT: UART ERROR: %d\r\n", err);
				SEGGER_RTT_WriteString(0, (const char*)debug_msg);
			#endif
			uart_err_flag=false;
			ret =  false;
		}
		else
		{
		
			int len_start_pos = strstr((char*)rx_buf, "+IPD" ) - (char*)rx_buf + 1;
			int len_end_pos = strstr((char*)rx_buf, ":" ) - (char*)rx_buf + 1;
			if((len_end_pos - len_start_pos)>0)
			{
				strncpy(debug_msg, (char*)rx_buf + len_start_pos - 1, len_end_pos - len_start_pos); // +IPD,0,10
				p = strtok(debug_msg, ",");//+IPD				
				p = strtok(NULL, "\0");//+IPD
				rx_len = atoi(p);
				socket->payload.rx.len = rx_len;
				memcpy(socket->payload.rx.data, rx_buf + len_end_pos, socket->payload.rx.len);
			}
		
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"TCP:  RX data success!\r\n");
			#endif
			ret =  true;
		}
		return ret;

}/**/
bool ESP8266_Session_ReadLine(m_SOCKET *socket)
{
	char answer[BUF_MAX_SIZE];
	char debug_msg[128];

	memset(answer, 0 , BUF_MAX_SIZE);
	memset(debug_msg, 0 , 128);
	#ifdef ESP_DEBUG
	SEGGER_RTT_WriteString(0, (const char*)"TCP:  RX \r\n");
	#endif
	if(ESP8266_AtCmd(answer, NULL, "+IPD", 10000))
	{
		sprintf(debug_msg, "TCP:  RX %s\r\n", answer);
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)debug_msg);
		#endif
	}
	else{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"TCP:  RX timeout!\r\n");
		#endif
	}
		return true;
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
	SEGGER_RTT_WriteString(0, (const char*)"WLAN: Get status...\r\n");
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
		SEGGER_RTT_WriteString(0, (const char*)"WLAN: Unknown status!\r\n");
		#endif
	}
	#ifdef ESP_DEBUG
	sprintf(cmd, " WLAN: Connection status: %d\r\n", ret);
	SEGGER_RTT_WriteString(0, (const char*)cmd);
	#endif
	return ret;
}
/**/
bool ESP8266_NTP_Set(uint8_t time_zone, uint8_t en)
{	
	bool ret = false;	
	char cmd[64];
	memset(cmd, 0 , 64);
	sprintf(cmd, "AT+CIPSNTPCFG=%d,%d,\"%s\",\"%s\",\"%s\"", en, time_zone, "0.ru.pool.ntp.org", "1.ru.pool.ntp.org", "2.ru.pool.ntp.org" );
	if(ESP8266_AtCmd(NULL, cmd, "OK", 1000))
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"NTP:  Enabled!\r\n");
		#endif
		ret = true;
	}
	else
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"NTP:  Error!\r\n");
		#endif
	}
	return ret;
}
/**/
char* ESP8266_NTP_GetTime(void)
{		
	char answer[BUF_MAX_SIZE];
	bool ret = false;	
	char time[32];
	
	memset(answer, 	0, BUF_MAX_SIZE);
	memset(time, 		0, 32);
	
	if(ESP8266_AtCmd(answer, "AT+CIPSNTPTIME?", "OK", 1000))
	{
		#ifdef ESP_DEBUG
		SEGGER_RTT_WriteString(0, (const char*)"NTP:  Time: ");
		SEGGER_RTT_WriteString(0, (const char*)answer);
		SEGGER_RTT_WriteString(0, (const char*)"\r\n");
		#endif
		return time;
	}	
	else
	{
		return NULL;
	}
}
/**/
bool ESP8266_GET_Request(const char* uri, m_SOCKET *socket)
{
	char answer[BUF_MAX_SIZE];
	bool ret = false;	
	char msg[256];
	char cmd[64];
	memset(answer, 0 , BUF_MAX_SIZE);
	memset(msg, 0 , 256);
	memset(cmd, 0 , 64);
	// /22dd4423c4a34bf5b989a6342cc691d6/update/V0?value=2.0
	sprintf(cmd, "AT+CIPSEND=%d", strlen(uri));
	if(ESP8266_AtCmd(NULL, cmd, ">", 100))
	{
		ESP8266_Serial_Print((uint8_t*)uri);
		if(ESP8266_AtCmd(answer, NULL, "SEND OK", 1000))
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"GET:  Message sended!\r\n");
			#endif
			if(ESP8266_Session_ReadBytes(socket))
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"GET:  Server answer recieved!\r\n");
				#endif
				if(strstr((const char*)socket->payload.rx.data, "200"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, (const char*)"GET:  200\r\n");
					#endif
					ret = true;
				}
				else if(strstr((const char*)socket->payload.rx.data, "400"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, (const char*)"GET:  400\r\n");
					#endif
					ret = false;
				}
				else if(strstr((const char*)socket->payload.rx.data, "500"))
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, (const char*)"GET:  500\r\n");
					#endif
					ret = false;
				}
				else
				{
					#ifdef ESP_DEBUG
					SEGGER_RTT_WriteString(0, (const char*)"GET:  ");
					SEGGER_RTT_WriteString(0, (const char*)socket->payload.rx.data);
					SEGGER_RTT_WriteString(0, (const char*)"\r\n");
					#endif
					ret = false;
				}
			}
			else
			{
				#ifdef ESP_DEBUG
				SEGGER_RTT_WriteString(0, (const char*)"GET:  No answer from server\r\n");
				#endif
				ret = false;
			}
		}
		else
		{
			#ifdef ESP_DEBUG
			SEGGER_RTT_WriteString(0, (const char*)"GET:  Message send error!\r\n");
			#endif
		}
	}
	return ret;	
}
/**/
void ESP8266_Serial_Config(uint32_t baud)
{
	// Power pin
	nrf_gpio_cfg_output(28);
	nrf_gpio_pin_clear(28);
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
	NRF_UART0->EVENTS_TXDRDY = 0;
	NRF_UART0->EVENTS_RXDRDY = 0;
	NRF_UART0->INTENCLR = 0xFFFFFFFF;
	NRF_UART0->INTENSET = UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos |
												UART_INTENSET_ERROR_Enabled << UART_INTENSET_ERROR_Pos;
NRF_UART0->ENABLE = UART_ENABLE_ENABLE_Enabled;
	
#if defined (NRF51822)	
  NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_SetPriority(UART0_IRQn, 3);
//	NVIC_EnableIRQ(UART0_IRQn);
#elif defined (NRF52832)
  NVIC_ClearPendingIRQ(UARTE0_UART0_IRQn);
	NVIC_SetPriority(UARTE0_UART0_IRQn, 3);
//	NVIC_EnableIRQ(UARTE0_UART0_IRQn);
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
		ESP8266_Serial_Send(data[i++]);
	}
}

void ESP8266_Serial_SendBytes(uint8_t *data, uint16_t len)
{
	uint16_t i = 0;
	for(i=0; i<len; i++)
	{
		ESP8266_Serial_Send(data[i]);
	}
}


void FlushRx(void)
{
	memset(rx_buf, 0, BUF_MAX_SIZE);
	rx_data_rdy = false;
	bytes_rx = 0;
}