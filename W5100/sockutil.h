/*
*
@file		sockutil.h
@brief	Implementation of useful function of iinChip
*
*/

#ifndef __SOCKUTIL_H
#define __SOCKUTIL_H

#include "socket.h"

char* inet_ntoa(unsigned long addr);			/* Convert 32bit Address into Dotted Decimal Format */
char* inet_ntoa_pad(unsigned long addr);

unsigned long inet_addr(unsigned char* addr);		/* Converts a string containing an (Ipv4) Internet Protocol decimal dotted address into a 32bit address */

char VerifyIPAddress(char* src);			/* Verify dotted notation IP address string */

unsigned long GetDestAddr(SOCKET s);			/* Output destination IP address of appropriate channel */

unsigned int GetDestPort(SOCKET s);			/* Output destination port number of appropriate channel */

unsigned char CheckDestInLocal(unsigned long destip);			/* Check Destination in local or remote */

SOCKET getSocket(unsigned char status, SOCKET start); 	/* Get handle of socket which status is same to 'status' */

unsigned short checksum(unsigned char * src, unsigned int len);		/* Calculate checksum of a stream */

unsigned long GetIPAddress(void);					/* Get Source IP Address of iinChip. */

unsigned long GetGWAddress(void);					/* Get Source gateway IP Address of iinChip. */

unsigned long GetSubMask(void);					/* Get Source Subnet mask of iinChip. */

void GetMacAddress(unsigned char* mac);		/* Get Mac address of iinChip. */

void GetDestMacAddr(SOCKET s, unsigned char* mac);

void GetNetConfig(void);				/* Read established network information(G/W, IP, S/N, Mac) of iinChip and Output that through Serial.*/

void dump_iinchip(void);					/* dump the 4 channel status of iinChip */

#endif
