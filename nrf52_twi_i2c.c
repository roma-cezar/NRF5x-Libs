#if defined (NRF51)
		#include "nrf51.h"
#elif defined (NRF52)
		#include "nrf52.h"
#endif

#include "nrf_gpio.h"
#include "nrf52_twi_i2c.h"
#include "nrf_delay.h"


nrf52_twi_i2c_init_p nrf52_twi_i2c_init;

		
void TWI_I2C_Init(void)
{
	NRF_GPIO->PIN_CNF[TWI_I2C_SCL_PIN] =     \
			(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
		| (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
		| (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
		| (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
		| (GPIO_PIN_CNF_DIR_Output      << GPIO_PIN_CNF_DIR_Pos);

	NRF_GPIO->PIN_CNF[TWI_I2C_SDA_PIN] =      \
			(GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
		| (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
		| (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
		| (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
		| (GPIO_PIN_CNF_DIR_Output      << GPIO_PIN_CNF_DIR_Pos);
	
	TWI_MASTER->EVENTS_RXDREADY = 0;
  TWI_MASTER->EVENTS_TXDSENT  = 0;
	TWI_MASTER->FREQUENCY = TWI_I2C_SPEED_100K;
	TWI_MASTER->PSELSDA = TWI_I2C_SDA_PIN;
	TWI_MASTER->PSELSCL = TWI_I2C_SCL_PIN;
	
	NRF_PPI->CH[0].EEP        = (uint32_t)&TWI_MASTER->EVENTS_BB;
	NRF_PPI->CH[0].TEP        = (uint32_t)&TWI_MASTER->TASKS_SUSPEND;
	NRF_PPI->CHENCLR          = PPI_CHENCLR_CH0_Msk;
		
	TWI_MASTER->ENABLE = TWI_I2C_ENABLED;
	nrf_delay_ms(5);
}

void TWI_I2C_Write_Byte(uint8_t slave_adr, uint8_t reg_data)
{

	TWI_MASTER->ADDRESS = slave_adr >> 1;	
	TWI_MASTER->TXD = (uint32_t)reg_data;
	
	TWI_MASTER->EVENTS_TXDSENT = 0;
	TWI_MASTER->TASKS_STARTTX = 1;
	while(TWI_MASTER->EVENTS_TXDSENT == 0 && TWI_MASTER->EVENTS_ERROR == 0);
	TWI_MASTER->EVENTS_TXDSENT = 0;	
	
	TWI_MASTER->EVENTS_STOPPED = 0;
	TWI_MASTER->TASKS_STOP = 1;
	while(TWI_MASTER->EVENTS_STOPPED == 0 && TWI_MASTER->EVENTS_ERROR == 0);
	TWI_MASTER->EVENTS_STOPPED = 0;		
}
/***********************************************************************************/

/***********************************************************************************/
bool TWI_I2C_Write_Buff(uint8_t slave_adr, uint8_t *data, uint8_t data_length, uint8_t stop_condition)
{	
		TWI_MASTER->ADDRESS = slave_adr >> 1;	
		if (data_length == 0)
    {
        /* Return false for requesting data of size 0 */
        return false;
    }
    TWI_MASTER->TXD = *data++;
    TWI_MASTER->TASKS_STARTTX = 1;
    while (true)
    {
        while (TWI_MASTER->EVENTS_TXDSENT == 0 && TWI_MASTER->EVENTS_ERROR == 0){}
        if (NRF_TWI1->EVENTS_ERROR != 0)
        {
            return false;
        }
        TWI_MASTER->EVENTS_TXDSENT = 0;
        if (--data_length == 0)
        {
            break;
        }
        TWI_MASTER->TXD = *data++;
    }
    if (stop_condition)
    {
        TWI_MASTER->EVENTS_STOPPED = 0;
        TWI_MASTER->TASKS_STOP     = 1;
        /* Wait until stop sequence is sent */ 
        while(TWI_MASTER->EVENTS_STOPPED == 0){}
    }
    return true;
}

bool TWI_I2C_Read_Buf(uint8_t slave_adr, uint8_t *data, uint8_t data_length)
{
	//uint32_t timeout = MAX_TIMEOUT_LOOPS;
	
	if (data_length == 0)
	{
			/* Return false for requesting data of size 0 */
			return false;
	}
	else if (data_length == 1)
	{
			NRF_PPI->CH[0].TEP = (uint32_t)&TWI_MASTER->TASKS_STOP;
	}
	else
	{
			NRF_PPI->CH[0].TEP = (uint32_t)&TWI_MASTER->TASKS_SUSPEND;
	}
	
	NRF_PPI->CHENSET          = PPI_CHENSET_CH0_Msk;	
	TWI_MASTER->EVENTS_RXDREADY = 0;
  TWI_MASTER->TASKS_STARTRX   = 1;
	
	while(true)
	{
		while (TWI_MASTER->EVENTS_RXDREADY == 0 && TWI_MASTER->EVENTS_ERROR == 0);
		TWI_MASTER->EVENTS_RXDREADY = 0;
		
		if (TWI_MASTER->EVENTS_ERROR != 0)
    {
			return false;
    }			
					
		*data++ = TWI_MASTER->RXD;
		
		if (--data_length == 1) NRF_PPI->CH[0].TEP = (uint32_t)&TWI_MASTER->TASKS_STOP;
		if (data_length == 0) break;
		nrf_delay_us(20);
		TWI_MASTER->TASKS_RESUME = 1;
	}
	//TWI_MASTER->TASKS_STOP;
	while(TWI_MASTER->EVENTS_STOPPED == 0);
	TWI_MASTER->EVENTS_STOPPED = 0;
	
	NRF_PPI->CHENCLR = PPI_CHENCLR_CH0_Msk;
	return true;
}