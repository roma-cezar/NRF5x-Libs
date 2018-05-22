/*
 * BME280.h
 *
 *  Created on: 18.11.2015
 *      Author: Roman
 */
#include <stdint.h>

#define BME_ID 	0xD0	//id as 0x60
#define BME_WR 	0xEC	//write data
#define BME_RD 	0xED	//read	data
#define BME_RST 0xE0	//reset by writing 0xB6 only
#define BME_CTH 0xF2  //ctrl humidity reg
#define BME_STA 0xF3	//status reg
#define BME_CTM 0xF4	//ctrl meas reg
#define BME_CFG 0xF5	//config reg

#define BME_PTH 0xF7  //8bytes to read from 0xF7 to 0xFE

void BME280_Config(void);
void BME280_Reset(void);
void BME280_Read_PTH(uint8_t *data);
void BME280_Read_Cal_data(uint8_t *data);

int32_t BME280_compensate_T(int32_t adc_T);
uint32_t BME280_compensate_P(int32_t adc_P);
uint32_t BME280_compensate_H(int32_t adc_H);
/**/