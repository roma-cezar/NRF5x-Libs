/**
 @file		ping.h
 */
 
#ifndef __PING_API__
#define __PING_API__


#if defined (NRF51822)
		#include "nrf51.h"
		#include "nrf51_bitfields.h"
#elif defined (NRF52832)
		#include "nrf52.h"
		#include "nrf52_bitfields.h"
#elif defined (STM32)
	#include "stm32f0xx.h"
#endif

#define PINGBUF_LEN 32


typedef struct _PINGMSG
{
	char	Type; 		/**< 0 - Ping Reply, 8 - Ping Request */
	char 	Code;		/**< Always 0 */
	unsigned short CheckSum;	/**< Check sum */
	unsigned short	ID;		/**< Identification  */ 
	unsigned short SeqNum;		/**< Sequence Number */
	char	Data[PINGBUF_LEN];/**< Ping Data  : 1452 = IP RAW MTU - sizeof(Type+Code+CheckSum+ID+SeqNum) */
}PINGMSG;

typedef struct __PINGLOG
{
	unsigned short CheckSumErr;	/**< Check sum Error Count */
	unsigned short UnreachableMSG;	/**< Count of receiving unreachable message from a peer */
	unsigned short TimeExceedMSG;	/**< Count of receiving time exceeded message from a peer */
	unsigned short UnknownMSG;	/**< Count of receiving unknown message from a peer */
	unsigned short ARPErr;		/**< count of fail to send ARP to the specified peer */
	unsigned short PingRequest;	/**< Count of sending ping-request message to the specified peer */
	unsigned short PingReply;	/**< Count of receiving ping reply message from the specifed peer */
	unsigned short Loss;		/**< Count of timeout  */
}PINGLOG;


char ping(int count, unsigned int size, unsigned int time, unsigned char* addr/*, PINGLOG* log*/); /* Send ping-request to the specified peer and receive ping-reply from the specified peer. */
//void DisplayPingStatistics(PINGLOG log);				/* Display result of ping */
void SendPingReply(uint8_t s, PINGMSG * pingrequest,unsigned int len, unsigned long destaddr);
#endif	// end __PING_API_
