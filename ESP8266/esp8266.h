#ifndef ESP8266_H
#define ESP8266_H


#include <stdint.h>
#include <stdbool.h>

#define BUF_MAX_SIZE 512

bool ESP8266_AtCmd(uint8_t* databack, const char* cmd, const char* answer, uint16_t fuse_time_ms);

void ESP8266_Serial_Config(uint32_t baud);
void ESP8266_Serial_Send(uint8_t data);
void ESP8266_Serial_Print(uint8_t *data);

bool ESP8266_Preinit(void);

bool ESP8266_Wlan_Start(const char* SSID, const char* PASS);
bool ESP8266_Wlan_Stop(void);
char *ESP8266_Get_IP(void);


bool ESP8266_Session_Open(const char* type, const char* ip, uint16_t port);
bool ESP8266_Session_Send(const char* msg);
bool ESP8266_Session_Close(void);
uint8_t ESP8266_Session_Status(void);
bool ESP8266_GET_Req(const char* uri);

#if defined (NRF51822)
	void UART0_IRQHandler(void);
#elif defined (NRF52832)
	void UARTE0_UART0_IRQHandler(void);
#endif

void FlushRx(void);

#endif /* ESP8266_H */