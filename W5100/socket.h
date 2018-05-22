/*
 * socket.h
 *
 * Created: 11.08.2017 21:02:20
 *  Author: Amigo
 */ 


#ifndef SOCKET_H_
#define SOCKET_H_

#include "nrf.h"

#define	MAX_SOCK_NUM				4

#define __DEF_IINCHIP_MAP_BASE__	0x8000
#define COMMON_BASE					0x0000

#define __DEF_IINCHIP_MAP_TXBUF__	(0x4000) /* Internal Tx buffer address of the chip */
#define __DEF_IINCHIP_MAP_RXBUF__	(0x6000) /* Internal Rx buffer address of the chip */

//#define NULL						((void *) 0)

#define MAX_BUF						256	/* largest buffer we can read from chip */
#define SOCKET_PORT				2000	/* TCP port for HTTP */
#define SOCKET						uint8_t

uint8_t socket(SOCKET s, uint8_t protocol, uint16_t port, uint8_t flag); // Opens a socket(TCP or UDP or IP_RAW mode)
void	close(SOCKET s); // Close socket
uint8_t connect(SOCKET s, uint8_t * addr, uint16_t port); // Establish TCP connection (Active connection)
void	disconnect(SOCKET s); // disconnect the connection
uint8_t listen(SOCKET s);	// Establish TCP connection (Passive connection)
uint16_t send(SOCKET s, const uint8_t * buf, uint16_t len); // Send data (TCP)
int16_t recv(SOCKET s, uint8_t * buf, uint16_t len);	// Receive data (TCP)

int16_t sendto(SOCKET s, const uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t port); // Send data (UDP/IP RAW)
int16_t recvfrom(SOCKET s, uint8_t * buf, uint16_t len, uint8_t * addr, uint16_t  *port); // Receive data (UDP/IP RAW)

uint16_t igmpsend(SOCKET s, const uint8_t * buf, uint16_t len);

#endif /* SOCKET_H_ */