/*
 * SHT21.c
 *
 *  Created on: 18.11.2015
 *      Author: Roman
 */

#if defined (NRF51)
		#include "nrf51.h"
#elif defined (NRF52)
		#include "nrf52.h"
#endif

#define I2C_SOFT_MODE

#if defined (I2C_HARD_MODE)
		#include "nrf52_twi_i2c.h"
#elif defined (I2C_SOFT_MODE)
		#include "i2c.h"
#endif

#include "SHT21.h"
#include "nrf_delay.h"


void SHT21_Reset(void)
{
#if defined (I2C_SOFT_MODE)
	i2c_Start();
	i2c_Write(0x80);
	i2c_Write(0xFE);
	i2c_Stop();
#elif defined (I2C_HARD_MODE)
	TWI_I2C_Write_Byte(0x80, 0xFE);
#endif	
	nrf_delay_ms(20);
}

uint16_t SHT21_RTemp(void)
{
	uint16_t UT;
#if defined (I2C_SOFT_MODE)
	i2c_Start();
	i2c_Write(0x80);
	i2c_Write(0xE3); //e3
	//nrf_delay_ms(65);
	i2c_Start();
	i2c_Write(0x81); //read
	UT |= ((uint16_t)i2c_Read(ACK)) <<8;
	UT |= (uint16_t)i2c_Read(NACK);
	i2c_Stop();
#elif defined (I2C_HARD_MODE)
	uint8_t I2C_BUF[2];
	TWI_I2C_Write_Byte(0x80, 0xE3);
	TWI_I2C_Read_Buf(0x80, I2C_BUF, 2);
	UT = (((uint16_t)I2C_BUF[0])<<8)|((uint16_t)I2C_BUF[1]);
#endif	
	return UT;
}

uint16_t SHT21_RHumid(void)
{
	uint16_t UH;
#if defined (I2C_SOFT_MODE)	
	i2c_Start();
	i2c_Write(0x80);
	i2c_Write(0xE5); //e5
	//nrf_delay_ms(25);
	i2c_Start();
	i2c_Write(0x81); //read
	UH |= ((uint16_t)i2c_Read(ACK)) <<8;
	UH |= (uint16_t)i2c_Read(NACK);
	i2c_Stop();
#elif defined (I2C_HARD_MODE)
	uint8_t I2C_BUF[2];
	TWI_I2C_Write_Byte(0x80, 0xE5);
	TWI_I2C_Read_Buf(0x80, I2C_BUF, 2);
	UH = (((uint16_t)I2C_BUF[0])<<8)|((uint16_t)I2C_BUF[1]);
#endif	
	return UH;
}
