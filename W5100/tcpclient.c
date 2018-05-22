#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "nrf.h"
#include "tcpclient.h"
#include "Serial.h"


extern unsigned char debug_str[64]; // uart buffer
uint8_t data_buf_rx[255];
uint8_t data_buf_tx[255];
uint8_t data_buf_len=0;

uint8_t destip[4];
uint16_t destport;

bool flag_connected = false;
/**/
bool tcpclient_open(SOCKET s, uint8_t protocol, uint8_t* ip, uint16_t port)
{
	process_tcpclient(s);
	if(!flag_connected)
	{
		/* reinitialize the socket */
		if(socket(s, protocol, port, 0x00) == 0)    /* reinitialize the socket */
		{
			Serial_Print((uint8_t*)"\r\nFail to create socket\r\n");
		}
		else  // socket already created
		{
			Serial_Print((uint8_t*)"\r\nConnecting to server...\r\n");
			sprintf((char*)debug_str, "SOCK: %d IP: %d.%d.%d.%d:%d\r\n", s, ip[0], ip[1], ip[2], ip[3], port);
			Serial_Print((uint8_t*)debug_str);
			memset(debug_str, 0, 64);
			if(!connect(s, ip, port))
			{
				Serial_Print((uint8_t*)"\r\nConnecting failed!\r\n");
				return false;
			}
			else
			{
				Serial_Print((uint8_t*)"\r\nSuccessfully connected!\r\n");
				return true;
			}
		}
	}
}
/**/
bool tcpclient_close(SOCKET s)
{
		disconnect(s);
}
/**/
bool tcpclient_send(SOCKET s, uint8_t *data)
{	
	process_tcpclient(s);
	if(flag_connected)
	{
			send(s, data, sizeof(data));
			//memset(data, 0, 4);
			//Serial_Print((uint8_t*)"Data sended!\r\n");
			return true;
	}
	else
	{
		return false;
	}
}
/**/
bool tcpclient_sendbytes(SOCKET s, uint8_t *data, uint8_t size)
{	
	process_tcpclient(s);
	if(flag_connected)
	{
			send(s, data, size);
			//memset(data, 0, size);
			//Serial_Print((uint8_t*)"Data sended!\r\n");
		return true;
	}
	else
	{
		Serial_Print((uint8_t*)"No connected socket!\r\n");
		return false;
	}
}
/**/
bool tcpclient_recivedata(SOCKET s, uint8_t *data, uint16_t len, void (*handler)(uint8_t*, uint8_t))
{
	process_tcpclient(s);
	if(flag_connected)
	{
		if(IINCHIP_READ(Sn_IR(s)) & Sn_IR_RECV)
		{
			memset(data, 0, 255);
			if ((len = getSn_RX_RSR(s)) > 0) 			// check Rx data 
			{
				if (len > 255) len = 255;	// if Rx data size is lager than TX_RX_MAX_BUF_SIZE 
				len = recv(s, data, len);// read the received data 
				//Serial_Print((uint8_t*)"Data received\r\n");
				handler(data, len);
				return true;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}/**/
void process_tcpclient(SOCKET s)
{
	switch (getSn_SR(s))
	{
		case SOCK_ESTABLISHED:						// if connection is established
			//Serial_Print((uint8_t*)"SOCK_ESTABLISHED\r\n");
			flag_connected = true;
			break;
		
		case SOCK_SYNRECV: 
			//Serial_Print((uint8_t*)"SOCK_SYNC\r\n");
			break;
			
		case SOCK_CLOSING: 
			//Serial_Print((uint8_t*)"SOCK_CLOSING\r\n");
			break;
		
		case SOCK_CLOSE_WAIT:                           		/* If the client request to close */
			//Serial_Print((uint8_t*)"SOCK_CLOSE_WAIT\r\n");
			disconnect(s);
			flag_connected = false;
			break;
		
		case SOCK_CLOSED: 			
			//Serial_Print((uint8_t*)"SOCK_CLOSED\r\n");
			flag_connected = false;
			break;
	}
}
/**/
void ArrayToStr(char *output, uint8_t *input, uint8_t len)
{
	for (uint8_t i=0; i<len; i++)
	{
   sprintf((char*)output++, "%d", input[i]);
	}
	//strcpy((char*)output + len, "\0");
}
/**/
void StrToArray(char *output, uint8_t *input)
{
	
}