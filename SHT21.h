/*
 * SHT21.h
 *
 *  Created on: 18.11.2015
 *      Author: Roman
 */

#ifndef SHT21_H_
#define SHT21_H_

#include <stdint.h>

uint16_t SHT21_RHumid(void);
uint16_t SHT21_RTemp(void);
void SHT21_Reset(void);

#endif /* SHT21_H_ */
