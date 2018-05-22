/*
 *  w5100.c      library of target-independent AVR support routines
 *               for the Wiznet W5100 Ethernet interface device
 */


#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "w5100.h"
#include "socket.h"

#include "nrf_gpio.h"  
#include "NRF5x_SPI.h"

#ifndef  FALSE
	#define  FALSE          0
	#define  TRUE           !FALSE
#endif
/**/
uint8_t 	I_STATUS[MAX_SOCK_NUM];
uint16_t 	SMASK[MAX_SOCK_NUM];           /**< Variable for Tx buffer MASK in each channel */
uint16_t 	RMASK[MAX_SOCK_NUM];           /**< Variable for Rx buffer MASK in each channel */
uint16_t 	SSIZE[MAX_SOCK_NUM];           /**< Max Tx buffer size by each channel */
uint16_t 	RSIZE[MAX_SOCK_NUM];           /**< Max Rx buffer size by each channel */
uint16_t 	SBUFBASEADDRESS[MAX_SOCK_NUM]; /**< Tx buffer base address by each channel */
uint16_t 	RBUFBASEADDRESS[MAX_SOCK_NUM]; /**< Rx buffer base address by each channel */
uint8_t 	SUBN_VAR[] = {0, 0, 0, 0};// = {255, 255, 255, 0};
/**/

uint16_t getIINCHIP_RxMAX(uint8_t s)
{
   return RSIZE[s];
}
uint16_t getIINCHIP_TxMAX(uint8_t s)
{
   return SSIZE[s];
}
uint16_t getIINCHIP_RxMASK(uint8_t s)
{
   return RMASK[s];
}

uint16_t getIINCHIP_TxMASK(uint8_t s)
{
   return SMASK[s];
}
uint16_t getIINCHIP_RxBASE(uint8_t s)
{
   return RBUFBASEADDRESS[s];
}
uint16_t getIINCHIP_TxBASE(uint8_t s)
{
   return SBUFBASEADDRESS[s];
}
 /**
@brief	This function writes the data into W5100 registers.
*/
uint8_t IINCHIP_WRITE(uint16_t addr,uint8_t data)
{
	CS_SELECT;
	SPI_RW(W5100_WRITE_OPCODE);
	SPI_RW((addr & 0xFF00) >> 8);
	SPI_RW(addr & 0x00FF);
	SPI_RW(data);
	CS_DESELECT;     
	return 1;
}
/**
@brief	This function reads the value from W5100 registers.
*/
uint8_t IINCHIP_READ(uint16_t addr)
{
	uint8_t data;
	CS_SELECT;
	SPI_RW(W5100_READ_OPCODE);
	SPI_RW((addr & 0xFF00) >> 8);
	SPI_RW(addr & 0x00FF);
	data = SPI_RW(0);
	CS_DESELECT;
	return data;
}
/**
@brief	This function writes into W5100 memory(Buffer)
*/ 
uint16_t wiz_write_buf(uint16_t addr,uint8_t* buf,uint16_t len)
{
	uint16_t idx = 0;	
	for(idx=0;idx<len;idx++)
	{
		CS_SELECT;
		SPI_RW(W5100_WRITE_OPCODE);
		SPI_RW(((addr+idx) & 0xFF00) >> 8);
		SPI_RW((addr+idx) & 0x00FF);
		SPI_RW(buf[idx]);
		CS_DESELECT;
	}	   
	return len;
}
/**
@brief	This function reads into W5100 memory(Buffer)
*/ 
uint16_t wiz_read_buf(uint16_t addr, uint8_t* buf,uint16_t len)
{
	uint16_t idx = 0;
	for (idx=0; idx<len; idx++)
  {
		CS_SELECT;
		SPI_RW(W5100_READ_OPCODE);
		SPI_RW(((addr+idx) & 0xFF00) >> 8);
		SPI_RW((addr+idx) & 0x00FF);
		buf[idx] = SPI_RW(0);
		CS_DESELECT;   
	}
	return len;
}
/**
@brief	This function is for resetting of the iinchip. Initializes the iinchip to work in whether DIRECT or INDIRECT mode
*/ 
void iinchip_init(void)
{	
	setMR(MR_RST);
	//clear all w5100 interrupts
	for(uint8_t i=0; i<4; i++){
		IINCHIP_WRITE(Sn_IR(i), 0xFF);
	}
	IINCHIP_WRITE(IR, 0xFF);
}
/**
@brief	This function set the transmit & receive buffer size as per the channels is used
Note for TMSR and RMSR bits are as follows\n
bit 1-0 : memory size of channel #0 \n
bit 3-2 : memory size of channel #1 \n
bit 5-4 : memory size of channel #2 \n
bit 7-6 : memory size of channel #3 \n\n
Maximum memory size for Tx, Rx in the W5100 is 8K Bytes,\n
In the range of 8KBytes, the memory size could be allocated dynamically by each channel.\n
Be attentive to sum of memory size shouldn't exceed 8Kbytes\n
and to data transmission and receiption from non-allocated channel may cause some problems.\n
If the 8KBytes memory is already  assigned to centain channel, \n
other 3 channels couldn't be used, for there's no available memory.\n
If two 4KBytes memory are assigned to two each channels, \n
other 2 channels couldn't be used, for there's no available memory.\n
*/ 
/**< tx_size Tx memory size (00 - 1KByte, 01- 2KBtye, 10 - 4KByte, 11 - 8KByte) */
/**< rx_size Rx memory size (00 - 1KByte, 01- 2KBtye, 10 - 4KByte, 11 - 8KByte) */
void sysinit(uint8_t tx_size, uint8_t rx_size)
{
	int16_t i;
	int16_t ssum;
	int16_t rsum;
	ssum = 0;
	rsum = 0;	
	IINCHIP_WRITE(TMSR,tx_size); /* Set Tx memory size for each channel */
	IINCHIP_WRITE(RMSR,rx_size);	 /* Set Rx memory size for each channel */
	SBUFBASEADDRESS[0] = (uint16_t)(__DEF_IINCHIP_MAP_TXBUF__);		/* Set base address of Tx memory for channel #0 */
	RBUFBASEADDRESS[0] = (uint16_t)(__DEF_IINCHIP_MAP_RXBUF__);		/* Set base address of Rx memory for channel #0 */
    for (i = 0 ; i < MAX_SOCK_NUM; i++)       // Set the size, masking and base address of Tx & Rx memory by each channel
	{
		SSIZE[i] = (int16_t)(0);
		RSIZE[i] = (int16_t)(0);
		if (ssum < 8192)
		{
         switch((tx_size >> i*2) & 0x03)  // Set Tx memory size
			{
			case 0:
				SSIZE[i] = (int16_t)(1024);
				SMASK[i] = (uint16_t)(0x03FF);
				break;
			case 1:
				SSIZE[i] = (int16_t)(2048);
				SMASK[i] = (uint16_t)(0x07FF);
				break;
			case 2:
				SSIZE[i] = (int16_t)(4096);
				SMASK[i] = (uint16_t)(0x0FFF);
				break;
			case 3:
				SSIZE[i] = (int16_t)(8192);
				SMASK[i] = (uint16_t)(0x1FFF);
				break;
			}
		}
		if (rsum < 8192)
		{
         switch((rx_size >> i*2) & 0x03)     // Set Rx memory size
			{
			case 0:
				RSIZE[i] = (int16_t)(1024);
				RMASK[i] = (uint16_t)(0x03FF);
				break;
			case 1:
				RSIZE[i] = (int16_t)(2048);
				RMASK[i] = (uint16_t)(0x07FF);
				break;
			case 2:
				RSIZE[i] = (int16_t)(4096);
				RMASK[i] = (uint16_t)(0x0FFF);
				break;
			case 3:
				RSIZE[i] = (int16_t)(8192);
				RMASK[i] = (uint16_t)(0x1FFF);
				break;
			}
		}
		ssum += SSIZE[i];
		rsum += RSIZE[i];
		if (i != 0)// Sets base address of Tx and Rx memory for channel #1,#2,#3
		{
			SBUFBASEADDRESS[i] = SBUFBASEADDRESS[i-1] + SSIZE[i-1];
			RBUFBASEADDRESS[i] = RBUFBASEADDRESS[i-1] + RSIZE[i-1];
		}
	}
}
/**/
void setMR(uint8_t val)
{
	/* 	DIRECT ACCESS	*/
	IINCHIP_WRITE(MR,val);
}
/**
@brief	This function sets up gateway IP address.
*/ 
/**< a pointer to a 4 -byte array responsible to set the Gateway IP address. */
void setGAR(uint8_t * addr)
{
	IINCHIP_WRITE((GAR0 + 0),addr[0]);
	IINCHIP_WRITE((GAR0 + 1),addr[1]);
	IINCHIP_WRITE((GAR0 + 2),addr[2]);
	IINCHIP_WRITE((GAR0 + 3),addr[3]);
}
/**/
void getGWIP(uint8_t * addr)
{
	addr[0] = IINCHIP_READ((GAR0 + 0));
	addr[1] = IINCHIP_READ((GAR0 + 1));
	addr[2] = IINCHIP_READ((GAR0 + 2));
	addr[3] = IINCHIP_READ((GAR0 + 3));
}
/**
@brief	It sets up SubnetMask address
*/ 
/**< a pointer to a 4 -byte array responsible to set the SubnetMask address */
void setSUBR(uint8_t * addr)
{
	SUBN_VAR[0] = addr[0];
	SUBN_VAR[1] = addr[1];
	SUBN_VAR[2] = addr[2];
	SUBN_VAR[3] = addr[3];
	applySUBR();
}
/**
@brief	It sets up SubnetMask address
*/ 
/**< a pointer to a 4 -byte array responsible to set the SubnetMask address */
void applySUBR(void)
{
	IINCHIP_WRITE((SUBR0 + 0), SUBN_VAR[0]);
	IINCHIP_WRITE((SUBR0 + 1), SUBN_VAR[1]);
	IINCHIP_WRITE((SUBR0 + 2), SUBN_VAR[2]);
	IINCHIP_WRITE((SUBR0 + 3), SUBN_VAR[3]);
}
/**
@brief	It sets up SubnetMask address
*/ 
/**< a pointer to a 4 -byte array responsible to set the SubnetMask address */
void clearSUBR(void)
{
	uint8_t IP_VAR[] = {0, 0, 0, 0};
	getSIPR(IP_VAR);
	if((IP_VAR[0]==0 && IP_VAR[1]==0 && IP_VAR[2]==0 && IP_VAR[3]==0))
	{
		IINCHIP_WRITE((SUBR0 + 0), 0);
		IINCHIP_WRITE((SUBR0 + 1), 0);
		IINCHIP_WRITE((SUBR0 + 2), 0);
		IINCHIP_WRITE((SUBR0 + 3), 0);
	}
}
/**
@brief	This function sets up MAC address.
*/
/**< a pointer to a 6 -byte array responsible to set the MAC address. */
void setSHAR(uint8_t * addr)
{
	IINCHIP_WRITE((SHAR0 + 0),addr[0]);
	IINCHIP_WRITE((SHAR0 + 1),addr[1]);
	IINCHIP_WRITE((SHAR0 + 2),addr[2]);
	IINCHIP_WRITE((SHAR0 + 3),addr[3]);
	IINCHIP_WRITE((SHAR0 + 4),addr[4]);
	IINCHIP_WRITE((SHAR0 + 5),addr[5]);
}
/**
@brief	This function sets up Source IP address.
*/
/**< a pointer to a 4 -byte array responsible to set the Source IP address. */
void setSIPR(uint8_t * addr)
{
	IINCHIP_WRITE((SIPR0 + 0),addr[0]);
	IINCHIP_WRITE((SIPR0 + 1),addr[1]);
	IINCHIP_WRITE((SIPR0 + 2),addr[2]);
	IINCHIP_WRITE((SIPR0 + 3),addr[3]);
}
/**
@brief	This function gets Interrupt register in common register.
 */
uint8_t getIR( void )
{
   return IINCHIP_READ(IR);
}
/**
@brief	This function sets up Retransmission time.
If there is no response from the peer or delay in response then retransmission 
will be there as per RTR (Retry Time-value Register)setting
*/
void setRTR(uint16_t timeout)
{
	IINCHIP_WRITE(RTR0,(uint8_t)((timeout & 0xff00) >> 8));
	IINCHIP_WRITE((RTR0 + 1),(uint8_t)(timeout & 0x00ff));
}
/**
@brief	This function set the number of Retransmission.

If there is no response from the peer or delay in response then recorded time 
as per RTR & RCR register seeting then time out will occur.
*/
void setRCR(uint8_t retry)
{
	IINCHIP_WRITE(RCR,retry);
}
/**
@brief	This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)
If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void setIMR(uint8_t mask)
{
	IINCHIP_WRITE(IMR,mask); // must be setted 0x10.
}
/**
@brief	These below functions are used to get the Gateway, SubnetMask
and Source Hardware Address (MAC Address) and Source IP address
*/
void getGAR(uint8_t * addr)
{
	addr[0] = IINCHIP_READ(GAR0);
	addr[1] = IINCHIP_READ(GAR0+1);
	addr[2] = IINCHIP_READ(GAR0+2);
	addr[3] = IINCHIP_READ(GAR0+3);
}
/**/
void getSUBR(uint8_t * addr)
{
	addr[0] = IINCHIP_READ(SUBR0);
	addr[1] = IINCHIP_READ(SUBR0+1);
	addr[2] = IINCHIP_READ(SUBR0+2);
	addr[3] = IINCHIP_READ(SUBR0+3);
}
/**/
void getSHAR(uint8_t * addr)
{
	addr[0] = IINCHIP_READ(SHAR0);
	addr[1] = IINCHIP_READ(SHAR0+1);
	addr[2] = IINCHIP_READ(SHAR0+2);
	addr[3] = IINCHIP_READ(SHAR0+3);
	addr[4] = IINCHIP_READ(SHAR0+4);
	addr[5] = IINCHIP_READ(SHAR0+5);
}
/**/
void getSIPR(uint8_t * addr)
{
	addr[0] = IINCHIP_READ(SIPR0);
	addr[1] = IINCHIP_READ(SIPR0+1);
	addr[2] = IINCHIP_READ(SIPR0+2);
	addr[3] = IINCHIP_READ(SIPR0+3);
}
/**
@brief	These below functions are used to get the Destination Hardware Address (MAC Address), Destination IP address and Destination Port.
*/
void getSn_DHAR(SOCKET s, uint8_t * addr)
{
	addr[0] = IINCHIP_READ(Sn_DHAR0(s));
	addr[1] = IINCHIP_READ(Sn_DHAR0(s)+1);
	addr[2] = IINCHIP_READ(Sn_DHAR0(s)+2);
	addr[3] = IINCHIP_READ(Sn_DHAR0(s)+3);
	addr[4] = IINCHIP_READ(Sn_DHAR0(s)+4);
	addr[5] = IINCHIP_READ(Sn_DHAR0(s)+5);
}
void setSn_DHAR(SOCKET s, uint8_t * addr)
{
	IINCHIP_WRITE((Sn_DHAR0(s) + 0),addr[0]);
	IINCHIP_WRITE((Sn_DHAR0(s) + 1),addr[1]);
	IINCHIP_WRITE((Sn_DHAR0(s) + 2),addr[2]);
	IINCHIP_WRITE((Sn_DHAR0(s) + 3),addr[3]);
	IINCHIP_WRITE((Sn_DHAR0(s) + 4),addr[4]);
	IINCHIP_WRITE((Sn_DHAR0(s) + 5),addr[5]);
}
void getSn_DIPR(SOCKET s, uint8_t * addr)
{
	addr[0] = IINCHIP_READ(Sn_DIPR0(s));
	addr[1] = IINCHIP_READ(Sn_DIPR0(s)+1);
	addr[2] = IINCHIP_READ(Sn_DIPR0(s)+2);
	addr[3] = IINCHIP_READ(Sn_DIPR0(s)+3);
}
void setSn_DIPR(SOCKET s, uint8_t * addr)
{
	IINCHIP_WRITE((Sn_DIPR0(s) + 0),addr[0]);
	IINCHIP_WRITE((Sn_DIPR0(s) + 1),addr[1]);
	IINCHIP_WRITE((Sn_DIPR0(s) + 2),addr[2]);
	IINCHIP_WRITE((Sn_DIPR0(s) + 3),addr[3]);
}
void getSn_DPORT(SOCKET s, uint8_t * addr)
{
	addr[0] = IINCHIP_READ(Sn_DPORT0(s));
	addr[1] = IINCHIP_READ(Sn_DPORT0(s)+1);
}
void setSn_DPORT(SOCKET s, uint8_t * addr)
{
	IINCHIP_WRITE((Sn_DPORT0(s) + 0),addr[0]);
	IINCHIP_WRITE((Sn_DPORT0(s) + 1),addr[1]);
}
/**
@brief	This sets the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
*/
void setSn_MSS(SOCKET s, uint16_t Sn_MSSR0)
{
	IINCHIP_WRITE(Sn_MSSR0(s),(uint8_t)((Sn_MSSR0 & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_MSSR0(s) + 1),(uint8_t)(Sn_MSSR0 & 0x00ff));
}
void setSn_TTL(SOCKET s, uint8_t ttl)
{
   IINCHIP_WRITE(Sn_TTL(s), ttl);
}
/**
@brief	These below function is used to setup the Protocol Field of IP Header when
executing the IP Layer RAW mode.
*/
void setSn_PROTO(SOCKET s, uint8_t proto)
{
	IINCHIP_WRITE(Sn_PROTO(s),proto);
}
/**
@brief	get socket interrupt status
These below functions are used to read the Interrupt & Soket Status register
*/
uint8_t getSn_IR(SOCKET s)
{
   return IINCHIP_READ(Sn_IR(s));
}
/**/
void setSn_IR(SOCKET s, uint8_t ir)
{
	IINCHIP_WRITE(Sn_IR(s),ir);
}
/*get socket status*/
uint8_t getSn_SR(SOCKET s)
{
   return IINCHIP_READ(Sn_SR(s));
}
/**
@brief	get socket TX free buf size
This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint16_t getSn_TX_FSR(SOCKET s)
{
	uint16_t val=0,val1=0;
	do{
		val1 = IINCHIP_READ(Sn_TX_FSR0(s));
		val1 = (val1 << 8) + IINCHIP_READ(Sn_TX_FSR0(s) + 1);
		if (val1 != 0)
		{
   			val = IINCHIP_READ(Sn_TX_FSR0(s));
   			val = (val << 8) + IINCHIP_READ(Sn_TX_FSR0(s) + 1);
		}
	} while (val != val1);
	return val;
}
/**
@brief	 get socket RX recv buf size
This gives size of received data in receive buffer. 
*/
uint16_t getSn_RX_RSR(SOCKET s)
{
	uint16_t val=0,val1=0;
	do{
		val1 = IINCHIP_READ(Sn_RX_RSR0(s));
		val1 = (val1 << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
		if(val1 != 0)
		{
   			val = IINCHIP_READ(Sn_RX_RSR0(s));
   			val = (val << 8) + IINCHIP_READ(Sn_RX_RSR0(s) + 1);
		}
	}while (val != val1);
	return val;
}
/**
@brief	 This function is being called by send() and sendto() function also. 
This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
*/
void send_data_processing(SOCKET s, uint8_t *data, uint16_t len)
{
	uint16_t ptr;
	ptr = IINCHIP_READ(Sn_TX_WR0(s));
	ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_TX_WR0(s) + 1);
	write_data(s, data, (uint8_t *)(ptr), len);
	ptr += len;
	IINCHIP_WRITE(Sn_TX_WR0(s),(uint8_t)((ptr & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_TX_WR0(s) + 1),(uint8_t)(ptr & 0x00ff));
}
/**
@brief	This function is being called by recv() also.
This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
*/
void recv_data_processing(SOCKET s, uint8_t *data, uint16_t len)
{
	uint16_t ptr;
	ptr = IINCHIP_READ(Sn_RX_RD0(s));
	ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_RX_RD0(s) + 1);
	read_data(s, (uint8_t *)ptr, data, len); // read data
	ptr += len;
	IINCHIP_WRITE(Sn_RX_RD0(s),(uint8_t)((ptr & 0xff00) >> 8));
	IINCHIP_WRITE((Sn_RX_RD0(s) + 1),(uint8_t)(ptr & 0x00ff));
}
/**
@brief	for copy the data form application buffer to Transmite buffer of the chip.
This function is being used for copy the data form application buffer to Transmite
buffer of the chip. It calculate the actual physical address where one has to write
the data in transmite buffer. Here also take care of the condition while it exceed
the Tx memory uper-bound of socket.
*/
void write_data(SOCKET s, uint8_t * src, uint8_t * dst, uint16_t len)
{
	uint16_t size;
	uint16_t dst_mask;
	uint8_t * dst_ptr;
	dst_mask = (uint16_t)dst & getIINCHIP_TxMASK(s);
	dst_ptr = (uint8_t *)(getIINCHIP_TxBASE(s) + dst_mask);
	if (dst_mask + len > getIINCHIP_TxMAX(s)) 
	{
		size = getIINCHIP_TxMAX(s) - dst_mask;
		wiz_write_buf((uint16_t)dst_ptr, (uint8_t*)src, size);
		src += size;
		size = len - size;
		dst_ptr = (uint8_t *)(getIINCHIP_TxBASE(s));
		wiz_write_buf((uint16_t)dst_ptr, (uint8_t*)src, size);
	} 
	else
	{
		wiz_write_buf((uint16_t)dst_ptr, (uint8_t*)src, len);
	}
}
/**
@brief	This function is being used for copy the data form Receive buffer of the chip to application buffer.
It calculate the actual physical address where one has to read
the data from Receive buffer. Here also take care of the condition while it exceed
the Rx memory uper-bound of socket.
*/
void read_data(SOCKET s, uint8_t * src, uint8_t * dst, uint16_t len)
{
	uint16_t size;
	uint16_t src_mask;
	uint8_t * src_ptr;
	src_mask = (uint16_t)src & getIINCHIP_RxMASK(s);
	src_ptr = (uint8_t *)(getIINCHIP_RxBASE(s) + src_mask);
	if( (src_mask + len) > getIINCHIP_RxMAX(s) ) 
	{
		size = getIINCHIP_RxMAX(s) - src_mask;
		wiz_read_buf((uint16_t)src_ptr, (uint8_t*)dst,size);
		dst += size;
		size = len - size;
		src_ptr = (uint8_t *)(getIINCHIP_RxBASE(s));
		wiz_read_buf((uint16_t)src_ptr, (uint8_t*) dst,size);
	} 
	else
	{
		wiz_read_buf((uint16_t)src_ptr, (uint8_t*) dst,len);
	}
}