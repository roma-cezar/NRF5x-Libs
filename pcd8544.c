
#include "nrf.h"
#include "nrf_gpio.h"      
#include "pcd8544.h" 
#include "nrf_delay.h"
#include "font6x8.h"

/*********************************************************************************/
// Инициализация
void initLCD() {

		//nrf_gpio_cfg_input(1, NRF_GPIO_PIN_NOPULL);  //MISO	
	nrf_gpio_cfg_output(LCD5110_RST_PIN);	//RST
	nrf_gpio_cfg_output(LCD5110_DC_PIN); 	//DC
	nrf_gpio_cfg_output(LCD5110_SCE_PIN); //CS
	nrf_gpio_cfg_output(LCD5110_DN_PIN); //MOSI
	nrf_gpio_cfg_output(LCD5110_SCLK_PIN); //SCK
	
	nrf_gpio_pin_set(LCD5110_RST_PIN);	//RST
	nrf_gpio_pin_set(LCD5110_DC_PIN); 	//DC
	nrf_gpio_pin_set(LCD5110_SCE_PIN);  //CS
	nrf_gpio_pin_set(LCD5110_DN_PIN);   //MOSI
	nrf_gpio_pin_set(LCD5110_SCLK_PIN); //SCK
	
	NRF_SPI0->POWER = SPI_POWER_POWER_Enabled;	
	NRF_SPI0->PSELMOSI = 0;
	NRF_SPI0->PSELMISO = 1;
	NRF_SPI0->PSELSCK = 2;
	NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_K125;
	NRF_SPI0->CONFIG = 	4;
	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Enabled;
	NRF_SPI0->EVENTS_READY = 0;

	LCD5110_SELECT;
	LCD5110_SET_COMMAND;

	// сброс дисплея
	LCD_RST0;           // RST=0
	nrf_delay_ms(1);		// пауза
	LCD_RST1;           // RST=1
	nrf_delay_ms(10);		// пауза
	
	writeToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION);
	writeToLCD(LCD5110_COMMAND, PCD8544_SETVOP | 0x3F);
	writeToLCD(LCD5110_COMMAND, PCD8544_SETTEMP | 0x02);
	writeToLCD(LCD5110_COMMAND, PCD8544_SETBIAS | 0x03);
	writeToLCD(LCD5110_COMMAND, PCD8544_FUNCTIONSET);
	writeToLCD(LCD5110_COMMAND, PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL);
	
	LCD5110_DESELECT;
}
/*********************************************************************************/
void writeToLCD(unsigned char dataCommand, unsigned char data) {
		uint8_t ret;
    LCD5110_SELECT;
    if(dataCommand) {
        LCD5110_SET_DATA;
    } else {
        LCD5110_SET_COMMAND;
    }

		if(NRF_SPI0->EVENTS_READY == 0)
		{
			NRF_SPI0->TXD = (uint8_t)data;
			ret = NRF_SPI0->RXD;
		}
		while(NRF_SPI0->EVENTS_READY != 1);
		NRF_SPI0->EVENTS_READY = 0;
}

/*********************************************************************************/
void writeCharToLCD(char c) {
    unsigned char i;
    for(i = 0; i < 5; i++) {
        writeToLCD(LCD5110_DATA, font[c - 0x20][i]);
    }
    writeToLCD(LCD5110_DATA, 0);
}
/*********************************************************************************/
void writeStringToLCD(const char *string) {
    while(*string) {
        writeCharToLCD(*string++);
    }
}
/*********************************************************************************/
void writeBlockToLCD(char *byte, unsigned char length) {
    unsigned char c = 0;
    while(c < length) {
        writeToLCD(LCD5110_DATA, *byte++);
        c++;
    }
}
/*********************************************************************************/
void writeGraphicToLCD(const char *byte, unsigned char transform) {
    int c = 0;
    char block[8];

    if(transform & FLIP_V) {
      //  SPI_LSB_FIRST;
    }
    if(transform & ROTATE) {
        c = 1;
        while(c != 0) {
            (*byte & 0x01) ? (block[7] |= c) : (block[7] &= ~c);
            (*byte & 0x02) ? (block[6] |= c) : (block[6] &= ~c);
            (*byte & 0x04) ? (block[5] |= c) : (block[5] &= ~c);
            (*byte & 0x08) ? (block[4] |= c) : (block[4] &= ~c);
            (*byte & 0x10) ? (block[3] |= c) : (block[3] &= ~c);
            (*byte & 0x20) ? (block[2] |= c) : (block[2] &= ~c);
            (*byte & 0x40) ? (block[1] |= c) : (block[1] &= ~c);
            (*byte & 0x80) ? (block[0] |= c) : (block[0] &= ~c);
            *byte++;
            c <<= 1;
        }
    } else {
        while(c < 8) {
            block[c++] = *byte++;
        }
    }

    if(transform & FLIP_H) {
        c = 7;
        while(c > -1) {
            writeToLCD(LCD5110_DATA, block[c--]);
        }
    } else {
        c = 0;
        while(c < 8) {
            writeToLCD(LCD5110_DATA, block[c++]);
        }
    }
  //  SPI_MSB_FIRST;
}
/*********************************************************************************/
void clearLCD() {
    setAddr(0, 0);
    int c = 0;
    while(c < PCD8544_MAXBYTES) {
        writeToLCD(LCD5110_DATA, 0);
        c++;
    }
    setAddr(0, 0);
}
/*********************************************************************************/
void Set_pixel(unsigned char x, unsigned char y) {
    setAddr(x, y);
    writeToLCD(LCD5110_DATA, 1);
}
/*********************************************************************************/
void clearBank(unsigned char bank) {
    setAddr(0, bank);
    int c = 0;
    while(c < PCD8544_HPIXELS) {
        writeToLCD(LCD5110_DATA, 0);
        c++;
    }
    setAddr(0, bank);
}
/*********************************************************************************/
void setAddr(unsigned char xAddr, unsigned char yAddr) {
    writeToLCD(LCD5110_COMMAND, PCD8544_SETXADDR | xAddr);
    writeToLCD(LCD5110_COMMAND, PCD8544_SETYADDR | yAddr);
}