/*
*
@file		util.h
*/

#ifndef _UTIL_H_
#define _UTIL_H_

#if defined (NRF51822)
		#include "nrf51.h"
		#include "nrf51_bitfields.h"
#elif defined (NRF52832)
		#include "nrf52.h"
		#include "nrf52_bitfields.h"
#elif defined (STM32)
	#include "stm32f0xx.h"
#endif

#define NO_USE_UTIL_FUNC


extern unsigned char D2C(char c); 					/* Convert HEX(0-F) to a character */
extern char C2D(unsigned char c); 					/* Convert a character to HEX */
extern unsigned long ATOI(char* str, unsigned int base); 			/* Convert a string to integer number */
extern int ValidATOI(char* str, int base, int* ret); 		/* Verify character string and Convert it to (hexa-)decimal. */
extern char * strtok(char * s,const char * ct); 		/* Tokenize a string */
extern void replacetochar(char * str, char oldchar, char newchar); /* Replace old character with new character in the string */

extern unsigned short swaps(unsigned int i);
extern unsigned long swapl(unsigned long l);

#ifndef NO_USE_UTIL_FUNC
extern char* ITOA(unsigned int value, char* str, unsigned int base);		/* Convert Decimal Number to string */
#endif

#endif /* _UTIL_H */

