#ifndef LCD_PCD8544
#define LCD_PCD8544

#include <stdint.h>
#include <stdbool.h>

#define 	LCD5110_RST_PIN 		5  // RST
#define   LCD5110_DC_PIN			4  // D/C
#define		LCD5110_SCE_PIN			3  // SCE
#define		LCD5110_DN_PIN			0  // MOSI
#define		LCD5110_SCLK_PIN		2  // SCLK

// Упраление линией LCD_RST
#define LCD_RST1  						nrf_gpio_pin_set(LCD5110_RST_PIN)
#define LCD_RST0   						nrf_gpio_pin_clear(LCD5110_RST_PIN)
// Упраление линией LCD_DC
#define LCD5110_SET_DATA      nrf_gpio_pin_set(LCD5110_DC_PIN)
#define LCD5110_SET_COMMAND   nrf_gpio_pin_clear(LCD5110_DC_PIN)
// Управление линией LCD_CS
#define LCD5110_DESELECT   		nrf_gpio_pin_set(LCD5110_SCE_PIN)
#define LCD5110_SELECT     		nrf_gpio_pin_clear(LCD5110_SCE_PIN)
//-------------------------------------------------------------//

#define LCD5110_COMMAND 0
#define LCD5110_DATA    1

//-------------------------------------------------------------//

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

// H = 0
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80
#define PCD8544_HPIXELS	84
#define PCD8544_VBANKS	6
#define PCD8544_MAXBYTES 504 // PCD8544_HPIXELS * PCD8544_VBANKS

// H = 1
#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

//transform
#define NONE 0x00
#define FLIP_H 0x01
#define FLIP_V 0x02
#define ROTATE 0x04 // 90 deg CW
#define ROTATE_90_CW ROTATE
#define ROTATE_90_CCW (FLIP_H | FLIP_V | ROTATE)
#define ROTATE_180 (FLIP_H | FLIP_V)

//-------------------------------------------------------------//
void initLCD();
void writeToLCD(unsigned char dataCommand, unsigned char data);

void clearLCD();
void clearBank(unsigned char bank);
void setAddr(unsigned char xAddr, unsigned char yAddr);
void Set_pixel(unsigned char x, unsigned char y);
void writeStringToLCD(const char *string);
void writeCharToLCD(char c);
void writeBlockToLCD(char *byte, unsigned char length);
void writeGraphicToLCD(const char *byte, unsigned char transform);



#endif