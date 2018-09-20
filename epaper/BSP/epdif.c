/**
 *  @filename   :   epdif.c
 *  @brief      :   Implements EPD interface functions
 *                  Users have to implement all the functions in epdif.cpp
 *  @author     :   Yehui from Waveshare
 *
 *  Copyright (C) Waveshare     July 7 2017
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "epdif.h"
#include "NRF5x_SPI.h"
#if defined  (NRF51)
	#include "nrf.h"
#elif defined  (NRF52)	
	#include "nrf52.h"
	#include "nrf52_bitfields.h"
#endif

#include "nrf_gpio.h"
#include "nrf_delay.h"

void EpdDigitalWriteCallback(int pin, int value) {
  if (value == HIGH) {
    nrf_gpio_pin_set(pin);
  } else {
    nrf_gpio_pin_clear(pin);
  }
}

int EpdDigitalReadCallback(int pin) {
  if (nrf_gpio_pin_read(pin) == 1) {
    return HIGH;
  } else {
    return LOW;
  }
}

void EpdDelayMsCallback(unsigned int delaytime) {
  nrf_delay_ms(delaytime);
}

void EpdSpiTransferCallback(unsigned char data) {
   CS_SELECT; 
	if(NRF_SPI0->EVENTS_READY == 0)
	{
		NRF_SPI0->TXD = (uint8_t)data;
	}
	while(NRF_SPI0->EVENTS_READY != 1);
	uint8_t ret = (uint8_t)(NRF_SPI0->RXD);
	NRF_SPI0->EVENTS_READY = 0;
	CS_DESELECT;
}


