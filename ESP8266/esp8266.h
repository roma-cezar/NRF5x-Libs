#ifndef ESP8266_H
#define ESP8266_H


#include <stdint.h>
#include <stdbool.h>

#define BUF_MAX_SIZE 2048

#define WLAN_MODE_STA 			0
#define WLAN_MODE_AP  			1
#define WLAN_MODE_AP_STA  	2
#define	IP_MODE_STATIC  		0
#define	IP_MODE_DHCP  			1

#define CON_TYPE_TCP 	"TCP"
#define CON_TYPE_UDP	"UDP"
#define CON_TYPE_SSL	"SSL"

#define HTTP_TYPE_GET		"GET"
#define HTTP_TYPE_POST	"POST"

#define HTTP_200		0
#define HTTP_400		1
#define HTTP_401		2
#define HTTP_404		3
#define HTTP_500		4


#define NTP_URI "GET /0.ru.pool.ntp.org HTTP/1.1\r\n\r\n"
typedef struct
{
	uint32_t 		baudrate;
	uint8_t 		ip[16];
	uint8_t 		mac[17];
	uint8_t*		ssid;		
	uint8_t*		psk;	
	uint8_t			wlan_mode;
	uint8_t			ip_mode;
	uint8_t 		status;
	uint8_t 		tx_power;
} m_WLAN;

typedef struct
{
	uint8_t 				*type;
	unsigned char* 	host;
	uint16_t 				port;
	uint32_t 				rx_timeout_ms;
	struct{
		struct{
			uint8_t 		data[BUF_MAX_SIZE];
			uint16_t 		len;
		}rx;
		struct{
			uint8_t 		data[BUF_MAX_SIZE];
			uint16_t 		len;
		}tx;
	}payload;
} m_SOCKET;

typedef struct
{
	unsigned char *type;
	unsigned char *header;
	unsigned char *content;
	uint16_t *ret_code;
}m_HTTP;

bool ESP8266_AtCmd(uint8_t* databack, const char* cmd, const char* answer, uint32_t fuse_time_ms);

void ESP8266_Serial_Config(uint32_t baud);
void ESP8266_Serial_Send(uint8_t data);
void ESP8266_Serial_Print(uint8_t *data);
void ESP8266_Serial_SendBytes(uint8_t *data, uint16_t len);

bool ESP8266_Preinit(m_WLAN *wlan);
void ESP8266_Deinit(void);

bool ESP8266_Wlan_Start(m_WLAN *wlan);
bool ESP8266_Wlan_Stop(void);
char *ESP8266_Get_IP(void);
char *ESP8266_Get_MAC(void);
bool ESP8266_Ping(unsigned char *host, uint8_t tries);

bool ESP8266_Session_Open(m_SOCKET *socket);
bool ESP8266_Session_Send(m_SOCKET *socket);
bool ESP8266_Session_ReadLine(m_SOCKET *socket);
bool ESP8266_Session_ReadBytes(m_SOCKET *socket);
bool ESP8266_Session_Close(void);
uint8_t ESP8266_Session_Status(void);

bool ESP8266_GET_Request(const char* uri, m_SOCKET *socket);






bool ESP8266_NTP_Set(uint8_t time_zone, uint8_t en);
char* ESP8266_NTP_GetTime(void);


#if defined (NRF51822)
	void UART0_IRQHandler(void);
#elif defined (NRF52832)
	void UARTE0_UART0_IRQHandler(void);
#endif

void FlushRx(void);

#endif /* ESP8266_H */