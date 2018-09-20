#ifndef eSERIAL_H
#define eSERIAL_H


#include <stdint.h>
#include <stdbool.h>

#define MAX_USART_BUFFER 256


void eSERIAL_Init(uint32_t baud);
void eSERIAL_Transmit(uint8_t* data, uint16_t len);

uint16_t eSERIAL_Recieve(uint8_t* data);


#if defined (NRF51822)
	void UART0_IRQHandler(void);
#elif defined (NRF52832)
	void UARTE0_UART0_IRQHandler(void);
#endif


#endif /* eSERIAL_H */