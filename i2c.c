/*
 * i2c-soft.c
 *
 *  Created on: 19.12.2009
 *      Author: Pavel V. Gololobov
 */
#if defined (NRF51)
		#include "nrf51.h"
#elif defined (NRF52)
		#include "nrf52.h"
#endif

#include "nrf_gpio.h"
#include "i2c.h"

//------------------------------------------------------------------
// I2C Speed Down
//------------------------------------------------------------------
#define I2CWAIT i2c_Wait(1) //100 kHz operations - 10
														//400 kHz operations - 3
														//1 	MHz operations - 1 witout __nop
//------------------------------------------------------------------
// I2C Delay
//------------------------------------------------------------------
void i2c_Wait(int32_t n)
{
	do
	{
		__nop();
	}
	while(--n);
}

//------------------------------------------------------------------
// I2C SDA SCL Control
//------------------------------------------------------------------
void SetLowSDA(void)
{
	nrf_gpio_pin_clear(SDA_PIN);
}
void SetHighSDA(void)
{
	nrf_gpio_pin_set(SDA_PIN);
}

void SetLowSCL(void)
{
	nrf_gpio_pin_clear(SCL_PIN);
}
void SetHighSCL(void)
{
	nrf_gpio_pin_set(SCL_PIN);
}

//------------------------------------------------------------------
// I2C Initialize Bus
//------------------------------------------------------------------
void i2c_Init(void)
{
	nrf_gpio_pin_clear(SDA_PIN);
	nrf_gpio_pin_clear(SCL_PIN);
	
	nrf_gpio_cfg(SDA_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, GPIO_PIN_CNF_SENSE_Disabled);
	nrf_gpio_cfg(SCL_PIN, NRF_GPIO_PIN_DIR_OUTPUT, NRF_GPIO_PIN_INPUT_CONNECT, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_S0S1, GPIO_PIN_CNF_SENSE_Disabled);
	
	nrf_gpio_pin_set(SCL_PIN);
	nrf_gpio_pin_set(SDA_PIN);
}

//------------------------------------------------------------------
// I2C Start Data Transfer
//------------------------------------------------------------------
void i2c_Start(void)
{
	SetHighSDA();
	I2CWAIT;
	SetHighSCL(); 
	I2CWAIT;
	SetLowSDA();
	I2CWAIT;
	SetLowSCL();
	I2CWAIT;
}
//------------------------------------------------------------------
// I2C Stop  Transfer
//------------------------------------------------------------------
void i2c_Stop(void)
{
	SetLowSDA();
	I2CWAIT;
	SetHighSCL();
	I2CWAIT;
	SetHighSDA();
	I2CWAIT;
}
//------------------------------------------------------------------
// I2C Write  Transfer
//------------------------------------------------------------------
uint8_t i2c_Write(uint8_t a)
{
	uint8_t i;
	uint8_t ack;

	for (i = 0; i < 8; i++)
  {
		if (a & 0x80) SetHighSDA();
		else SetLowSDA();
		SetHighSCL();
		I2CWAIT;
		a<<=1;
		SetLowSCL();
		I2CWAIT;
	}
	/* ack Read */
	SetHighSDA();
	SetHighSCL();
	I2CWAIT;

	ack = (uint8_t)(nrf_gpio_pin_read(SDA_PIN));

	SetLowSCL();
	I2CWAIT;
	return (ack);
}

//------------------------------------------------------------------
// I2C Read  Transfer
//------------------------------------------------------------------
uint8_t i2c_Read(uint8_t ack)
{
	uint8_t i;
	uint8_t caracter = 0x00;

	SetHighSDA();

	for (i = 0; i < 8; i++)
  {
		caracter <<= 1;
		do {
			SetHighSCL();
			I2CWAIT;
		} 
		while ((uint8_t)nrf_gpio_pin_read(SCL_PIN) == 0);
		
		if ((uint8_t)(nrf_gpio_pin_read(SDA_PIN))==1){
			caracter |= 1;
		}
		SetLowSCL();
		I2CWAIT;
	}

	if(ack){
		SetLowSDA();
	}
	else {
		SetHighSDA();
	}

	SetHighSCL();	
	I2CWAIT;
	
	SetLowSCL();
	SetHighSDA();
	I2CWAIT;
	return (caracter);
}
