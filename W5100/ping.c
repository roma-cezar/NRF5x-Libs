/**
 @file		ping.c
 @brief 		Implemation of Ping

To send 'ping-request' to peer & To receive 'ping-reply' from peer.
 */
#include "Serial.h"
#if defined (NRF51)
	#include "nrf_delay.h"
#elif defined (NRF52)	
	#include "nrf_delay.h"
#else
	#include "Delay.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "util.h"			// Useful function
#include "sockutil.h"			// Util of iinChip
#include "socket.h"
#include "w5100.h"
#include "ping.h"                       // header file


/**
 @brief 	Send ping-request to the specified peer and receive ping-reply from the specified peer.
 @return	1 - success, 0 - fail because free socket is not found or can't be opened.
 */
 /**< Ping reqeust count.  If count is -1 then to send ping request to the specified peer infinitely. */
 /**< Data size to be sent, The range is greater than 0 and less than PINGBUF_LEN */
 /**< wait ping reply time (unit : ms) */
 /**< Peer Ip Address(32bit network odering address) */
 /**< result of ping */
 
char ping(int count, unsigned int size, unsigned int time, unsigned char* addr/*, PINGLOG* log*/)
{
	char str[64];
	PINGMSG  PingRequest;		// Variable for Ping Request
	PINGMSG  PingReply;		// Variable for Ping Reply		
	unsigned long peerip;                  // 32 bit Peer IP Address
	unsigned long tempip;			// ip address received from a destination
	uint8_t  addrstr[16];        	// Decimal dotted-notation IP address string
	uint16_t  port;               	// port number received from a destination
	int i;
	int len;
	SOCKET s = 0;		// socket variable for pinging
	char bLoop = 0;                 // Infinite = 1, finite = 0
	char IsReceived = 0;            // Received packet = 1, not received packet = 0
	unsigned int  RemainTime=0;       	// Remained Time for waiting to ping-reply
	
	/* Initialize PingRequest */
	static unsigned int  RandomID = 0x1234;			// Ping-Request ID
	static unsigned int  RandomSeqNum = 0x4321;			// Ping-Request packet's sequence number
	PingRequest.Type = 8;           			// Ping-Request 
	PingRequest.Code = 0;					// always 0
	PingRequest.CheckSum = 0;				// value of checksum before calucating checksum of ping-request packet
	PingRequest.ID = RandomID++;			// set ping-request's ID to random integer value
	PingRequest.SeqNum = RandomSeqNum;		// set ping-request's sequence number to ramdom integer value
	for(i = 0 ; i < PINGBUF_LEN; i++){	
		PingRequest.Data[i] = 'a' + i % 23;		// fill 'a'~'w' characters into ping-request's data 
	}
	/* Initialize result of pinging */
	//memset((void*)log,0,sizeof(PINGLOG));

  /* Verify arguments */	
	if(count == -1) {
		bLoop = 1;				// if it is infinite 
	}
	else if(!count) {
		count = 4;				// set count to defalut value
	}
	if(size < 1 || size > PINGBUF_LEN) size = PINGBUF_LEN;  // set size to default value
	
	if(!time) time = 1000;					// set time to default value

	/* Create a ping socket */
	s = getSocket(SOCK_CLOSED, 0);
	if(s == MAX_SOCK_NUM) return 0;				// if it isn't exist free socket, Error
	
	setSn_PROTO(s,IPPROTO_ICMP);                         	// Set upper-protocol of IP proctol
	
	peerip = inet_addr(addr);			// convert address's string into 32bit address
	
	uint8_t ip_hex[] = {192, 168, 1, 12};
	if(!tcpclient_open(s, Sn_MR_IPRAW, ip_hex, 3000) == 0) 
	{
		//Serial_Print((uint8_t*)"Failed to open sock at port 3000\r\n");
		// 0;	// Open IP_RAW Mode , if fail then Error
	}
	

	sprintf(str, "\r\nPinging %s with %d bytes of data :\r\n", addr, size);
	Serial_Print(str);

	Serial_Print((uint8_t*)"Start ping loop\r\n");
	/* Ping Service */
	while(bLoop || count-- > 0)				// if Infinite or count > 0, Loop 
	{
		IsReceived = 0;
		RemainTime = time/2;
		PingRequest.SeqNum = RandomSeqNum++;                           // Increase Sequence number for next ping-request packet
		PingRequest.CheckSum = 0;
		PingRequest.CheckSum = checksum((unsigned char*)&PingRequest,size+8);	// update checksum field

		if(send(s, (const unsigned char *)&PingRequest,size+8)==-1)	// Send Ping-Request to the specified peer. If fail, then it is occurred ARP Error.
		{
			close(s);                               // close the pinging socket
			/* Reopen pinging socket */
			Serial_Print((uint8_t*)"Socket reopen\r\n");
			setSn_PROTO(s, IPPROTO_ICMP);          
			if(socket(s, Sn_MR_IPRAW, 3000, 0)==0) return 0;
			continue;
		}
		while((RemainTime--) > 0)	// until wait_time is remaining 
		{
			nrf_delay_ms(10);
			if(IINCHIP_READ(Sn_IR(s)) & Sn_IR_RECV)
			{
				if((len = getSn_RX_RSR(s)) > 0)		// Is pinging socket  received a packet?
				{
					len = recvfrom(s,(unsigned char*)&PingReply,len, (unsigned char*)&tempip, &port);	// receive a packet from unknown peer
					strcpy((char*)addrstr,inet_ntoa(tempip));                            // convert 32 bit unknown peer IP address into string of IP Address.
					sprintf(str, "Reply from %s\r\n", addrstr);
					Serial_Print(str);
					if(checksum((unsigned char*)&PingReply,len) != 0)			// if the packet's checksum value is correct
					{                                                               // not correct                                  // checksum error
						if(tempip == peerip) IsReceived = 1;       
						Serial_Print(" : Checksum Error.\r\n"); 
					}
					else if(PingReply.Type == 0)					// if the received packet is ping-reply 
					{
						// verify id,sequence number, and ip address					  		
						if((PingReply.ID!=PingRequest.ID) || (PingReply.SeqNum!=PingRequest.SeqNum) || (tempip!=peerip)) 
						{							
							Serial_Print(" : Unknown peer.\r\n");			// fail to verify 
						}
						else                                                    // success
						{
							IsReceived = 1;
							sprintf(str, " : bytes=%d, time<=%dms\r\n",len-8,(time/2-RemainTime)*2);
							Serial_Print(str);
						}
					}
					else if( PingReply.Type == 3)  					// If the packet is unreachable message
					{                                                               
						IsReceived = 1;
						Serial_Print(" : Destination Unreachable.\r\n");
					}
					else if( PingReply.Type == 11)                 			// If the packet is time exceeded message
					{
							IsReceived = 1;
						Serial_Print(" : TTL expired in transit.\r\n");
					}
					else if( PingReply.Type == 8)					// Send ping reply to a peer 
					{
						Serial_Print(" : Ping Request Message.\r\n");
						SendPingReply(s, &PingReply,len,tempip);
					}				
					else                                                            // if the packet is unknown message
					{
						sprintf(str, " : Unknown Message. (type = 0x%02X)\r\n", PingReply.Type);
						Serial_Print(str);
					}
				}
				else if(getSn_SR(s)==SOCK_CLOSED) 				// if it is occurred to fail to send arp packet
				{
					Serial_Print("Ping sock closed\r\n");
					close(s);							// close the pinging socket
					setSn_PROTO(s, IPPROTO_ICMP);                                  // Reopen the pinging socket
					if(socket(s,Sn_MR_IPRAW,3000,0)==0) return 0;
					break;
				}
			}
//			if(RemainTime == 0 && IsReceived == 0)					// If it is not received packet from the specified peer during waiting ping-reply packet.
//			{
//				Serial_Print("Request timed out.\r\n");
//			}
		}
	}
	/* Release pinging socket */
	setSn_PROTO(s, 0);
	close(s);
	return 1;
}


/**
 @brief 	Display result of ping 
 */
/*void DisplayPingStatistics(PINGLOG log)
{
	uint8_t str[64];
	Serial_Print("Ping statistics :");
		sprintf(str, "\tPackets: Sent = %d, Received = %d, Lost = %d\r\n", log.PingRequest,log.PingReply+log.CheckSumErr+log.UnknownMSG+log.UnreachableMSG+log.TimeExceedMSG,log.Loss+log.ARPErr);
    	Serial_Print(str);
	if(log.CheckSumErr > 0)
		sprintf(str, "\t\tChecksum Error Packets = %d\r\n",log.CheckSumErr);
		Serial_Print(str);
	if(log.UnreachableMSG > 0)
		sprintf(str, "\t\tUnreachable Message Packets = %d\r\n",log.UnreachableMSG);
		Serial_Print(str);
	if(log.TimeExceedMSG > 0)
		sprintf(str, "\t\tTime Exceeded Message Packets = %d\r\n",log.TimeExceedMSG);
		Serial_Print(str);
	if(log.UnknownMSG > 0)
		sprintf(str, "\t\tUnknown Message Packets = %d\r\n",log.UnknownMSG);
		Serial_Print(str);
	if(log.ARPErr > 0)
		sprintf(str, "\t\tFail To Send ARP Packets = %d\r\n",log.ARPErr);
		Serial_Print(str);
	if(log.Loss > 0)
		sprintf(str, "\t\tRequest timed out = %d\r\n",log.Loss);
		Serial_Print(str);
	if(log.PingReply > 0)
		sprintf(str, "\t\tPing Reply Packets = %d\r\n",log.PingReply);
		Serial_Print(str);
}*/


/**
 @brief 	Send ping reply packet to the specified peer
 If Error is occurred in sending pingreply packet, then ignored\n
 Because it is not need to send ping-reply packet to the specified peer everytime.
 */
 /**< socket handle of receiving ICMP Message */
 /**< received pingreply packet from the specified peer */
 /**< size of received pingrequest packet */
 /**< 32bit peer ip address (network ordering) */
void SendPingReply(SOCKET s, PINGMSG * pingrequest, unsigned int len, unsigned long destaddr)
{
	uint8_t str[64];
	SOCKET PingReplySocket;
	unsigned int size = len;
    if(size > PINGBUF_LEN) size = PINGBUF_LEN;   					// Set size to MSS size(1460)
	PingReplySocket = getSocket(SOCK_CLOSED,0);			// Find free socket number
	if(PingReplySocket != MAX_SOCK_NUM )        // Is it exist free socket?
	{
		setSn_PROTO(PingReplySocket,IPPROTO_ICMP); 	      		// Set upper protocol of IP_RAW mode
		if(socket(PingReplySocket,Sn_MR_IPRAW,3001,0) != 0)	// Open ICMP socket
		{
			(*pingrequest).Type = 0;				// Ping-Reply
			(*pingrequest).Code = 0;                                // Always 0
  			(*pingrequest).CheckSum = 0;                            
			(*pingrequest).CheckSum = checksum((unsigned char*)pingrequest,size);	// Calculate checksum
			if(sendto(PingReplySocket,(unsigned char*)pingrequest,size,(unsigned char*)&destaddr,3001)==-1)  // sent ping-reply packet to the specified peer
			{
				sprintf(str, "Fail to send ping-reply packet to %s\r\n",inet_ntoa(destaddr));
				Serial_Print(str);
				close(PingReplySocket);				// Close ICMP socket
			}
		}
		setSn_PROTO(PingReplySocket,0);		// Clear IP protocol register.
	}
	else{
			sprintf(str, "Fail to send ping-reply packet to %s",inet_ntoa(destaddr));
			Serial_Print(str);
	}
}
