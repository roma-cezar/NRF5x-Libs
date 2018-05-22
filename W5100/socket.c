/*
 * socket.c
 *
 * Created: 11.08.2017 21:02:05
 *  Author: Amigo
 */ 
#include "nrf_delay.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "socket.h"
#include "w5100.h"

static uint16_t local_port;

/**
@brief  This Socket function initialize the channel in perticular mode, and set the port and wait for W5100 done it.
@return   1 for sucess else 0.
*/
/**< for socket number */
/**< for socket protocol */
/**< the source port for the socket */
/**< the option for the socket */
uint8_t socket(SOCKET s, uint8_t protocol,   uint16_t port,   uint8_t flag)
{
	uint8_t ret;
	if ((protocol == Sn_MR_TCP) || (protocol == Sn_MR_UDP) || (protocol == Sn_MR_IPRAW) || (protocol == Sn_MR_MACRAW) || (protocol == Sn_MR_PPPOE))
	{
		close(s);
		IINCHIP_WRITE(Sn_MR(s), protocol | flag);
		if (port != 0) {
			IINCHIP_WRITE(Sn_PORT0(s),		(uint8_t)((port & 0xff00) >> 8));
			IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8_t)(port & 0x00ff));
		}
		else
		{
			local_port++; // if don't set the source port, set local_port number.
			IINCHIP_WRITE(Sn_PORT0(s),		(uint8_t)((local_port & 0xff00) >> 8));
			IINCHIP_WRITE((Sn_PORT0(s) + 1),(uint8_t)(local_port & 0x00ff));
		}
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_OPEN); // run sockinit Sn_CR
		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)));
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}
/**
@brief  This function close the socket and parameter is "s" which represent the socket number
*/
void close(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s),Sn_CR_CLOSE);
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) );
	/*  clear interrupt */
	/*  all clear */
	IINCHIP_WRITE(Sn_IR(s), 0xFF);
}
/**
@brief  This function established  the connection for the channel in passive (server) mode. This function waits for the request from the peer.
@return 1 for success else 0.
*/
uint8_t listen(SOCKET s  /**< the socket number */)
{
	uint8_t ret;
	if (IINCHIP_READ(Sn_SR(s)) == SOCK_INIT)
	{
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_LISTEN);
		/* +20071122[chungs]:wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) );
		ret = 1;
	}
	else
	{
		ret = 0;
	}
	return ret;
}
/**
@brief  This function established  the connection for the channel in Active (client) mode.
    This function waits for the untill the connection is established.
@return 1 for success else 0.
*/
uint8_t connect(SOCKET s, uint8_t * addr, uint16_t port)
{
	uint8_t ret;
	if
    (
		((addr[0] == 0xFF) && (addr[1] == 0xFF) && (addr[2] == 0xFF) && (addr[3] == 0xFF)) ||
		((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) ||
		(port == 0x00)
    )
	{
		ret = 0;
	}
	else
	{
		ret = 1;
		// set destination IP
		IINCHIP_WRITE(Sn_DIPR0(s),			addr[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),	addr[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),	addr[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),	addr[3]);
		
		IINCHIP_WRITE(Sn_DPORT0(s),			(uint8_t)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),	(uint8_t)(port & 0x00ff));
		// set/clear subnet for ARP Errata
		clearSUBR();
		IINCHIP_WRITE(Sn_CR(s),	Sn_CR_CONNECT);
		/* wait for completion */
		while ( IINCHIP_READ(Sn_CR(s)) );
		/*
		// check the SYN packet sending...
		while ( IINCHIP_READ(Sn_SR(s)) != SOCK_SYNSENT )
		{
		   if(IINCHIP_READ(Sn_SR(s)) == SOCK_ESTABLISHED)
		   {
			  break;
		   }
		   if (getSn_IR(s) & Sn_IR_TIMEOUT)
		   {
			  setSn_IR(s,(Sn_IR_TIMEOUT ));  // clear TIMEOUT Interrupt
			  ret = 0;
			  break;
		   }
		}*/
		applySUBR();	
	}
	return ret;
}
/**
@brief  This function used for disconnect the socket and parameter is "s" which represent the socket number
@return 1 for success else 0.
*/
void disconnect(SOCKET s)
{
	IINCHIP_WRITE(Sn_CR(s), Sn_CR_DISCON);
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) );
}
/**
@brief  This function used to send the data in TCP mode
@return 1 for success else 0.
*/
uint16_t send(SOCKET s, const uint8_t * buf, uint16_t len)
{
	uint8_t status=0;
	uint16_t ret=0;
	uint16_t freesize=0;
	if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	else ret = len;
	// if freebuf is available, start.
	do
	{
		freesize = getSn_TX_FSR(s);
		status = IINCHIP_READ(Sn_SR(s));
		if ((status != SOCK_ESTABLISHED) && (status != SOCK_CLOSE_WAIT))
		{
			ret = 0;
			break;
		}
	} while (freesize < ret);
	// copy data
	send_data_processing(s, (uint8_t *)buf, ret);
	IINCHIP_WRITE(Sn_CR(s), Sn_CR_SEND);
	/* wait to process the command... */
	while( IINCHIP_READ(Sn_CR(s)) );
	while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
	{
		/* reduce code */
		if ( IINCHIP_READ(Sn_SR(s)) == SOCK_CLOSED )
		{
			close(s);
			return 0;
		}
	}
	IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
	return ret;
}
/**
@brief  This function is an application I/F function which is used to receive the data in TCP mode.
 It continues to wait for data as much as the application wants to receive.
@return received data size for success else -1.
*/
int16_t recv(SOCKET s, uint8_t * buf, uint16_t len)
{
	uint16_t ret=0;
	if ( len > 0 )
	{
		recv_data_processing(s, buf, len);
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);
		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) );
		ret = len;
	}
	return ret;
}
/**
@brief  This function is an application I/F function which is used to send the data for other then TCP mode.
Unlike TCP transmission, The peer's destination address and the port is needed.
@return This function return send data size for success else -1.
*/
/**< socket index */
/**< a pointer to the data */
/**< the data size to send */
/**< the peer's Destination IP address */
/**< the peer's destination port number */
int16_t sendto(SOCKET s, const uint8_t *buf, uint16_t len, uint8_t * addr, uint16_t port)
{
	uint16_t ret=0;
	if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	else ret = len;
	if(((addr[0] == 0x00) && (addr[1] == 0x00) && (addr[2] == 0x00) && (addr[3] == 0x00)) || ((port == 0x00)) ||(ret == 0))
	{
		/*  added return value */
		ret = 0;
	}
	else
	{
		IINCHIP_WRITE(Sn_DIPR0(s),		addr[0]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
		IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
		IINCHIP_WRITE(Sn_DPORT0(s),		(uint8_t)((port & 0xff00) >> 8));
		IINCHIP_WRITE((Sn_DPORT0(s) + 1),(uint8_t)(port & 0x00ff));
		// copy data
		send_data_processing(s, (uint8_t*)buf, ret);
		// set/clear subnet for ARP Errata
		clearSUBR();
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) );
		while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
		{
			if (IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT)
			{
				/*  clear interrupt */
				IINCHIP_WRITE(Sn_IR(s), (Sn_IR_SEND_OK | Sn_IR_TIMEOUT)); /* clear SEND_OK & TIMEOUT */
				return 0;
			}
		}
		IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
		applySUBR();
	}
	return ret;
}
/**
@brief  This function is an application I/F function which is used to receive the data in other then
TCP mode. This function is used to receive UDP, IP_RAW and MAC_RAW mode, and handle the header as well.
@return This function return received data size for success else -1.
*/
/**< the socket number */
/**< a pointer to copy the data to be received */
/**< the data size to read */
/**< a pointer to store the peer's IP address */
/**< a pointer to store the peer's port number. */
int16_t recvfrom(SOCKET s, uint8_t *buf, uint16_t len, uint8_t *addr, uint16_t *port)
{
	uint8_t	 head[8];
	uint16_t data_len=0;
	uint16_t ptr=0;
	if ( len > 0 )
	{
		ptr = IINCHIP_READ(Sn_RX_RD0(s));
		ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);
		switch (IINCHIP_READ(Sn_MR(s)) & 0x07)
		{
			case Sn_MR_UDP :
				read_data(s, (uint8_t *)ptr, head, 0x08);
				ptr += 8;
				// read peer's IP address, port number.
				addr[0] = head[0];
				addr[1] = head[1];
				addr[2] = head[2];
				addr[3] = head[3];
				*port = head[4];
				*port = (*port << 8) + head[5];
				data_len = head[6];
				data_len = (data_len << 8) + head[7];
				read_data(s, (uint8_t *)ptr, buf, data_len); // data copy.
				ptr += data_len;
				IINCHIP_WRITE(Sn_RX_RD0(s),(uint8_t)((ptr & 0xff00) >> 8));
				IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8_t)(ptr & 0x00ff));
			break;
			case Sn_MR_IPRAW :
				read_data(s, (uint8_t *)ptr, head, 0x06);
				ptr += 6;
				addr[0] = head[0];
				addr[1] = head[1];
				addr[2] = head[2];
				addr[3] = head[3];
				data_len = head[4];
				data_len = (data_len << 8) + head[5];
				read_data(s, (uint8_t *)ptr, buf, data_len); // data copy.
				ptr += data_len;
				IINCHIP_WRITE(Sn_RX_RD0(s),(uint8_t)((ptr & 0xff00) >> 8));
				IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8_t)(ptr & 0x00ff));
			break;
			case Sn_MR_MACRAW :
				read_data(s,(uint8_t*)ptr,head,2);
				ptr+=2;
				data_len = head[0];
				data_len = (data_len<<8) + head[1] - 2;

				read_data(s,(uint8_t*) ptr,buf,data_len);
				ptr += data_len;
				IINCHIP_WRITE(Sn_RX_RD0(s),(uint8_t)((ptr & 0xff00) >> 8));
				IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8_t)(ptr & 0x00ff));
			break;
			default :
			break;
		}
		IINCHIP_WRITE(Sn_CR(s), Sn_CR_RECV);
		/* wait to process the command... */
		while( IINCHIP_READ(Sn_CR(s)) );
	}
	return data_len;
}
/**/
uint16_t igmpsend(SOCKET s, const uint8_t * buf, uint16_t len)
{
	uint8_t status=0;
	uint16_t ret=0;
	if (len > getIINCHIP_TxMAX(s)) ret = getIINCHIP_TxMAX(s); // check size not to exceed MAX size.
	else ret = len;
	if  (ret == 0){;}
	else
	{
		// copy data
		send_data_processing(s, (uint8_t *)buf, ret);
		IINCHIP_WRITE(Sn_CR(s),Sn_CR_SEND);
		while( IINCHIP_READ(Sn_CR(s)) );
		while ( (IINCHIP_READ(Sn_IR(s)) & Sn_IR_SEND_OK) != Sn_IR_SEND_OK )
		{
			status = IINCHIP_READ(Sn_SR(s));
			if (IINCHIP_READ(Sn_IR(s)) & Sn_IR_TIMEOUT)
			{
				/* in case of igmp, if send fails, then socket closed */
				/* if you want change, remove this code. */
				close(s);
				return 0;
			}
		}
		IINCHIP_WRITE(Sn_IR(s), Sn_IR_SEND_OK);
	}
	return ret;
}