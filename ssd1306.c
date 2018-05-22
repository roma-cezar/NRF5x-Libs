#include "ssd1306.h"

#if defined (NRF51)
		#include "nrf51.h"
#elif defined (NRF52)
		#include "nrf52.h"
#endif

#define I2C_SOFT_MODE

#if defined (I2C_HARD_MODE)
		#include "nrf52_twi_i2c.h"
#elif defined (I2C_SOFT_MODE)
		#include "i2c.h"
#endif

#include "nrf_delay.h"
#include <stdint.h>
#include <stdio.h>

uint8_t LCD_X,LCD_Y;

uint8_t buffer_pointer[(SSD1306_LCDWIDTH/8)*SSD1306_LCDHEIGHT];

const uint8_t Font5x8[][22] = { 
	{0x00, 0x00, 0x00, 0x00, 0x00},// (space)
	{0x00, 0x00, 0x5F, 0x00, 0x00},// !
	{0x00, 0x07, 0x00, 0x07, 0x00},// "
	{0x14, 0x7F, 0x14, 0x7F, 0x14},// #
	{0x24, 0x2A, 0x7F, 0x2A, 0x12},// $
	{0x23, 0x13, 0x08, 0x64, 0x62},// %
	{0x36, 0x49, 0x55, 0x22, 0x50},// &
	{0x00, 0x05, 0x03, 0x00, 0x00},// '
	{0x00, 0x1C, 0x22, 0x41, 0x00},// (
	{0x00, 0x41, 0x22, 0x1C, 0x00},// )
	{0x08, 0x2A, 0x1C, 0x2A, 0x08},// *
	{0x08, 0x08, 0x3E, 0x08, 0x08},// +
	{0x00, 0x50, 0x30, 0x00, 0x00},// ,
	{0x08, 0x08, 0x08, 0x08, 0x08},// -
	{0x00, 0x30, 0x30, 0x00, 0x00},// .
	{0x20, 0x10, 0x08, 0x04, 0x02},// /
	{0x3E, 0x51, 0x49, 0x45, 0x3E},// 0
	{0x00, 0x42, 0x7F, 0x40, 0x00},// 1
	{0x42, 0x61, 0x51, 0x49, 0x46},// 2
	{0x21, 0x41, 0x45, 0x4B, 0x31},// 3
	{0x18, 0x14, 0x12, 0x7F, 0x10},// 4
	{0x27, 0x45, 0x45, 0x45, 0x39},// 5
	{0x3C, 0x4A, 0x49, 0x49, 0x30},// 6
	{0x01, 0x71, 0x09, 0x05, 0x03},// 7
	{0x36, 0x49, 0x49, 0x49, 0x36},// 8
	{0x06, 0x49, 0x49, 0x29, 0x1E},// 9
	{0x00, 0x36, 0x36, 0x00, 0x00},// :
	{0x00, 0x56, 0x36, 0x00, 0x00},// ;
	{0x00, 0x08, 0x14, 0x22, 0x41},// <
	{0x14, 0x14, 0x14, 0x14, 0x14},// =
	{0x41, 0x22, 0x14, 0x08, 0x00},// >
	{0x02, 0x01, 0x51, 0x09, 0x06},// ?
	{0x32, 0x49, 0x79, 0x41, 0x3E},// @
	{0x7E, 0x11, 0x11, 0x11, 0x7E},// A
	{0x7F, 0x49, 0x49, 0x49, 0x36},// B
	{0x3E, 0x41, 0x41, 0x41, 0x22},// C
	{0x7F, 0x41, 0x41, 0x22, 0x1C},// D
	{0x7F, 0x49, 0x49, 0x49, 0x41},// E
	{0x7F, 0x09, 0x09, 0x01, 0x01},// F
	{0x3E, 0x41, 0x41, 0x51, 0x32},// G
	{0x7F, 0x08, 0x08, 0x08, 0x7F},// H
	{0x00, 0x41, 0x7F, 0x41, 0x00},// I
	{0x20, 0x40, 0x41, 0x3F, 0x01},// J
	{0x7F, 0x08, 0x14, 0x22, 0x41},// K
	{0x7F, 0x40, 0x40, 0x40, 0x40},// L
	{0x7F, 0x02, 0x04, 0x02, 0x7F},// M
	{0x7F, 0x04, 0x08, 0x10, 0x7F},// N
	{0x3E, 0x41, 0x41, 0x41, 0x3E},// O
	{0x7F, 0x09, 0x09, 0x09, 0x06},// P
	{0x3E, 0x41, 0x51, 0x21, 0x5E},// Q
	{0x7F, 0x09, 0x19, 0x29, 0x46},// R
	{0x46, 0x49, 0x49, 0x49, 0x31},// S
	{0x01, 0x01, 0x7F, 0x01, 0x01},// T
	{0x3F, 0x40, 0x40, 0x40, 0x3F},// U
	{0x1F, 0x20, 0x40, 0x20, 0x1F},// V
	{0x7F, 0x20, 0x18, 0x20, 0x7F},// W
	{0x63, 0x14, 0x08, 0x14, 0x63},// X
	{0x03, 0x04, 0x78, 0x04, 0x03},// Y
	{0x61, 0x51, 0x49, 0x45, 0x43},// Z
	{0x00, 0x00, 0x7F, 0x41, 0x41},// [
	{0x02, 0x04, 0x08, 0x10, 0x20},// "\"
	{0x41, 0x41, 0x7F, 0x00, 0x00},// ]
	{0x04, 0x02, 0x01, 0x02, 0x04},// ^
	{0x40, 0x40, 0x40, 0x40, 0x40},// _
	{0x00, 0x01, 0x02, 0x04, 0x00},// `
	{0x20, 0x54, 0x54, 0x54, 0x78},// a
	{0x7F, 0x48, 0x44, 0x44, 0x38},// b
	{0x38, 0x44, 0x44, 0x44, 0x20},// c
	{0x38, 0x44, 0x44, 0x48, 0x7F},// d
	{0x38, 0x54, 0x54, 0x54, 0x18},// e
	{0x08, 0x7E, 0x09, 0x01, 0x02},// f
	{0x08, 0x14, 0x54, 0x54, 0x3C},// g
	{0x7F, 0x08, 0x04, 0x04, 0x78},// h
	{0x00, 0x44, 0x7D, 0x40, 0x00},// i
	{0x20, 0x40, 0x44, 0x3D, 0x00},// j
	{0x00, 0x7F, 0x10, 0x28, 0x44},// k
	{0x00, 0x41, 0x7F, 0x40, 0x00},// l
	{0x7C, 0x04, 0x18, 0x04, 0x78},// m
	{0x7C, 0x08, 0x04, 0x04, 0x78},// n
	{0x38, 0x44, 0x44, 0x44, 0x38},// o
	{0x7C, 0x14, 0x14, 0x14, 0x08},// p
	{0x08, 0x14, 0x14, 0x18, 0x7C},// q
	{0x7C, 0x08, 0x04, 0x04, 0x08},// r
	{0x48, 0x54, 0x54, 0x54, 0x20},// s
	{0x04, 0x3F, 0x44, 0x40, 0x20},// t
	{0x3C, 0x40, 0x40, 0x20, 0x7C},// u
	{0x1C, 0x20, 0x40, 0x20, 0x1C},// v
	{0x3C, 0x40, 0x30, 0x40, 0x3C},// w
	{0x44, 0x28, 0x10, 0x28, 0x44},// x
	{0x0C, 0x50, 0x50, 0x50, 0x3C},// y
	{0x44, 0x64, 0x54, 0x4C, 0x44},// z
	{0x00, 0x08, 0x36, 0x41, 0x00},// {
	{0x00, 0x00, 0x7F, 0x00, 0x00},// |
	{0x00, 0x41, 0x36, 0x08, 0x00},// }
	{0x02, 0x01, 0x02, 0x04, 0x02},// ~
	{0x08, 0x1C, 0x2A, 0x08, 0x08} // <-
};
void LCD_Commmand(uint8_t ControByte, uint8_t DataByte)
{
#if defined (I2C_SOFT_MODE)
	i2c_Start();
	i2c_Write(SSD1306_I2C_ADDRESS);
	i2c_Write(ControByte);
	i2c_Write(DataByte);
	i2c_Stop();
#elif defined (I2C_HARD_MODE)
	uint8_t buff[2];
	buff[0]=ControByte;
	buff[1]=DataByte;
	TWI_I2C_Write_Buff(SSD1306_I2C_ADDRESS, buff, 2, 1);
#endif
}
void LCD_Goto(uint8_t x, uint8_t y)
{
	LCD_X = x;
	LCD_Y = y;
	// send a bunch of data in one xmission
	LCD_Commmand(COMAND, 0xB0 + y);//set page address
	LCD_Commmand(COMAND, x & 0xf);//set lower column address
	LCD_Commmand(COMAND,0x10 | (x >> 4));//set higher column address
}
void LCD_Clear(void)
{
	memset(buffer_pointer, 0, 8192);
	LCD_UpdateImage(buffer_pointer);
}

void LCD_ClearLine(uint8_t line, uint8_t position, uint8_t symbols)  //1 3 1
{
	uint16_t i;
	position = position * 8;
  symbols = (symbols-1) * 8;
	
	for (i=0; i<=symbols; i++) 
	{
		LCD_Goto(i+position, line);
		LCD_Print(' ');
	}
}
void LCD_init(void)
{
	nrf_delay_ms(100);
	LCD_Commmand(COMAND,0xAE); //display off
	LCD_Commmand(COMAND,0x20); //Set Memory Addressing Mode   
	LCD_Commmand(COMAND,0x10); //00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	LCD_Commmand(COMAND,0xB0); //Set Page Start Address for Page Addressing Mode,0-7

	LCD_Commmand(COMAND,0xC0); //Set COM Output Scan Direction*********** for rotating (C8 - normal, C0 - 180 deg)
	
	LCD_Commmand(COMAND,0x00); //---set low column address
	LCD_Commmand(COMAND,0x10); //---set high column address
	LCD_Commmand(COMAND,0x40); //--set start line address
	LCD_Commmand(COMAND,0x81); //--set contrast control register
	LCD_Commmand(COMAND,0xFF);
	
	LCD_Commmand(COMAND,0xA0); //--set segment re-map 0 to 127************* for rotating (A1 - normal, A0 - 180 deg)
	
	LCD_Commmand(COMAND,0xA6); //--set normal display
	LCD_Commmand(COMAND,0xA8); //--set multiplex ratio(1 to 64)
	LCD_Commmand(COMAND,0x3F); //
	LCD_Commmand(COMAND,0xA4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	LCD_Commmand(COMAND,0xD3); //-set display offset
	LCD_Commmand(COMAND,0x00); //-not offset
	LCD_Commmand(COMAND,0xD5); //--set display clock divide ratio/oscillator frequency
	LCD_Commmand(COMAND,0xF0); //--set divide ratio
	LCD_Commmand(COMAND,0xD9); //--set pre-charge period
	LCD_Commmand(COMAND,0x22); //
	LCD_Commmand(COMAND,0xDA); //--set com pins hardware configuration
	LCD_Commmand(COMAND,0x12);
	LCD_Commmand(COMAND,0xDB); //--set vcomh
	LCD_Commmand(COMAND,0x20); //0x20,0.77xVcc
	LCD_Commmand(COMAND,0x8D); //--set DC-DC enable
	LCD_Commmand(COMAND,0x14); //
	LCD_Commmand(COMAND,0xAF); //--turn on SSD1306 panel
}
void LCD_PowerON(void) {
	LCD_Commmand(COMAND,0x8D);  
	LCD_Commmand(COMAND,0x14);  
	LCD_Commmand(COMAND,0xAF);  
}
void LCD_PowerOFF(void) {
	LCD_Commmand(COMAND,0x8D);  
	LCD_Commmand(COMAND,0x10);
	LCD_Commmand(COMAND,0xAE);  
}
void LCD_draw8x8(uint8_t * buffer, uint8_t x, uint8_t y)
{
	// send a bunch of data in one xmission
	LCD_Commmand(COMAND, 0xB0 + y);//set page address
	LCD_Commmand(COMAND, x & 0xf);//set lower column address
	LCD_Commmand(COMAND,0x10 | (x >> 4));//set higher column address
#if defined (I2C_SOFT_MODE)
	i2c_Start();
	i2c_Write(SSD1306_I2C_ADDRESS);
	i2c_Write(DATA);  
	for (x=0; x<8; x++)
	{
		i2c_Write(buffer[x]);
  }
	i2c_Stop();
#elif defined (I2C_HARD_MODE)
#endif
}
void LCD_Print(char c)
{
	LCD_draw8x8((uint8_t*)&Font5x8[c-32], LCD_X, LCD_Y);
	LCD_X += 8;
	if(LCD_X>SSD1306_LCDWIDTH)
	{
		LCD_X = SSD1306_DEFAULT_SPACE;
	}
}
void LCD_Printf(uint8_t line, uint8_t position, char* buf)
{
	LCD_Goto(position*8, line);
	while (*buf!= 0)
	{
		if((LCD_X>SSD1306_LCDWIDTH)||(LCD_X<5)){LCD_X=SSD1306_DEFAULT_SPACE;}
		LCD_Print(*buf++);
	}
}
/* LCD_DrawImage((uint8_t*)&Logo); */
void LCD_UpdateImage(uint8_t *buffer)
{
	uint16_t i;
	uint8_t x=0;
	uint8_t y=0;
	LCD_Goto(0,0);
	for (i=0; i<(SSD1306_LCDWIDTH*SSD1306_LCDHEIGHT/8); i++)
	{
		LCD_Commmand(DATA, buffer[i]);
		x++;
		if(x>=SSD1306_LCDWIDTH)
		{
			x =0;
			y++;
			LCD_Goto(0,y);
		}
	}
}
void LCD_DrawPixel(uint8_t x, uint8_t y, uint8_t *buffer)
{
		uint8_t which_bit, which_page, bit_mask, value = 0;
    uint16_t which_byte; 
    which_page = y / 8;             // There are 8 vertical pixels per page
    which_byte = x + (SSD1306_LCDWIDTH * which_page);
    which_bit = y % 8;
    bit_mask = (1<<which_bit);
    buffer[which_byte] = (buffer[which_byte] & ~bit_mask) | ((1<<which_bit) & bit_mask);
}
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t *buffer) {
	int16_t dx, dy, sx, sy, err, e2, i, tmp; 
	/* Check for overflow */
	if (x0 >= SSD1306_LCDWIDTH) {
		x0 = SSD1306_LCDWIDTH - 1;
	}
	if (x1 >= SSD1306_LCDWIDTH) {
		x1 = SSD1306_LCDWIDTH - 1;
	}
	if (y0 >= SSD1306_LCDHEIGHT) {
		y0 = SSD1306_LCDHEIGHT - 1;
	}
	if (y1 >= SSD1306_LCDHEIGHT) {
		y1 = SSD1306_LCDHEIGHT - 1;
	}
	dx = (x0 < x1) ? (x1 - x0) : (x0 - x1); 
	dy = (y0 < y1) ? (y1 - y0) : (y0 - y1); 
	sx = (x0 < x1) ? 1 : -1; 
	sy = (y0 < y1) ? 1 : -1; 
	err = ((dx > dy) ? dx : -dy) / 2; 
	if (dx == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}	
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}		
		/* Vertical line */
		for (i = y0; i <= y1; i++) {
			LCD_DrawPixel(x0, i, buffer);
		}		
		/* Return from function */
		return;
	}	
	if (dy == 0) {
		if (y1 < y0) {
			tmp = y1;
			y1 = y0;
			y0 = tmp;
		}		
		if (x1 < x0) {
			tmp = x1;
			x1 = x0;
			x0 = tmp;
		}	
		/* Horizontal line */
		for (i = x0; i <= x1; i++) {
			LCD_DrawPixel(i, y0, buffer);
		}		
		/* Return from function */
		return;
	}	
	while (1) {
		LCD_DrawPixel(x0, y0, buffer);
		if (x0 == x1 && y0 == y1) {
			break;
		}
		e2 = err; 
		if (e2 > -dx) {
			err -= dy;
			x0 += sx;
		} 
		if (e2 < dy) {
			err += dx;
			y0 += sy;
		} 
	}
}
void LCD_DrawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *buffer) {
	/* Check input parameters */
	if (
		x >= SSD1306_LCDWIDTH ||
		y >= SSD1306_LCDHEIGHT
	) {
		/* Return error */
		return;
	}
	
	/* Check width and height */
	if ((x + w) >= SSD1306_LCDWIDTH) {
		w = SSD1306_LCDWIDTH - x;
	}
	if ((y + h) >= SSD1306_LCDHEIGHT) {
		h = SSD1306_LCDHEIGHT - y;
	}
	
	/* Draw 4 lines */
	LCD_DrawLine(x, y, x + w, y, buffer);         /* Top line */
	LCD_DrawLine(x, y + h, x + w, y + h, buffer); /* Bottom line */
	LCD_DrawLine(x, y, x, y + h, buffer);         /* Left line */
	LCD_DrawLine(x + w, y, x + w, y + h, buffer); /* Right line */
}
void SSD1306_DrawCircle(int16_t x0, int16_t y0, int16_t r, uint8_t *buffer) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

    LCD_DrawPixel(x0, y0 + r, buffer);
    LCD_DrawPixel(x0, y0 - r, buffer);
    LCD_DrawPixel(x0 + r, y0, buffer);
    LCD_DrawPixel(x0 - r, y0, buffer);

    while (x < y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        LCD_DrawPixel(x0 + x, y0 + y, buffer);
        LCD_DrawPixel(x0 - x, y0 + y, buffer);
        LCD_DrawPixel(x0 + x, y0 - y, buffer);
        LCD_DrawPixel(x0 - x, y0 - y, buffer);

        LCD_DrawPixel(x0 + y, y0 + x, buffer);
        LCD_DrawPixel(x0 - y, y0 + x, buffer);
        LCD_DrawPixel(x0 + y, y0 - x, buffer);
        LCD_DrawPixel(x0 - y, y0 - x, buffer);
    }
}
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// SSD1306_Startscrollright(0x00, 0x0F)
void SSD1306_Startscrollright(uint8_t start, uint8_t stop)
{
  LCD_Commmand(DATA, SSD1306_RIGHT_HORIZONTAL_SCROLL);
  LCD_Commmand(DATA, 0X00);
  LCD_Commmand(DATA, start);
  LCD_Commmand(DATA, 0X00);
  LCD_Commmand(DATA, stop);
  LCD_Commmand(DATA, 0X00);
  LCD_Commmand(DATA, 0XFF);
  LCD_Commmand(DATA, SSD1306_ACTIVATE_SCROLL);
}
// Activate a right handed scroll for rows start through stop
// Hint, the display is 16 rows tall. To scroll the whole display, run:
// display.scrollright(0x00, 0x0F)
void SSD1306_Startscrollleft(uint8_t start, uint8_t stop)
{
  LCD_Commmand(DATA,SSD1306_LEFT_HORIZONTAL_SCROLL);
  LCD_Commmand(DATA,0X00);
  LCD_Commmand(DATA,start);
  LCD_Commmand(DATA,0X00);
  LCD_Commmand(DATA,stop);
  LCD_Commmand(DATA,0X00);
  LCD_Commmand(DATA,0XFF);
  LCD_Commmand(DATA,SSD1306_ACTIVATE_SCROLL);
}
void SSD1306_Stopscroll(void)
{
  LCD_Commmand(DATA, SSD1306_DEACTIVATE_SCROLL);
}
/**/