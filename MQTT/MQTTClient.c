#include "MQTTClient.h"


#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "nrf.h"
#include "nrf_delay.h"

#include "tcpclient.h"
#include "Serial.h"

bool session_opened = false;
extern bool flag_connected;

extern uint8_t destip[4];
extern uint16_t destport;
extern uint8_t data_buf_rx[255];
extern uint8_t data_buf_tx[255];
extern uint8_t data_buf_len;

bool MQTTClient_Connect(SOCKET s, uint8_t* ip, uint16_t port)
{
	if(tcpclient_open(s, ip, port))
	{
		Serial_Print((uint8_t*)"MQTT server connected!\r\n");
		return true;
	}
	else
	{
		return false;
	}
}


bool MQTTClient_Disconnect(SOCKET s) 
{	
	if(session_opened)
	{
    uint8_t packet_224[] = {224, 0};
    tcpclient_sendbytes(s, (uint8_t*)packet_224, 2);
		Serial_Print((uint8_t*)"MQTT session closed\r\n");
		session_opened = false;
		return true;
	}
	else
	{
		return false;
	}
}

bool MQTTClient_Open_session(SOCKET s, unsigned char* id, unsigned char* user, unsigned char* pswd)
{
	uint16_t timeout = KEEPALIVE;
	uint8_t KTH = (uint8_t)(KEEPALIVE>>8);
	uint8_t KTL = (uint8_t)(KEEPALIVE);
	
	uint16_t id_len = (uint16_t)strlen(id);
	uint16_t username_len = (uint16_t)strlen(user) ;
	uint16_t paswd_len = (uint16_t)strlen(pswd);
	
	uint8_t header_size = 0;
	
	if(!session_opened)
	{
		/*fixed header: 2 bytes, big endian*/
		uint8_t fixed_header[] = {MQTTCONNECT, 12 + 2 + strlen(id) + 2 + strlen(user)+ 2 + strlen(pswd)};
		// 0x10 0x16+len
		// variable header																						 0x02
		uint8_t var_header[] = {0x00,0x06,'M','Q','I','s','d','p',0x03,0xC2,KTH, KTL};
		uint8_t packet[sizeof(fixed_header) + sizeof(var_header) + 6 + strlen(id) + strlen(user) + strlen(pswd)];

		header_size = sizeof(fixed_header)+sizeof(var_header);
		memset(packet,0,sizeof(packet));
		memcpy(packet, fixed_header, sizeof(fixed_header));
		memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
		// set id
		packet[header_size] = (uint8_t)(id_len>>8);
		packet[header_size+1] = (uint8_t)id_len;
		memcpy(packet+sizeof(fixed_header)+sizeof(var_header) + 2, id, strlen(id));  // id
		// set user name
		packet[header_size + strlen(id) + 2] = (uint8_t)(username_len>>8);
		packet[header_size + strlen(id) + 3] = (uint8_t)username_len;
		memcpy(packet+sizeof(fixed_header)+sizeof(var_header) + 2 + strlen(id) + 2, user, strlen(user));  // username
		// set password
		packet[header_size + strlen(id) + strlen(user) + 4] = (uint8_t)(paswd_len>>8);
		packet[header_size + strlen(id) + strlen(user) + 5] = (uint8_t)paswd_len;
		
		memcpy(packet+sizeof(fixed_header)+sizeof(var_header) + 2 + strlen(id) + 2 + strlen(user) + 2, pswd, strlen(pswd));  // username

		// send packet
		if(tcpclient_sendbytes(s, packet, sizeof(packet)/sizeof(uint8_t)))
		{
				while(!tcpclient_recivedata(s, data_buf_rx, data_buf_len));
				if(data_buf_rx[0] == 0x20 && data_buf_rx[3] == 0x00)
				{
					Serial_Print((uint8_t*)"MQTT session opened!\r\n");
					session_opened = true;
					return true;
				}
				else
				{
					Serial_Print((uint8_t*)"MQTT session not opened!\r\n");
					session_opened = false;
					return false;
				}
		}
		else{
			Serial_Print((uint8_t*)"MQTT session failed!\r\n");
			return false;
		}
	}
	else
	{
		return false;
	}
}

// max message length 255 bytes
bool MQTTClient_Publish(SOCKET s, unsigned char* topic, unsigned char* msg) 
{
	session_opened = true; //!!!!!!!!!!!!!!!!!!!!!!!!!!
	uint16_t RemainingLength = 0;
	RemainingLength = 2 + strlen(topic) + strlen(msg);
	if(session_opened && RemainingLength<=MAX_PACKET_SIZE)
	{
		uint8_t fixed_header[] = {MQTTPUBLISH, (uint8_t)RemainingLength};// 0x30 0x00 0x09
		uint8_t var_header[2 + strlen(topic)]; // 0x00 0x03 n r f
		
    var_header[0] = (uint8_t)((strlen(topic))>>8);
		var_header[1] = (uint8_t)(strlen(topic));
		
		memcpy(&var_header[2], topic, strlen(topic));
		
		uint8_t packet[sizeof(fixed_header) + sizeof(var_header) + strlen(msg)];
		
		memset(packet, 0, sizeof(packet));
		memcpy((unsigned char *)packet, fixed_header, sizeof(fixed_header));
		memcpy((unsigned char *)packet + sizeof(fixed_header), var_header, sizeof(var_header));
		memcpy((unsigned char *)packet + sizeof(fixed_header) + sizeof(var_header), msg, strlen(msg));

    if(tcpclient_sendbytes(s, packet, sizeof(packet)/sizeof(uint8_t)))
		{
			Serial_Print((uint8_t*)"MQTT publish!\r\n");
			return true;
		}
		else{
			Serial_Print((uint8_t*)"Failed MQTT publish!\r\n");
			return false;
		}
	}
	else{
		return false;
	}
}

bool MQTTClient_Alive(SOCKET s) 
{
    if (session_opened) 
    {
        //int t = timer.read_ms();
        //if (t - lastActivity > KEEPALIVE) {
            uint8_t packet_192[] = {0xc0, 0x00};
            tcpclient_sendbytes(s, packet_192, 2);
        //    lastActivity = t;
        //}
    }
}	
	
	