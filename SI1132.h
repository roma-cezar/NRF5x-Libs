#include <stdint.h>

#define SI1132_WR 	0x60	//write data
#define SI1132_RD 	0x61	//read	data
/*********************COMMANDS*********************/
#define PARAM_QUERY 0x80 // PARAM_QUERY|RAM_OFFSET
#define PARAM_SET 	0xA0 // PARAM_SET|RAM_OFFSET
#define NOP 				0x00
#define RESET 			0x01
#define BUSADDR 		0x02
#define GET_CAL 		0x12
#define ALS_FORCE 	0x06
#define ALS_PAUSE 	0x0A
#define ALS_AUTO 		0x0E
/*************I2C REGISTERS (page 24)**************/
#define PART_ID 			0x00												//
#define REV_ID 				0x01												//
#define SEQ_ID 				0x02												//
#define INT_CFG 			0x03												//
#define IRQ_ENABLE 		0x04												//
#define HW_KEY 				0x07												//
#define MEAS_RATE0 		0x08												//
#define MEAS_RATE1 		0x09												//
#define UCOEF0 				0x13												//
#define UCOEF1 				0x14												//
#define UCOEF2 				0x15												//
#define UCOEF3 				0x16												//
#define PARAM_WR 			0x17												//
#define COMMAND 			0x18												//
#define RESPONSE 			0x20												//
#define IRQ_STATUS 		0x21												//
#define ALS_VIS_DATA0 0x22												//
#define ALS_VIS_DATA1 0x23												//
#define ALS_IR_DATA0 	0x24												//
#define ALS_IR_DATA1 	0x25												//
#define UVINDEX0 			0x2C												//
#define UVINDEX1 			0x2D												//
#define PARAM_RD 			0x2E												//
#define CHIP_STAT 		0x30												//
#define ANA_IN_KEY 		0x3B // (4)									//

/**************RAM SECTION (page 35)***************/
#define I2C_ADDR 						0x00									//
#define CHLIST 							0x01									//
#define ALS_ENCODING 				0x06									//
#define ALS_IR_ADCMUX 			0x0E									//
#define AUX_ADCMUX 					0x0F									//
#define ALS_VIS_ADC_COUNTER 0x10									//
#define ALS_VIS_ADC_GAIN 		0x11									//
#define ALS_VIS_ADC_MISC 		0x12									//
#define ALS_IR_ADC_COUNTER 	0x1D									//
#define ALS_IR_ADC_GAIN 		0x1E									//
#define ALS_IR_ADC_MISC 		0x1F									//
/**************************************************/
void SI1132_Reset(void);
void SI1132_Init(void);
void SI1132_Measure(void);
uint16_t SI1132_GetVis(void);
uint16_t SI1132_GetUV(void);

uint8_t Write_Param_Reg(uint8_t reg, uint8_t data);
uint8_t Read_I2C_Reg(uint8_t reg);
void 		Write_I2C_Reg(uint8_t reg, uint8_t data);