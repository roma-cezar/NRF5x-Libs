/*
 * BME280.c
 *
 *  Created on: 18.11.2015
 *      Author: Roman
 */

#include "nrf51.h"
#include "i2c.h"
#include "BME280.h"
#include "nrf_delay.h"

uint16_t dig_T1 = 0x6F15;
int16_t  dig_T2 = 0x6863;
int16_t  dig_T3 = 0x0032;

uint16_t dig_P1 = 0x944F;
int16_t  dig_P2 = 0xD656;
int16_t  dig_P3 = 0x0BD0;
int16_t  dig_P4 = 0x1D1C;
int16_t  dig_P5 = 0xFF76;
int16_t  dig_P6 = 0xFFF9;
int16_t  dig_P7 = 0x26AC;
int16_t  dig_P8 = 0xD80A;
int16_t  dig_P9 = 0x10BD;
uint8_t  dig_H1 = 0x4B;

int16_t  dig_H2 = 0x016B;
uint8_t  dig_H3 = 0x00;

int16_t  dig_H4 = 0x13D;
int16_t  dig_H5 = 0x0D; //may replace tetrades

int8_t   dig_H6 = 0x1E;

int32_t t_fine;




/*-------------------------------------------------------------------*/
// Returns temperature in DegC, resolution is 0.01 DegC. Output value of “5123” equals 51.23 DegC.  
// t_fine carries fine temperature as global value 
int32_t BME280_compensate_T(int32_t adc_T) 
{ 
  int32_t var1, var2, T; 
  var1  = ((((adc_T>>3) - ((int32_t)dig_T1<<1))) * ((int32_t)dig_T2)) >> 11; 
  var2  = (((((adc_T>>4) - ((int32_t)dig_T1)) * ((adc_T>>4) - ((int32_t)dig_T1))) >> 12) *  ((int32_t)dig_T3)) >> 14; 
  t_fine = var1 + var2; 
  T  = (t_fine * 5 + 128) >> 8; 
  return T; 
} 
/*-------------------------------------------------------------------*/ 
// Returns pressure in Pa as unsigned 32 bit integer in Q24.8 format (24 integer bits and 8 fractional bits). 
// Output value of “24674867” represents 24674867/256 = 96386.2 Pa = 963.862 hPa 
uint32_t BME280_compensate_P(int32_t adc_P) 
{ 
  int64_t var1, var2, p; 
  var1 = ((int64_t)t_fine) - 128000; 
  var2 = var1 * var1 * (int64_t)dig_P6; 
  var2 = var2 + ((var1*(int64_t)dig_P5)<<17); 
  var2 = var2 + (((int64_t)dig_P4)<<35); 
  var1 = ((var1 * var1 * (int64_t)dig_P3)>>8) + ((var1 * (int64_t)dig_P2)<<12); 
  var1 = (((((int64_t)1)<<47)+var1))*((int64_t)dig_P1)>>33; 
  if (var1 == 0) 
  { 
    return 0; // avoid exception caused by division by zero 
  } 
  p = 1048576-adc_P; 
  p = (((p<<31)-var2)*3125)/var1; 
  var1 = (((int64_t)dig_P9) * (p>>13) * (p>>13)) >> 25; 
  var2 = (((int64_t)dig_P8) * p) >> 19; 
  p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7)<<4); 
  return (uint32_t)p; 
} 
/*-------------------------------------------------------------------*/
// Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format (22 integer and 10 fractional bits). 
// Output value of “47445” represents 47445/1024 = 46.333 %RH 
uint32_t BME280_compensate_H(int32_t adc_H) 
{ 
  int32_t v_x1_u32r;  
  v_x1_u32r = (t_fine - ((int32_t)76800)); 
  v_x1_u32r = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v_x1_u32r)) + ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)dig_H6)) >> 10) * (((v_x1_u32r * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14)); 
  v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4)); 
  v_x1_u32r = (v_x1_u32r < 0 ? 0 : v_x1_u32r);  
  v_x1_u32r = (v_x1_u32r > 419430400 ? 419430400 : v_x1_u32r);   
  return (uint32_t)(v_x1_u32r>>12); 
}
/*-------------------------------------------------------------------*/
void BME280_Config(void)
{
	i2c_Start();
	i2c_Write(BME_WR); //write to BME280
	i2c_Write(BME_CTH);//write to ctrl hum reg
	i2c_Write(0x01);//write osrs_h = 1 ovrsmpl
	i2c_Stop();
	
	i2c_Start();
	i2c_Write(BME_WR); //write to BME280
	i2c_Write(BME_CTM);//write to ctrl meas reg
	i2c_Write(0x25);//write osrs_p = 1 & osrs_t = 1 ovrsmpl,  force mode (0b001_001_01) perform one measurement, store results and return to sleep mode
	i2c_Stop();	
	
	i2c_Start();
	i2c_Write(BME_WR); //write to BME280
	i2c_Write(BME_CFG);//write to config reg
	i2c_Write(0x00);//write t_stb = 0.5 ms (0x40 for t_stb = 125 ms)
	i2c_Stop();
}
/*-------------------------------------------------------------------*/
void BME280_Reset(void)
{
	i2c_Start();
	i2c_Write(BME_WR); //write to BME280
	i2c_Write(BME_RST);//write toreset reg
	i2c_Write(0xB6);//write reset value
	i2c_Stop();
}
/*-------------------------------------------------------------------*/
void BME280_Read_PTH(uint8_t *data)
{
	uint8_t i;
	i2c_Start();
	i2c_Write(BME_WR);
	i2c_Write(BME_PTH); //read presss, temp, hum
	nrf_delay_ms(15);// 15 ms delay for measure PTH

	i2c_Start();
	i2c_Write(BME_RD); //read
	for(i=0;i<7;i++)
	{
		//read 25 bytes
		data[i] = i2c_Read(ACK);
	}
	data[7] = i2c_Read(NACK);
	i2c_Stop();
}
/*-------------------------------------------------------------------*/
void BME280_Read_Cal_data(uint8_t *data)
{
	uint8_t i;
	i2c_Start();
	i2c_Write(BME_WR); //write
	i2c_Write(0x88); //start address
	i2c_Start();
	i2c_Write(BME_RD); //read 25 bytes from 0x88 to 0xA1
	for(i=0;i<25;i++)
	{
		//read 25 bytes
		data[i] = i2c_Read(ACK);
	}
	data[25] = i2c_Read(NACK);
	i2c_Stop();
	/*-----------------------*/
	nrf_delay_ms(100);
	i2c_Start();
	i2c_Write(BME_WR); //write
	i2c_Write(0xE1); //start address
	i2c_Start();
	i2c_Write(BME_RD); //read 7 bytes from 0xE1 to 0xE7
	for(i=0;i<6;i++)
	{
		data[26+i] = i2c_Read(ACK);
	}
	data[32] = i2c_Read(NACK);
	i2c_Stop();	
}
/*-------------------------------------------------------------------*/