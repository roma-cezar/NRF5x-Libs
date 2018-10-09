#include "MQTTClient.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"

#include "SEGGER_RTT.h"
#include "esp8266.h"

m_SOCKET 	SOCKET;

bool session_opened = false;
extern bool flag_connected;

extern uint8_t destip[4];
extern uint16_t destport;
extern uint8_t data_buf_rx[255];
extern uint8_t data_buf_tx[255];
extern uint8_t data_buf_len;

bool MQTTClient_Connect(m_MQTT *mqtt)
{   
	if(mqtt->secure)
	{
		SOCKET.type = CON_TYPE_SSL;
	}
	else
	{
		SOCKET.type = CON_TYPE_TCP;
	}
	SOCKET.host = mqtt->host;
	SOCKET.port = mqtt->port;
	SOCKET.rx_timeout_ms = mqtt->timeout;
	if(ESP8266_Session_Open(&SOCKET))
	{
		SEGGER_RTT_WriteString(0, (const char*)"MQTT: Server connected!\r\n");
		return true;
	}
	else
	{
		return false;
	}
}


bool MQTTClient_Disconnect(void) 
{	
	if(session_opened)
	{
		uint8_t packet_224[] = {224, 0};
		SOCKET.payload.tx.len = 2;
		memcpy(SOCKET.payload.tx.data, packet_224, 2);
		if(ESP8266_Session_Send(&SOCKET))
		{
			SEGGER_RTT_WriteString(0, (const char*)"MQTT: Session closed\r\n");
			session_opened = false;
			return true;
		}
	}
	else
	{
		return false;
	}
}

bool MQTTClient_Open_session(m_MQTT *mqtt)
{
	uint16_t timeout = KEEPALIVE;
	uint8_t KTH = (uint8_t)(KEEPALIVE>>8);
	uint8_t KTL = (uint8_t)(KEEPALIVE);
	
	uint16_t id_len = (uint16_t)strlen(mqtt->id);
	uint16_t username_len = (uint16_t)strlen(mqtt->user) ;
	uint16_t paswd_len = (uint16_t)strlen(mqtt->pswd);

	uint8_t header_size = 0;
	
	if(!session_opened)
	{
		/*fixed header: 2 bytes, big endian*/
		uint8_t fixed_header[] = {MQTTCONNECT, 12 + 2 + strlen(mqtt->id) + 2 + strlen(mqtt->user)+ 2 + strlen(mqtt->pswd)};
		// 0x10 0x16+len
		// variable header																						 0x02
		uint8_t var_header[] = {0x00,0x06,'M','Q','I','s','d','p',0x03,0xC2,KTH, KTL};
		uint8_t packet[sizeof(fixed_header) + sizeof(var_header) + 6 + strlen(mqtt->id) + strlen(mqtt->user) + strlen(mqtt->pswd)];

		header_size = sizeof(fixed_header)+sizeof(var_header);
		memset(packet,0,sizeof(packet));
		memcpy(packet, fixed_header, sizeof(fixed_header));
		memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
		// set id
		packet[header_size] = (uint8_t)(id_len>>8);
		packet[header_size+1] = (uint8_t)id_len;
		memcpy(packet+sizeof(fixed_header)+sizeof(var_header) + 2, mqtt->id, strlen(mqtt->id));  // id
		

			// set user name
			packet[header_size + strlen(mqtt->id) + 2] = (uint8_t)(username_len>>8);
			packet[header_size + strlen(mqtt->id) + 3] = (uint8_t)username_len;
			memcpy(packet+sizeof(fixed_header)+sizeof(var_header) + 2 + strlen(mqtt->id) + 2, mqtt->user, strlen(mqtt->user));  // username
			// set password
			packet[header_size + strlen(mqtt->id) + strlen(mqtt->user) + 4] = (uint8_t)(paswd_len>>8);
			packet[header_size + strlen(mqtt->id) + strlen(mqtt->user) + 5] = (uint8_t)paswd_len;
			memcpy(packet+sizeof(fixed_header)+sizeof(var_header) + 2 + strlen(mqtt->id) + 2 + strlen(mqtt->user) + 2, mqtt->pswd, strlen(mqtt->pswd));  // username

		
		// send packet
		SOCKET.payload.tx.len = sizeof(packet)/sizeof(uint8_t);
		memcpy(SOCKET.payload.tx.data, packet, SOCKET.payload.tx.len);
		if(ESP8266_Session_Send(&SOCKET))
		{
				if(ESP8266_Session_ReadBytes(&SOCKET))
				{
					if(SOCKET.payload.rx.data[0] == 0x20 && SOCKET.payload.rx.data[3] == 0x00)
					{
						SEGGER_RTT_WriteString(0, (const char*)"MQTT: Session opened!\r\n");
						session_opened = true;
						return true;
					}
					else
					{
						SEGGER_RTT_WriteString(0, (const char*)"MQTT: Session not opened!\r\n");
						session_opened = false;
						return false;
					}
				}
		}
		else{
			SEGGER_RTT_WriteString(0, (const char*)"MQTT: Session failed!\r\n");
			return false;
		}
	}
	else
	{
		return false;
	}
}

// max message length 255 bytes
bool MQTTClient_Publish(unsigned char* topic, unsigned char* msg) 
{
	session_opened = true;
	uint16_t RemainingLength = 0;
	RemainingLength = 2 + strlen(topic) + strlen(msg);
	if(session_opened && RemainingLength<=MAX_PACKET_SIZE)
	{
		uint8_t fixed_header[] = {MQTTPUBLISH, (uint8_t)RemainingLength};
		uint8_t var_header[2 + strlen(topic)];
		
    var_header[0] = (uint8_t)((strlen(topic))>>8);
		var_header[1] = (uint8_t)(strlen(topic));
		memcpy(&var_header[2], topic, strlen(topic));
		
		uint8_t packet[sizeof(fixed_header) + sizeof(var_header) + strlen(msg)];
		memset(packet, 0, sizeof(packet));
		memcpy((unsigned char *)packet, fixed_header, sizeof(fixed_header));
		memcpy((unsigned char *)packet + sizeof(fixed_header), var_header, sizeof(var_header));
		memcpy((unsigned char *)packet + sizeof(fixed_header) + sizeof(var_header), msg, strlen(msg));

		SOCKET.payload.tx.len = sizeof(packet)/sizeof(uint8_t);
		memcpy(SOCKET.payload.tx.data, packet, SOCKET.payload.tx.len);
    if(ESP8266_Session_Send(&SOCKET))
		{
			SEGGER_RTT_WriteString(0, (const char*)"MQTT: Published!\r\n");
			return true;
		}
		else{
			SEGGER_RTT_WriteString(0, (const char*)"MQTT: Publish failed!\r\n");
			return false;
		}
	}
	else{
		return false;
	}
}

bool MQTTClient_Alive(void) 
{
    if (session_opened) 
    {
        //int t = timer.read_ms();
        //if (t - lastActivity > KEEPALIVE) {
        //    uint8_t packet_192[] = {0xc0, 0x00};
        //    tcpclient_sendbytes(s, packet_192, 2);
        //    lastActivity = t;
        //}
    }
}	
	
	