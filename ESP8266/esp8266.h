#ifndef ESP8266_H
#define ESP8266_H


#include <stdint.h>
#include <stdbool.h>

#define BUF_MAX_SIZE 512

void ESP8266_Serial_Config(uint32_t baud);
void ESP8266_Serial_Send(uint8_t data);
void ESP8266_Serial_Print(uint8_t *data);





#if defined (NRF51822)
	void UART0_IRQHandler(void);
#elif defined (NRF52832)
	void UARTE0_UART0_IRQHandler(void);
#endif

void FlushRx(void);

#endif /* ESP8266_H */