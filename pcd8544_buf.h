#ifndef LCD_PCD8544
#define LCD_PCD8544

#include <stdint.h>
#include <stdbool.h>

#define LCD_TO_SPI2
#define LCD_RST_DLY    2	

#define 	LCD_RST_PIN 		5
//                             +--+
#define   LCD_DC_PIN			4   // D/C
//                             +--+
#define		LCD_CSE_PIN			3   // SCE
//                             +--+
#define		LCD_SDA_PIN			0  // MOSI
//                             +--+
#define		LCD_SCK_PIN			2  //SCLK
//                             +--+

// Упраление линией LCD_RST
#define LCD_RST1  nrf_gpio_pin_set(LCD_RST_PIN)
#define LCD_RST0  nrf_gpio_pin_clear(LCD_RST_PIN)
// Упраление линией LCD_DC
#define LCD_DC1   nrf_gpio_pin_set(LCD_DC_PIN)
#define LCD_DC0   nrf_gpio_pin_clear(LCD_DC_PIN)
// Управление линией LCD_CS
#define LCD_CS1   nrf_gpio_pin_set(LCD_CSE_PIN)
#define LCD_CS0   nrf_gpio_pin_clear(LCD_CSE_PIN)
//-------------------------------------------------------------//
void lcd8544_init(void); // инициализаци¤ диспле
void lcd8544_senddata(uint8_t data);
		// отправка команды на дисплей
void lcd8544_sendCmd(unsigned char cmd);
		// очистка диспле¤
void lcd8544_refresh(void);
		// очистка буфера
void lcd8544_clear(void);
		// вывод пиксела
void lcd8544_putpix(unsigned char x, unsigned char y, unsigned char mode);
		// вывод линии
void lcd8544_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode);
		// рисование пр¤моугольника (не заполненного)
void lcd8544_rect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode);
		// вывод символа в цвете по координатам
void lcd8544_putchar(unsigned char px, unsigned char py, unsigned char ch, unsigned char mode);
		// лини¤ левее символа дл¤ печати в инверсе
void lcd8544_leftchline(unsigned char x, unsigned char y);
		// вывод строки в цвете по координатам
void lcd8544_putstr(unsigned char x, unsigned char y, const unsigned char str[], unsigned char mode);
		// печать дес¤тичного числа
void lcd8544_dec(unsigned int numb, unsigned char dcount, unsigned char x, unsigned char y, unsigned char mode);

#endif