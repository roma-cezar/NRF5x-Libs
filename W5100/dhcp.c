/*
 * dhcp.c
 *
 *     Created: 3/5/2014
 *      Author: Nabeel Ahmad (nbl14@hotmail.com)
 * Description: Definition of a function that implements an incomplete version
 *              of the DHCP client state-machine.
 */ 

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "socket.h"
#include "w5100.h"
#include "dhcp.h"
#include "Serial.h"
#include "Delay.h"

#define HOST_NAME	"W5100"		    /**< Host Name */

uint8_t runDHCP(uint8_t s)
{
	uint8_t status = 0;
	char debug_str[64] = "";
	uint8_t SRC_MAC_ADDR[] = {0x8A, 0x26, 0xF4, 0x1A, 0xD0, 0x83};				/**< Local MAC address */
	uint8_t GET_SN_MASK[] = {0, 0, 0, 0};			/**< Subnet mask received from the DHCP server */
	uint8_t GET_GW_IP[] = {0, 0, 0, 0};				/**< Gateway ip address received from the DHCP server */
	uint8_t GET_SIP[] = {0, 0, 0, 0};				/**< Local ip address received from the DHCP server */
	uint8_t DHCP_SIP[] = {0, 0, 0, 0};     	/**< DHCP server ip address is discovered */

	uint32_t DHCP_XID = 1234567890;
	RIP_MSG* pRIPMSG;													/**< Pointer for the DHCP message */
	RIP_MSG RIPMSG;                         	/**< Structure definition */

	uint16_t recv_msg_size;
	uint8_t *recv_msg_end;
	uint8_t *current_option;
	uint8_t  option_length;

	uint8_t ip[4];
	uint16_t i=0;
	uint16_t port=0;

	iinchip_init();
	Delay_ms(1000);
	setSIPR(GET_SIP);
	setSHAR(SRC_MAC_ADDR);
	sysinit(1, 1);
	Delay_ms(1000);

	// Set up socket
	if(socket(s, Sn_MR_UDP , DHCP_CLIENT_PORT, 0)) //Sn_MR_UDP
	{
		Serial_print("DHCP sock is opened!\r\n");
	}
		else
	{
		Serial_print("DHCP sock is not opened!\r\n");
	}
    
	/************************************************************************/
	/*                           Send DHCPDISCOVER                          */
	/************************************************************************/
	
	pRIPMSG = &RIPMSG;
	
	memset((void*)pRIPMSG, 0, sizeof(RIP_MSG));     // Set whole of RIPMSG to 0

	pRIPMSG->op = DHCP_BOOTREQUEST;
	pRIPMSG->htype = DHCP_HTYPE10MB;
	pRIPMSG->hlen = DHCP_HLENETHERNET;
	pRIPMSG->hops = DHCP_HOPS;
	pRIPMSG->xid = DHCP_XID;
	pRIPMSG->secs = DHCP_SECS;
	pRIPMSG->flags = DHCP_FLAGSBROADCAST;
	pRIPMSG->chaddr[0] = SRC_MAC_ADDR[0];
	pRIPMSG->chaddr[1] = SRC_MAC_ADDR[1];
	pRIPMSG->chaddr[2] = SRC_MAC_ADDR[2];
	pRIPMSG->chaddr[3] = SRC_MAC_ADDR[3];
	pRIPMSG->chaddr[4] = SRC_MAC_ADDR[4];
	pRIPMSG->chaddr[5] = SRC_MAC_ADDR[5];

	/* MAGIC_COOKIE */
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 24)& 0xFF);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 16)& 0xFF);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 8)& 0xFF);
	pRIPMSG->OPT[i++] = (char)(MAGIC_COOKIE& 0xFF);

	/* Option Request Param. */
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = DHCP_DISCOVER;

	// Client identifier
	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	pRIPMSG->OPT[i++] = 0x07;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[0];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[1];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[2];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[3];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[4];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[5];
	
	// host name
	pRIPMSG->OPT[i++] = hostName;
	pRIPMSG->OPT[i++] = strlen(HOST_NAME)+3; // length of hostname + 3
	strcpy((char*)&(pRIPMSG->OPT[i]),HOST_NAME);
	
	i+=strlen(HOST_NAME);
	
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[3];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[4];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[5];

	// IP Address Lease Time request
	pRIPMSG->OPT[i++] = dhcpIPaddrLeaseTime;
	pRIPMSG->OPT[i++] = 4;
	pRIPMSG->OPT[i++] = 0xFF;
	pRIPMSG->OPT[i++] = 0xFF;
	pRIPMSG->OPT[i++] = 0xFF;
	pRIPMSG->OPT[i++] = 0xFF;
    
    // Parameter Request List
	pRIPMSG->OPT[i++] = dhcpParamRequest;
	pRIPMSG->OPT[i++] = 0x02;
	pRIPMSG->OPT[i++] = subnetMask;
	pRIPMSG->OPT[i++] = routersOnSubnet;
	pRIPMSG->OPT[i++] = endOption;

	/* send broadcasting packet */
	ip[0] = 255;
	ip[1] = 255;
	ip[2] = 255;
	ip[3] = 255;
	
	Serial_print("DHCP discover...\r\n");
	if(!sendto(s, (uint8_t *)pRIPMSG, sizeof(RIP_MSG), ip, DHCP_SERVER_PORT))
	{
		Serial_print("DHCP discover: Fatal Error!\r\n");
	}
	sprintf(debug_str, "\r\nDHCP --> IP: %d.%d.%d.%d port: %d, XID: %ld\r\n", ip[0], ip[1], ip[2], ip[3], DHCP_SERVER_PORT, RIPMSG.xid);
	Serial_print(debug_str);
	sprintf(debug_str, "DHCP --> MAC: %2X:%2X:%2X:%2X:%2X:%2X\r\n",SRC_MAC_ADDR[0],SRC_MAC_ADDR[1], SRC_MAC_ADDR[2], SRC_MAC_ADDR[3], SRC_MAC_ADDR[4], SRC_MAC_ADDR[5]);
	Serial_print(debug_str);
	/************************************************************************/
	/*                           Receive DHCPOFFER                          */
	/************************************************************************/
	while (1)
	{
			if (IINCHIP_READ(Sn_IR(0)) & Sn_IR_RECV)
			{
					IINCHIP_WRITE(Sn_IR(0), Sn_IR_RECV);    // Clearing flag
					recv_msg_size = recvfrom(s, (uint8_t *)pRIPMSG, sizeof(RIPMSG), ip, &port);
					sprintf(debug_str, "\r\nDHCP <-- IP: %d.%d.%d.%d port: %d, XID: %ld\r\n", ip[0], ip[1], ip[2], ip[3], port,  RIPMSG.xid);
					Serial_print(debug_str);
					sprintf(debug_str, "DHCP <-- MAC: %2X:%2X:%2X:%2X:%2X:%2X\r\n", RIPMSG.chaddr[0], RIPMSG.chaddr[1], RIPMSG.chaddr[2], RIPMSG.chaddr[3], RIPMSG.chaddr[4], RIPMSG.chaddr[5]); 
					Serial_print(debug_str);
					if (port==DHCP_SERVER_PORT  && !memcmp(RIPMSG.chaddr, SRC_MAC_ADDR, 6) && RIPMSG.xid == DHCP_XID)
					{
							// Check options
							Serial_print("\r\nDHCP checking DHCPOFFER options...\r\n");
							recv_msg_end = (uint8_t *)((uint16_t)&(RIPMSG.op) + recv_msg_size);
							current_option = (uint8_t *)((uint16_t)&(RIPMSG.op) + 240);
							while(current_option < recv_msg_end)
							{
									switch (*(current_option++))
									{	
											case padOption:
													break;
											
											case endOption:
													break;
											
											case dhcpMessageType:
													current_option++;
													if ((*current_option++) != DHCP_OFFER)
													{
															return 0;
													}
													else break;
													
											case subnetMask:
													option_length = *current_option++;
													memcpy(GET_SN_MASK, current_option, 4);
													current_option += option_length;
													break;
													
											case routersOnSubnet:
													option_length = *current_option++;
													memcpy(GET_GW_IP, current_option, 4);
													current_option += option_length;
													break;
													
											case dhcpServerIdentifier:
													current_option++;
													memcpy(DHCP_SIP, current_option, 4);
													current_option += 4;
													break;
													
											default:
													option_length = *current_option++;
													current_option += option_length;
													break;
									}
							}
							memcpy(GET_SIP, (RIPMSG.yiaddr), 4);
							break;
					}
					else
					{
							Serial_print("\r\nDHCP checking options error...\r\n");
							return 0;
					}
			}
	}
	/************************************************************************/
	/*                            Send DHCPREQUEST                          */
	/************************************************************************/
	i=0;
	memset((void*)pRIPMSG, 0, sizeof(RIP_MSG));
	pRIPMSG->op = DHCP_BOOTREQUEST;
	pRIPMSG->htype = DHCP_HTYPE10MB;
	pRIPMSG->hlen = DHCP_HLENETHERNET;
	pRIPMSG->hops = DHCP_HOPS;
	pRIPMSG->xid = DHCP_XID;
	pRIPMSG->secs = DHCP_SECS;
	pRIPMSG->flags = DHCP_FLAGSBROADCAST;
	//pRIPMSG->flags = 0;
	pRIPMSG->chaddr[0] = SRC_MAC_ADDR[0];
	pRIPMSG->chaddr[1] = SRC_MAC_ADDR[1];
	pRIPMSG->chaddr[2] = SRC_MAC_ADDR[2];
	pRIPMSG->chaddr[3] = SRC_MAC_ADDR[3];
	pRIPMSG->chaddr[4] = SRC_MAC_ADDR[4];
	pRIPMSG->chaddr[5] = SRC_MAC_ADDR[5];

	/* MAGIC_COOKIE */
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 24)& 0xFF);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 16)& 0xFF);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 8)& 0xFF);
	pRIPMSG->OPT[i++] = (char)(MAGIC_COOKIE& 0xFF);

	/* Option Request Param. */
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = DHCP_REQUEST;

	// Client identifier
	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	pRIPMSG->OPT[i++] = 0x07;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[0];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[1];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[2];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[3];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[4];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[5];
	
	// Request IP Address
	pRIPMSG->OPT[i++] = dhcpRequestedIPaddr;
	pRIPMSG->OPT[i++] = 0x04;
	pRIPMSG->OPT[i++] = GET_SIP[0];
	pRIPMSG->OPT[i++] = GET_SIP[1];
	pRIPMSG->OPT[i++] = GET_SIP[2];
	pRIPMSG->OPT[i++] = GET_SIP[3];
	
	// Server Identifier
	pRIPMSG->OPT[i++] = dhcpServerIdentifier;
	pRIPMSG->OPT[i++] = 0x04;
	pRIPMSG->OPT[i++] = DHCP_SIP[0];
	pRIPMSG->OPT[i++] = DHCP_SIP[1];
	pRIPMSG->OPT[i++] = DHCP_SIP[2];
	pRIPMSG->OPT[i++] = DHCP_SIP[3];
	
	// host name
	pRIPMSG->OPT[i++] = hostName;
	pRIPMSG->OPT[i++] = strlen(HOST_NAME)+3; // length of hostname + 3
	strcpy((char*)&(pRIPMSG->OPT[i]),HOST_NAME);
	i+=strlen(HOST_NAME);
	
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[3];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[4];
	pRIPMSG->OPT[i++] = SRC_MAC_ADDR[5];
	
	// Parameter Request List
	pRIPMSG->OPT[i++] = dhcpParamRequest;
	pRIPMSG->OPT[i++] = 0x02;
	pRIPMSG->OPT[i++] = subnetMask;
	pRIPMSG->OPT[i++] = routersOnSubnet;
	pRIPMSG->OPT[i++] = endOption;

	/* send broadcasting packet */
	ip[0] = 255;
	ip[1] = 255;
	ip[2] = 255;
	ip[3] = 255;
		
	Serial_print("DHCP request...\r\n");
	if(!sendto(s, (uint8_t *)pRIPMSG, sizeof(RIP_MSG), ip, DHCP_SERVER_PORT))
	{
		Serial_print("DHCP request: Fatal Error!\r\n");
	}	
	sprintf(debug_str, "\r\nDHCP --> IP: %d.%d.%d.%d port: %d, XID: %ld\r\n", ip[0], ip[1], ip[2], ip[3], DHCP_SERVER_PORT, RIPMSG.xid);
	Serial_print(debug_str);
	sprintf(debug_str, "DHCP --> MAC: %2X:%2X:%2X:%2X:%2X:%2X\r\n",SRC_MAC_ADDR[0],SRC_MAC_ADDR[1], SRC_MAC_ADDR[2], SRC_MAC_ADDR[3], SRC_MAC_ADDR[4], SRC_MAC_ADDR[5]);
	Serial_print(debug_str);
	
	/************************************************************************/
	/*                           Receive DHCPACK                            */
	/************************************************************************/
	
	while (1)
	{
			if (IINCHIP_READ(Sn_IR(0)) & Sn_IR_RECV)
			{		
					IINCHIP_WRITE(Sn_IR(0), Sn_IR_RECV);    // Clearing flag
					recv_msg_size = recvfrom(s, (uint8_t*)pRIPMSG, sizeof(RIPMSG), ip, &port);
					
					sprintf(debug_str, "\r\nDHCP <-- IP: %d.%d.%d.%d port: %d, XID: %ld\r\n", ip[0], ip[1], ip[2], ip[3], port,  RIPMSG.xid);
					Serial_print(debug_str);
					sprintf(debug_str, "DHCP <-- MAC: %2X:%2X:%2X:%2X:%2X:%2X\r\n", RIPMSG.chaddr[0], RIPMSG.chaddr[1], RIPMSG.chaddr[2], RIPMSG.chaddr[3], RIPMSG.chaddr[4], RIPMSG.chaddr[5]); 
					Serial_print(debug_str);
					
					if (port==DHCP_SERVER_PORT  && !memcmp(RIPMSG.chaddr, SRC_MAC_ADDR, 6) && RIPMSG.xid == DHCP_XID)
					{
							Serial_print("\r\nDHCP checking DHCPACK options!\r\n");
							// Check options
							recv_msg_end = (uint8_t *)((uint16_t)&(RIPMSG.op) + recv_msg_size);
							current_option = (uint8_t *)((uint16_t)&(RIPMSG.op) + 240);
							while(current_option < recv_msg_end)
							{
									switch (*(current_option++))
									{
											case padOption:
												break;
											
											case endOption:
												break;
											
											case dhcpMessageType:
												current_option++;
												if ((*current_option++) != DHCP_ACK)
												{
														return 0;
												}
												else break;
											
											case subnetMask:
												option_length = *current_option++;
												memcpy(GET_SN_MASK, current_option, 4);
												current_option += option_length;
												break;
											
											case routersOnSubnet:
												option_length = *current_option++;
												memcpy(GET_GW_IP, current_option, 4);
												current_option += option_length;
												break;
											
											default:
												option_length = *current_option++;
												current_option += option_length;
												break;
									}
							}
							memcpy(GET_SIP, (RIPMSG.yiaddr), 4);
							memcpy(DHCP_SIP, (RIPMSG.siaddr), 4);
							break;
					}
					else
					{
							Serial_print("DHCP request error!\r\n");
							return 0;
					}
			}
	}	
	// Close socket
	close(s);

	iinchip_init();
	Delay_ms(1000);
	setSIPR(GET_SIP);
	setSUBR(GET_SN_MASK);
	setGAR(ip);
	setSHAR(SRC_MAC_ADDR);
	sysinit(1, 1);
	Delay_ms(1000);

	return(1);
}