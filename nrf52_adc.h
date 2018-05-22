#ifndef nrf52_adc_H
#define nrf52_adc_H


#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define 	MAX_SAADC_SAMPLES 			1 
#define 	MAX_SAADC_CHANNELS 			1 // VDD, AIN0, AIN1
#define 	MAX_ADC_CNT							(MAX_SAADC_SAMPLES * MAX_SAADC_CHANNELS)

void SAADC_IRQHandler(void);
void SAADC_Config(void);
void SAADC_StartMeasure(void);
void SAADC_StopMeasure(void);
void SAADC_Disable(void);
#endif /* nrf52_adc_H */
