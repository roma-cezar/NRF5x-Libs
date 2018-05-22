/*
 * i2c-soft.h
 *
 *  Created on: 19.12.2009
 *      Author: Pavel V. Gololobov
 */

#ifndef I2C_H_
#define I2C_H_

#include <stdint.h>
#include <stdbool.h>
//#include "twi_master_config.h"
#define ACK  1
#define NACK 0

#define SDA_PIN (30)
#define SCL_PIN (31)

void i2c_Wait(int32_t n);
void SetLowSDA(void);
void SetHighSDA(void);
void SetLowSCL(void);
void SetHighSCL(void);

// Init Bus
void i2c_Init(void);
// Start Transfer
void i2c_Start(void);
// Stop Transfer
void i2c_Stop(void);
// Write Transfer
uint8_t i2c_Write(uint8_t a);
// Read Transfer
uint8_t i2c_Read(uint8_t ack);
#endif
