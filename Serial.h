#ifndef Serial_H
#define Serial_H

#include <stdint.h>
#include <stdbool.h>

void Serial_Config(uint32_t baud);
void Serial_Print(const char *data);
void Serial_Send(uint8_t data);
void Serial_Send_Buff(uint8_t *data, uint16_t size);

#if defined (NRF51822)
	void UART0_IRQHandler(void);
#elif defined (NRF52832)
	void UART0_UART0_IRQHandler(void);
#endif

void FlushRx(void);
void getstring(uint8_t* text);
bool CheckCmd(const char* cmd);

#endif /* Serial_H */
