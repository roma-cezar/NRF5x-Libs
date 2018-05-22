
#include "nrf.h"
#include "nrf_gpio.h"      
#include "pcd8544.h" 
#include "nrf_delay.h"
#include "font6x8.h"

unsigned char lcd8544_buff[1]; 
/*********************************************************************************/
// Инициализация
void lcd8544_init(void) {

	//nrf_gpio_cfg_input(1, NRF_GPIO_PIN_NOPULL);  //MISO	
	nrf_gpio_cfg_output(LCD_RST_PIN);	//RST
	nrf_gpio_cfg_output(LCD_DC_PIN); 	//DC
	nrf_gpio_cfg_output(LCD_CSE_PIN); //CS
	nrf_gpio_cfg_output(LCD_SDA_PIN); //MOSI
	nrf_gpio_cfg_output(LCD_SCK_PIN); //SCK
	
	nrf_gpio_pin_set(LCD_RST_PIN);	//RST
	nrf_gpio_pin_set(LCD_DC_PIN); 	//DC
	nrf_gpio_pin_set(LCD_CSE_PIN); //CS
	nrf_gpio_pin_set(LCD_SDA_PIN); //MOSI
	nrf_gpio_pin_set(LCD_SCK_PIN); //SCK
	
	NRF_SPI0->POWER = SPI_POWER_POWER_Enabled;	
	NRF_SPI0->PSELMOSI = 0;
	NRF_SPI0->PSELMISO = 1;
	NRF_SPI0->PSELSCK = 2;
	NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_K125;
	NRF_SPI0->CONFIG = 	4;
	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Enabled;
	NRF_SPI0->EVENTS_READY = 0;
	
	LCD_CS0;            // CS=0  - начали сеанс работы с дисплеем
	LCD_DC0;			// A0=0  - переход в командный режим

	// сброс дисплея
	LCD_RST0;                   // RST=0
	nrf_delay_ms(1);				// пауза
	LCD_RST1;                   // RST=1
	nrf_delay_ms(LCD_RST_DLY);		// пауза

	// инициализация дисплея
	lcd8544_senddata(0x21);      // переход в расширенный режим
	lcd8544_senddata(0xC1);

	lcd8544_senddata(0x06);		// температурный коэффициент, от 4 до 7

	lcd8544_senddata(0x13);		// Bias 0b0001 0xxx - работает как контрастность

	lcd8544_senddata(0x20); 	// переход в обычный режим
	lcd8544_senddata(0x0C);	// 0b1100 - normal mode
		                        // 0b1101 - invert mode
		                        // 0b1001 - полностью засвеченный экран
		                        // 0b1000 - чистый экран
	LCD_CS1;
}
/*********************************************************************************/
void lcd8544_senddata(uint8_t data)
{
	uint8_t ret;
	if(NRF_SPI0->EVENTS_READY == 0)
	{
//		nrf_gpio_pin_clear(3);
		NRF_SPI0->TXD = (uint8_t)data;
		ret = NRF_SPI0->RXD;
	}
	while(NRF_SPI0->EVENTS_READY != 1);
	NRF_SPI0->EVENTS_READY = 0;
//	nrf_gpio_pin_set(3);
}
// очистка дисплея
void lcd8544_refresh(void) 
{
	LCD_CS0;            // СS=0 - начали сеанс работы с дисплеем

	LCD_DC0;            // передача комманд
	lcd8544_senddata(0x40); // установка курсора в позицию Y=0; X=0
	lcd8544_senddata(0x80);
	LCD_DC1;            // передача данных
	unsigned char y, x;
	for (y=0;y<6;y++) for (x=0;x<84;x++) lcd8544_senddata(lcd8544_buff[y*84+x]);
	
	LCD_CS1;			// СS=1 - закончили сеанс работы с дисплеем
}
/*********************************************************************************/
// очистка дисплея
void lcd8544_clear(void) 
{ 
	unsigned char x,y;
	for (y=0;y<6;y++) for(x=0;x<84;x++) lcd8544_buff[y*84+x]=0;
}
// вывод пиксела
void lcd8544_putpix(unsigned char x, unsigned char y, unsigned char mode) 
{
	if ((x>84) || (y>47)) return;

	unsigned int adr=(y>>3)*84+x;
	unsigned char data=(1<<(y&0x07));

	if (mode) lcd8544_buff[adr]|=data;
   else lcd8544_buff[adr]&=~data;
}
// процедура рисования линии
void lcd8544_line(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode) {
signed char   dx, dy, sx, sy;
unsigned char  x,  y, mdx, mdy, l;
	
  dx=x2-x1; dy=y2-y1;

  if (dx>=0) { mdx=dx; sx=1; } else { mdx=x1-x2; sx=-1; }
  if (dy>=0) { mdy=dy; sy=1; } else { mdy=y1-y2; sy=-1; }

  x=x1; y=y1;

  if (mdx>=mdy) {
     l=mdx;
     while (l>0) {
         if (dy>0) { y=y1+mdy*(x-x1)/mdx; }
            else { y=y1-mdy*(x-x1)/mdx; }
         lcd8544_putpix(x,y,mode);
         x=x+sx;
         l--;
     }
  } else {
     l=mdy;
     while (l>0) {
        if (dy>0) { x=x1+((mdx*(y-y1))/mdy); }
          else { x=x1+((mdx*(y1-y))/mdy); }
        lcd8544_putpix(x,y,mode);
        y=y+sy;
        l--;
     }
  }
  lcd8544_putpix(x2, y2, mode);

}
// рисование прямоугольника (не заполненного)
void lcd8544_rect(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char mode) {
	lcd8544_line(x1,y1, x2,y1, mode);
	lcd8544_line(x1,y2, x2,y2, mode);
	lcd8544_line(x1,y1, x1,y2, mode);
	lcd8544_line(x2,y1, x2,y2, mode);
}
// печать десятичного числа
void lcd8544_dec(unsigned int numb, unsigned char dcount, unsigned char x, unsigned char y, unsigned char mode) {
	unsigned int divid=10000;
	unsigned char i;

	for (i=5; i!=0; i--) {

		unsigned char res=numb/divid;

		if (i<=dcount) {
			lcd8544_putchar(x, y, res+'0', mode);
			x=x+6;
		}

		numb%=divid;
		divid/=10;
	}
}
// вывод символа на экран по координатам
void lcd8544_putchar(unsigned char px, unsigned char py, unsigned char ch, unsigned char mode) {
	const unsigned char *fontpointer;

	if (ch<127) {	// знакогенератор латиницы
		fontpointer=NewFontLAT; ch=ch-32; }
		else	{	// знакогенератор русских букв
		fontpointer=NewFontRUS; ch=ch-192;}

    unsigned char lcd_YP=7- (py & 0x07);    // битовая позиция символа в байте
    unsigned char lcd_YC=(py & 0xF8)>>3; 	// байтовая позиция символа на экране
    unsigned char x;
	for (x=0; x<6; x++) {

		unsigned char temp=*(fontpointer+ch*6+x);

		if (mode!=0) {
			temp=~temp;
			if (py>0) lcd8544_putpix(px, py-1, 1);	// если печать в режиме инверсии - сверху отчертим линию
		}

		temp&=0x7F;

		lcd8544_buff[lcd_YC*84+px]=lcd8544_buff[lcd_YC*84+px] | (temp<<(7-lcd_YP)); 	// печать верхней части символа

	    if (lcd_YP<7) lcd8544_buff[(lcd_YC+1)*84+px]=lcd8544_buff[(lcd_YC+1)*84+px] | (temp>>(lcd_YP+1)); 	// печать нижней части символа

		px++;
		if (px>83) return;
	}
}
// линия левее символа для печати в инверсе
void lcd8544_leftchline(unsigned char x, unsigned char y) {
	if (x>0) lcd8544_line(x-1, y-1, x-1, y+6, 1);
}
// вывод строки
void lcd8544_putstr(unsigned char x, unsigned char y, const unsigned char str[], unsigned char mode) {
	if (mode) lcd8544_leftchline(x, y);
	while (*str!=0) {
		lcd8544_putchar(x, y, *str, mode);
		x=x+6;
		str++;
	}
}
//