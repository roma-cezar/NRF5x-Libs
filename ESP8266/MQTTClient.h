#ifndef MQTTClient_H
#define MQTTClient_H


#include <stdint.h>
#include <stdbool.h>

#define MQTT_PORT 1883

#define MQTTCONNECT     1 << 4  // Client request to connect to Server
#define MQTTCONNACK     2 << 4  // Connect Acknowledgment
#define MQTTPUBLISH     3 << 4  // Publish message
#define MQTTPUBACK      4 << 4  // Publish Acknowledgment
#define MQTTPUBREC      5 << 4  // Publish Received (assured delivery part 1)
#define MQTTPUBREL      6 << 4  // Publish Release (assured delivery part 2)
#define MQTTPUBCOMP     7 << 4  // Publish Complete (assured delivery part 3)
#define MQTTSUBSCRIBE   8 << 4  // Client Subscribe request
#define MQTTSUBACK      9 << 4  // Subscribe Acknowledgment
#define MQTTUNSUBSCRIBE 10 << 4 // Client Unsubscribe request
#define MQTTUNSUBACK    11 << 4 // Unsubscribe Acknowledgment
#define MQTTPINGREQ     12 << 4 // PING Request
#define MQTTPINGRESP    13 << 4 // PING Response
#define MQTTDISCONNECT  14 << 4 // Client is Disconnecting
#define MQTTReserved    15 << 4 // Reserved

#define MAX_PACKET_SIZE 255
#define KEEPALIVE 			10 // seconds

typedef struct
{
	unsigned char* 	host;
	uint16_t 		    port;
	unsigned char*	user;		
	unsigned char*	pswd;
	unsigned char*  id;	
	unsigned char*  topic;	
	uint16_t 		    timeout;
	bool						secure;
} m_MQTT;

bool MQTTClient_Connect(m_MQTT *mqtt);
bool MQTTClient_Open_session(m_MQTT *mqtt);
bool MQTTClient_Publish(unsigned char* pub_topic, unsigned char* msg);
bool MQTTClient_Disconnect(void);
bool MQTTClient_Alive(void);












#endif /* MQTTClient_H */