#if defined (NRF51822)
		#include "nrf51.h"
		#include "nrf51_bitfields.h"
#elif defined (NRF52832)
		#include "nrf52.h"
		#include "nrf52_bitfields.h"
#endif
#include "nrf.h"
#include "Serial.h"
#include "custom_utils.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char  *p;

/**/
void GetIP(uint8_t *text, uint8_t *ip)
{
	uint8_t data[32];
	memset(data, 0, 32);
	Serial_Print((uint8_t*)text);
	getstring(data);
	Serial_Print((uint8_t*)strtok ((char*)data, "\r\n"));
	p = strtok((char*)data, ".");
	// Выделение последующих частей
	while (p != NULL)
	{
		// Вывод очередной выделенной части
		*ip++ = (uint8_t)atoi(p);
		// Выделение очередной части строки
		p = strtok (NULL, ".");
	}
}
/**/
void GetSN(uint8_t *text, uint8_t *sn)
{
	uint8_t data[32];
	memset(data, 0, 32);
	Serial_Print((uint8_t*)text);
	getstring(data);
	Serial_Print((uint8_t*)strtok ((char*)data, "\r\n"));
	p = strtok((char*)data, ".");
	// Выделение последующих частей
	while (p != NULL)
	{
		// Вывод очередной выделенной части
		*sn++ = (uint8_t)atoi(p);
		// Выделение очередной части строки
		p = strtok (NULL, ".");
	}
}
/**/
void GetGW(uint8_t *text, uint8_t *gw)
{
	uint8_t data[32];
	memset(data, 0, 32);
	Serial_Print((uint8_t*)text);
	getstring(data);
	Serial_Print((uint8_t*)strtok ((char*)data, "\r\n"));
	p = strtok((char*)data, ".");
	// Выделение последующих частей
	while (p != NULL)
	{
		// Вывод очередной выделенной части
		*gw++ = (uint8_t)atoi(p);
		// Выделение очередной части строки
		p = strtok (NULL, ".");
	}
}
/**/
void GetHOST(uint8_t *text, uint8_t *hst)
{
	uint8_t data[32];
	memset(data, 0, 32);
	Serial_Print((uint8_t*)text);
	getstring(data);
	Serial_Print((uint8_t*)strtok ((char*)data, "\r\n"));
	p = strtok((char*)data, ".");
	// Выделение последующих частей
	while (p != NULL)
	{
		// Вывод очередной выделенной части
		*hst++ = (uint8_t)atoi(p);
		// Выделение очередной части строки
		p = strtok (NULL, ".");
	}
}
/**/
void GetPORT(uint8_t *text, uint16_t *rp)
{
	uint8_t data[32];
	uint16_t port;
	memset(data, 0, 32);
	Serial_Print((uint8_t*)text);
	getstring(data);
	p = strtok((char*)data, "\r\n");
	Serial_Print((uint8_t*)p);
	*rp=(uint16_t)(atoi(p));
	//return port;
}