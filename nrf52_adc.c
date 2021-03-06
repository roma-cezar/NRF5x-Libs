#include "nrf52.h"
#include "nrf52_bitfields.h"
#include "nrf52_adc.h"


uint8_t SAADC_FLAG = 0;
uint8_t sample=0;
uint8_t ch=0;
bool SAADC_DATAREADY_FLAG=false;

// ��������� ����� ��� ��������� ������ �� ���� �������, ����� ����� ������������� ������� � �� N = ch * samples ���
typedef struct ArrayList
{
	uint16_t saadc_ch_buf[MAX_SAADC_CHANNELS];
} ArrayList_type;

ArrayList_type SamplesList[MAX_SAADC_SAMPLES];

uint16_t VBAT = 0;

void SAADC_IRQHandler(void)
{				
		// Event on start
		if(NRF_SAADC->EVENTS_STARTED == 1)
		{
			NRF_SAADC->EVENTS_STARTED = 0;	
			NRF_SAADC->RESULT.MAXCNT = MAX_ADC_CNT;
			NRF_SAADC->RESULT.PTR = (uint32_t)(&SamplesList);
			NRF_SAADC->TASKS_SAMPLE = 1;
		}		
		// Event on one sample done
		if(NRF_SAADC->EVENTS_DONE == 1)
		{
			NRF_SAADC->EVENTS_DONE=0;
			NRF_SAADC->TASKS_SAMPLE = 1;
		}	
		// Event on one conversation done
		if(NRF_SAADC->EVENTS_END == 1)
		{
			NRF_SAADC->EVENTS_END = 0;
			NRF_SAADC->TASKS_STOP = 1;
			VBAT = (uint16_t)((SamplesList[0].saadc_ch_buf[0] * 6 * 60) / 4096) ;
//			bat_level =(float)SamplesList[0].saadc_ch_buf[0] * 6 * 0.6f / 4096;	
//			CO_level = (float)SamplesList[0].saadc_ch_buf[1] * 6 * 0.6f / 4096; // MQ-7
//			CH_level = (float)SamplesList[0].saadc_ch_buf[2] * 6 * 0.6f / 4096; // MQ-4
			SAADC_FLAG=0;
			SAADC_DATAREADY_FLAG=true;			
		}
}

void SAADC_Config(void)
{
	NVIC_DisableIRQ(SAADC_IRQn);
  NVIC_ClearPendingIRQ(SAADC_IRQn);
	NVIC_SetPriority(SAADC_IRQn, 5);

		uint8_t anlog_channels[] =
	{
		SAADC_CH_PSELN_PSELN_VDD,
		SAADC_CH_PSELP_PSELP_AnalogInput0,
		SAADC_CH_PSELP_PSELP_AnalogInput1,
		SAADC_CH_PSELP_PSELP_AnalogInput2,
		SAADC_CH_PSELP_PSELP_AnalogInput3,
		SAADC_CH_PSELP_PSELP_AnalogInput4,
		SAADC_CH_PSELP_PSELP_AnalogInput5,
		SAADC_CH_PSELP_PSELP_AnalogInput6,
		SAADC_CH_PSELP_PSELP_AnalogInput7
	};
	
	NRF_SAADC->RESOLUTION = SAADC_RESOLUTION_VAL_12bit << SAADC_RESOLUTION_VAL_Pos;
	for(uint8_t ch=0; ch<MAX_SAADC_CHANNELS; ch++)
	{
		NRF_SAADC->CH[ch].PSELP = anlog_channels[ch] << SAADC_CH_PSELP_PSELP_Pos;
		NRF_SAADC->CH[ch].PSELN = SAADC_CH_PSELN_PSELN_NC << SAADC_CH_PSELN_PSELN_Pos;
		NRF_SAADC->CH[ch].CONFIG =(SAADC_CH_CONFIG_BURST_Disabled 	<< SAADC_CH_CONFIG_BURST_Pos) |
															(SAADC_CH_CONFIG_MODE_SE 					<< SAADC_CH_CONFIG_MODE_Pos) 	|
															(SAADC_CH_CONFIG_TACQ_40us				<< SAADC_CH_CONFIG_TACQ_Pos) 	| 
															(SAADC_CH_CONFIG_REFSEL_Internal 	<< SAADC_CH_CONFIG_REFSEL_Pos)|
															(SAADC_CH_CONFIG_GAIN_Gain1_6 		<< SAADC_CH_CONFIG_GAIN_Pos);
	}
	NRF_SAADC->OVERSAMPLE = SAADC_OVERSAMPLE_OVERSAMPLE_Bypass << SAADC_OVERSAMPLE_OVERSAMPLE_Pos;
	NRF_SAADC->SAMPLERATE = SAADC_SAMPLERATE_MODE_Task << SAADC_SAMPLERATE_MODE_Pos;
	
	NRF_SAADC->INTENSET = SAADC_INTENSET_STARTED_Enabled << SAADC_INTENSET_STARTED_Pos;
	//NRF_SAADC->INTENSET = SAADC_INTENSET_STOPPED_Enabled << SAADC_INTENSET_STOPPED_Pos;
	NRF_SAADC->INTENSET = SAADC_INTENSET_END_Enabled << SAADC_INTENSET_END_Pos;
  NRF_SAADC->INTENSET = SAADC_INTENSET_DONE_Enabled << SAADC_INTENSET_DONE_Pos;
	//NRF_SAADC->INTENSET = SAADC_INTENSET_RESULTDONE_Enabled << SAADC_INTENSET_RESULTDONE_Pos;
}

void SAADC_Enable(void)
{
	//Enable ADC
	NVIC_EnableIRQ(SAADC_IRQn);
	NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Enabled << SAADC_ENABLE_ENABLE_Pos);
}

void SAADC_Disable(void)
{
	// Disable ADC
	NVIC_DisableIRQ(SAADC_IRQn);
	NRF_SAADC->ENABLE = (SAADC_ENABLE_ENABLE_Disabled << SAADC_ENABLE_ENABLE_Pos);	
}


void SAADC_StartMeasure(void)
{
	if(SAADC_FLAG==0){
		NRF_SAADC->EVENTS_STARTED = 0;
		SAADC_FLAG=1;
    NRF_SAADC->TASKS_START = 1;
	}
}

void SAADC_StopMeasure(void)
{
	NRF_SAADC->TASKS_STOP = 1;
	while (NRF_SAADC->EVENTS_STOPPED == 0);
	NRF_SAADC->EVENTS_STOPPED = 0;
}