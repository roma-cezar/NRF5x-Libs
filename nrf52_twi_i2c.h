#ifndef nrf52_twi_i2c_H
#define nrf52_twi_i2c_H

#if defined (NRF51)
		#include "nrf51_bitfields.h"
#elif defined (NRF52)
		#include "nrf52_bitfields.h"
#endif
#include <stdint.h>
#include <stdbool.h>

#define I2C_TWI_STOP 1
#define TWI_MASTER NRF_TWI1

#define TWI_I2C_ENABLED  5
#define TWI_I2C_DISABLED 0

#define TWI_I2C_SDA_PIN  (30U)
#define TWI_I2C_SCL_PIN  (31U)

#define TWI_I2C_SPEED_100K  0x01980000
#define TWI_I2C_SPEED_250K  0x04000000
#define TWI_I2C_SPEED_400K  0x06680000

#define MAX_TIMEOUT_LOOPS   1000000

typedef struct 
{
	uint32_t speed;
	uint32_t sda_pin;
	uint32_t scl_pin;
	uint32_t enable; 
}nrf52_twi_i2c_init_p;

void TWI_I2C_Init(void);
void TWI_I2C_Write_Byte(uint8_t slave_adr, uint8_t reg_data);
bool TWI_I2C_Read_Buf(uint8_t slave_adr, uint8_t *data, uint8_t data_length);
bool TWI_I2C_Write_Buff(uint8_t slave_adr, uint8_t *data, uint8_t data_length, uint8_t stop_condition);
#endif /* nrf52_twi_i2c_H */