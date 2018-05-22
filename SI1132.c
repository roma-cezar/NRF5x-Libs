#include "nrf_delay.h"
#include "nrf51.h"
#include "i2c.h"
#include "SI1132.h"
//
void SI1132_Reset(void)
{
	uint8_t query;
	
	i2c_Start();
	i2c_Write(SI1132_WR); //адрес на запись
	i2c_Write(COMMAND); //запись в командный регистр
	i2c_Write(NOP); // очищаем регистр RESPONSE
	i2c_Stop();
	
	nrf_delay_ms(30);
	
	i2c_Start();
	i2c_Write(SI1132_WR); //  устанавливаем регистр на чтение
	i2c_Write(RESPONSE);
	
	i2c_Start();
	i2c_Write(SI1132_RD); //read
	query = i2c_Read(NACK);
	i2c_Stop();

	if((query & 0x0F)==0)
	{
		i2c_Start();
		i2c_Write(SI1132_WR); //адрес на запись
		i2c_Write(COMMAND); //запись в командный регистр
		i2c_Write(RESET); 
		i2c_Stop();
	}
}
/**/
void SI1132_Init(void)
{
	Write_I2C_Reg(MEAS_RATE0, 0x00);
	Write_I2C_Reg(MEAS_RATE1, 0x00);
	
	Write_I2C_Reg(UCOEF0, 0x7B);
	Write_I2C_Reg(UCOEF1, 0x6B);
	Write_I2C_Reg(UCOEF2, 0x01);
	Write_I2C_Reg(UCOEF3, 0x00);
	
	Write_Param_Reg(CHLIST, 0x90); // enable UV, VIS
	Write_Param_Reg(ALS_VIS_ADC_MISC, 0x20); //enable VIS_RANGE
	Write_Param_Reg(ALS_IR_ADC_MISC, 0x20);  //enable IR_RANGE
}
/**/
void SI1132_Measure(void)
{
	i2c_Start();
	i2c_Write(SI1132_WR); //адрес на запись
	i2c_Write(COMMAND); //запись в командный регистр
	i2c_Write(ALS_FORCE); // запускаем измерение
	i2c_Stop();
	nrf_delay_ms(1);
}
/**/
uint16_t SI1132_GetVis(void)
{
	uint16_t data;
	i2c_Start();
	i2c_Write(SI1132_WR); //  устанавливаем регистр на чтение
	i2c_Write(ALS_VIS_DATA0);
	
	i2c_Start();
	i2c_Write(SI1132_RD); //read
	data = (((uint16_t)i2c_Read(ACK))<<8)|((uint16_t)i2c_Read(NACK));
	i2c_Stop();
	return data;
}
/**/
uint16_t SI1132_GetUV(void)
{
	uint16_t data;
	i2c_Start();
	i2c_Write(SI1132_WR); //  устанавливаем регистр на чтение
	i2c_Write(UVINDEX0);
	
	i2c_Start();
	i2c_Write(SI1132_RD); //read
	data = (((uint16_t)i2c_Read(ACK))<<8)|((uint16_t)i2c_Read(NACK));
	i2c_Stop();
	return data;
}
/**/
uint8_t Write_Param_Reg(uint8_t reg, uint8_t data)
{
	uint8_t query;
	
	i2c_Start();
	i2c_Write(SI1132_WR); //адрес на запись
	i2c_Write(COMMAND); //запись в командный регистр
	i2c_Write(NOP); // очищаем регистр RESPONSE
	i2c_Stop();
	
	nrf_delay_ms(30);
	
	i2c_Start();
	i2c_Write(SI1132_WR); //  устанавливаем регистр на чтение
	i2c_Write(RESPONSE);
	
	i2c_Start();
	i2c_Write(SI1132_RD); //read
	query = i2c_Read(NACK);
	i2c_Stop();

	if((query & 0x0F)==0)
	{
		i2c_Start();
		i2c_Write(SI1132_WR); //адрес на запись
		i2c_Write(COMMAND); //запись в командный регистр
		i2c_Write(PARAM_SET | reg); // адрес на запись в RAM регистр 
		i2c_Write(PARAM_WR); // адрес регистра на устанавлеваемый параметр
		i2c_Write(data); // устанавлеваемый параметр
		i2c_Stop();
		
		nrf_delay_ms(30);
	
		i2c_Start();
		i2c_Write(SI1132_WR); //read
		i2c_Write(RESPONSE);
		
		i2c_Start();
		i2c_Write(SI1132_RD); //read
		query = i2c_Read(NACK);
		i2c_Stop();
		
		if((query & 0x0F)>0)
		{
			return 1;
		}
		else return 0;
	}
	else return 0;
}
/**/
uint8_t  Read_I2C_Reg(uint8_t reg)
{
	uint8_t data;
	
	i2c_Start();
	i2c_Write(SI1132_WR); //адрес на запись
	i2c_Write(reg); // i2c регистр на чтение
		
	i2c_Start();
	i2c_Write(SI1132_RD); //  устанавливаем регистр на чтение
	data = i2c_Read(NACK);  //  данные
	i2c_Stop();

	return data;
}/**/
void Write_I2C_Reg(uint8_t reg, uint8_t data)
{
	i2c_Start();
	i2c_Write(SI1132_WR); //адрес на запись
	i2c_Write(reg); // i2c регистр на запись
	i2c_Write(data); //  данные
	i2c_Stop();
}