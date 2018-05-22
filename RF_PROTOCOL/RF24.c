#if defined (NRF51822)
	#include "nrf51.h"
	#include "nrf51_bitfields.h"
#elif defined (NRF52832)
	#include "nrf52.h"
	#include "nrf52_bitfields.h"
#endif

#include "sdk_common.h"
#include "nrf.h"
#include "nrf_esb_error_codes.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_error.h"
#include "boards.h"
#include "nrf_esb.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "Serial.h"

#include "RF24.h"
#include "FLASH.h"

nrf_esb_payload_t 				rx_payload;
nrf_esb_payload_t        	tx_payload;
nrf_esb_config_t 					nrf_esb_config;

rf24_device_load_t 				rf24_device_load;

uint8_t device_lost_packets = 0;
uint8_t retry_tx_counter =0;



uint32_t GATEWAY_ID = 0x8A2CEF10; //
uint32_t DEVICE_ID  = 0x010000FF; //
uint32_t mem_gateway_id = 0;


bool device_send_flag = false;
bool device_bond_flag = false;
bool rx_done_flag = false;
bool tx_done_flag = false;
bool tx_fail_flag = false;

bool rx_mode = false;
bool tx_mode = false;
bool rx_timeout = false;

uint16_t ok_counter=0;

uint8_t current_channel = 0;
uint8_t radio_channel_1 = 5;
uint8_t radio_channel_2 = 55;
uint8_t radio_channel_3 = 95;
/****************************************************************************/
void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
		uint32_t err_code;
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
						tx_done_flag = true;
						WAIT;
						//Serial_Print((uint8_t*)"TX SUCCESS EVENT\r\n");	
            break;
        case NRF_ESB_EVENT_TX_FAILED:
						tx_fail_flag = true;
						WAIT;
						//Serial_Print((uint8_t*)"TX FAILED EVENT\r\n");
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
						rx_done_flag = true;
						WAIT;
						//Serial_Print((uint8_t*)"RX RECEIVED EVENT\r\n");
            break;
    }
}
/****************************************************************************/
bool RF24_Init(uint8_t state)
{
	bool success = false;
	uint32_t err_code;
	
	uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7}; // gercons channel
	uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2}; // 
	uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };
	
	if(state == RX){
		nrf_esb_config.mode             			= NRF_ESB_MODE_PRX;
	}
	else if(state == TX){
		nrf_esb_config.mode             			= NRF_ESB_MODE_PTX;
	}
	nrf_esb_config.payload_length						= 32;
	nrf_esb_config.tx_output_power					= NRF_ESB_TX_POWER_4DBM;	
	nrf_esb_config.tx_mode									= NRF_ESB_TXMODE_MANUAL;
	nrf_esb_config.radio_irq_priority     	= 1;                                
	nrf_esb_config.event_irq_priority     	= 2;
	nrf_esb_config.event_handler            = nrf_esb_event_handler;
	nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_250KBPS;
	nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB;
	nrf_esb_config.crc	 										= NRF_ESB_CRC_16BIT;
	nrf_esb_config.retransmit_count					= 3;
	nrf_esb_config.retransmit_delay					= 750;
	nrf_esb_config.selective_auto_ack       = true;

	if(nrf_esb_init(&nrf_esb_config) == NRF_SUCCESS)
	{
		err_code = nrf_esb_set_base_address_0(base_addr_0);
		VERIFY_SUCCESS(err_code);
		err_code = nrf_esb_set_base_address_1(base_addr_1);
		VERIFY_SUCCESS(err_code);
		err_code = nrf_esb_set_prefixes(addr_prefix, 8);
		VERIFY_SUCCESS(err_code);
		success = true;
	}
	else {
		success = false;
	}
	return success;
}
/****************************************************************************/
bool RF24_SetChannell(uint8_t ch)
{		
	bool success = false;
	if(nrf_esb_set_rf_channel(ch) == NRF_SUCCESS) // 0 to 100
	{
		success = true;
	}
	else {
		success = false;
	}
	return success;
}
/**/
bool RF24_Listen(uint8_t state)
{
	bool success = false;
	uint32_t err_code;
	if(state == START){
			err_code = nrf_esb_start_rx();
			VERIFY_SUCCESS(err_code);
	}
	else if(state == STOP){
			err_code = nrf_esb_stop_rx();
			VERIFY_SUCCESS(err_code);
	}
	if(err_code == NRF_SUCCESS) 
		success = true;
	else
		success = false;
	return success;
}
/****************************************************************************/
/*********************************GERKON*************************************/
/****************************************************************************/
bool RF24_GatewayProcess(void)
{
	bool success = false;
	uint32_t err_code;
	if(tx_done_flag)
	{
		tx_done_flag = false;
		success = true;
		nrf_gpio_pin_toggle(LED_1);
		RF24_Init(RX);
		RF24_Listen(START);
		NRF_TIMER0->TASKS_START = 1; // Запускаем таймер
	}
	if(tx_fail_flag)
	{
		tx_fail_flag = false;
		success = false;
		nrf_esb_flush_tx();
		RF24_Init(RX);
		RF24_Listen(START);
		NRF_TIMER0->TASKS_START = 1; // Запускаем таймер
	}
	if(rx_done_flag)
	{
		rx_done_flag = false;
		nrf_gpio_pin_toggle(LED_1);
		NRF_TIMER0->TASKS_STOP  = 1; // Останавливаем таймер
		RF24_Listen(STOP);
		if (nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
		{
			nrf_esb_flush_rx();
			if(rx_payload.pipe == PIPE_TYPE_GERKON)
			{
					//RF24_DebugPrintPayload();
					if(RF24_GatewayParseData(&rf24_device_load))
					{
						if(RF24_Init(TX)){
							Serial_Print((uint8_t*)"\r\nOK\r\n");
							RF24_SendOk(&rf24_device_load);
						}
					}
					else
					{
						Serial_Print((uint8_t*)"\r\nStart Listen!\r\n");
						RF24_Listen(START);
						NRF_TIMER0->TASKS_START = 1; // Запускаем таймер
					}
			}
		}
	}
	return success;
}
/****************************************************************************/
bool RF24_SendMessage(rf24_device_load_t *device)
{
	bool success = false;
	uint8_t i;
	uint32_t err_code;
	tx_payload.length = 32;
	switch(device->device_type)
	{
		case DEVICE_TYPE_GERKON:
			tx_payload.pipe = PIPE_TYPE_GERKON;
			break;
		case DEVICE_TYPE_RELAY:
			tx_payload.pipe = PIPE_TYPE_RELAY;
			break;
		default:
			success = false;
			break;
	}
	tx_payload.noack = false;
	//memset(tx_payload.data, 0, 32);
	tx_payload.data[0] = device->device_msg_type;	// message type	
	tx_payload.data[1] = device->device_type;	// device type	
	// Device Id
	tx_payload.data[2] = (uint8_t)(device->device_id>>24);
	tx_payload.data[3] = (uint8_t)(device->device_id>>16);
	tx_payload.data[4] = (uint8_t)(device->device_id>>8);
	tx_payload.data[5] = (uint8_t)(device->device_id);
	// Bonded Gateway Id
	tx_payload.data[6] = (uint8_t)(device->gateway_id>>24);
	tx_payload.data[7] = (uint8_t)(device->gateway_id>>16);
	tx_payload.data[8] = (uint8_t)(device->gateway_id>>8);
	tx_payload.data[9] = (uint8_t)(device->gateway_id);
	// Device Vbat
	tx_payload.data[10] = (uint8_t)((device->device_vbat)>>8);	
	tx_payload.data[11] = (uint8_t)(device->device_vbat);
	// Current freq
	tx_payload.data[12] = device->device_freq_n;
	// Lost packet counter
	tx_payload.data[13] = device->device_lost_packs;
	// Device data length
	tx_payload.data[14] = device->device_data_len;
	memcpy(&tx_payload.data[15], device->device_data, device->device_data_len);
	if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
	{
		success = true;
		err_code = nrf_esb_start_tx();
		VERIFY_SUCCESS(err_code);
	}	
	else{
		success = false;
	} 
	return success;
}
/****************************************************************************/
/*********************************GATEWAY************************************/
/****************************************************************************/
bool RF24_SendOk(rf24_device_load_t *device)
{
	bool success = false;
	uint32_t err_code;
	tx_payload.length = 32;
	switch(device->device_type)
	{
		case DEVICE_TYPE_GERKON:
			tx_payload.pipe = PIPE_TYPE_GERKON;
			break;
		case DEVICE_TYPE_RELAY:
			tx_payload.pipe = PIPE_TYPE_RELAY;
			break;
		default:
			success = false;
			break;
	}
	tx_payload.noack = false;
	tx_payload.data[0] = MASSAGE_TYPE_CMD;	// type	
	tx_payload.data[1] = device->device_type;	// type	
	tx_payload.data[2] = (uint8_t)(device->device_id>>24);
	tx_payload.data[3] = (uint8_t)(device->device_id>>16);
	tx_payload.data[4] = (uint8_t)(device->device_id>>8);
	tx_payload.data[5] = (uint8_t)(device->device_id);
	
	tx_payload.data[6] = (uint8_t)(GATEWAY_ID>>24);
	tx_payload.data[7] = (uint8_t)(GATEWAY_ID>>16);
	tx_payload.data[8] = (uint8_t)(GATEWAY_ID>>8);
	tx_payload.data[9] = (uint8_t)(GATEWAY_ID);
	
	tx_payload.data[10] = CMD_OK;	// OK	
	if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
	{
		err_code = nrf_esb_start_tx();
		VERIFY_SUCCESS(err_code);
	}	
	return success;
}
/****************************************************************************/
bool RF24_SendBond(rf24_device_load_t *device)
{
	bool success = false;
	uint32_t err_code;
	tx_payload.length = 32;
	switch(device->device_type)
	{
		case DEVICE_TYPE_GERKON:
			tx_payload.pipe = PIPE_TYPE_GERKON;
			break;
		case DEVICE_TYPE_RELAY:
			tx_payload.pipe = PIPE_TYPE_RELAY;
			break;
		default:
			success = false;
			break;
	}
	tx_payload.noack = false;
	tx_payload.data[0] = MASSAGE_TYPE_CMD;	// type	
	tx_payload.data[1] = device->device_type;	// type
	tx_payload.data[2] = (uint8_t)(device->device_id>>24);
	tx_payload.data[3] = (uint8_t)(device->device_id>>16);
	tx_payload.data[4] = (uint8_t)(device->device_id>>8);
	tx_payload.data[5] = (uint8_t)(device->device_id);
				
	tx_payload.data[6] = CMD_BOND;	// BOND	
	
	if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
	{
		err_code = nrf_esb_start_tx();
		VERIFY_SUCCESS(err_code);
	}	
	return success;	
}
/****************************************************************************/
bool RF24_GatewayParseData(rf24_device_load_t *device)
{
	uint8_t device_number=0;
	bool success = false;
	uint8_t textbuff[128];
	uint8_t gerkon_state;

	uint32_t gateway_id;
	
	device->device_msg_type = rx_payload.data[0];
	device->device_type = rx_payload.data[1];
	device->device_id	= (((uint32_t)rx_payload.data[2])<<24) |
											(((uint32_t)rx_payload.data[3])<<16) |
											(((uint32_t)rx_payload.data[4])<<8)  |
											(((uint32_t)rx_payload.data[5]));
	
	if(device->device_msg_type == MASSAGE_TYPE_TELE)
	{
			// считаем ID шлюза, к которому обращается устройство
			gateway_id	= (((uint32_t)rx_payload.data[6])<<24) |
										(((uint32_t)rx_payload.data[7])<<16) |
										(((uint32_t)rx_payload.data[8])<<8)  |
										(((uint32_t)rx_payload.data[9]));
			// если устройство привязано к нам
			if(gateway_id == GATEWAY_ID)
			{
					if(device->device_type == DEVICE_TYPE_GERKON) // Gerkon type
					{
						device->device_vbat = (((uint16_t)(rx_payload.data[10]))<<8) | ((uint16_t)(rx_payload.data[11]));
						device->device_freq_n = rx_payload.data[12];
						device->device_lost_packs = rx_payload.data[13];
						device->device_data_len = rx_payload.data[14];
						memcpy(device->device_data, &rx_payload.data[15], device->device_data_len);
						
						gerkon_state = device->device_data[0];
						memset(textbuff, 0, 128);
						sprintf((char*)textbuff, "TELE,TYPE:GERKON,ID:0x%.8X,", device->device_id);
						sprintf((char*)textbuff + strlen((char*)textbuff), "VBAT:%d,STATE:%d,", device->device_vbat, gerkon_state);
						sprintf((char*)textbuff + strlen((char*)textbuff), "FREQ:%d,LOST:%d,\r\n", device->device_freq_n, device->device_lost_packs);
						Serial_Print(textbuff);	
						success = true;
					}
					else
					{
						Serial_Print((uint8_t*)"\r\nUnknown sensor type!\r\n");
						success = false;
					}
			}
			else
			{
				Serial_Print((uint8_t*)"Unknown's gateway message!\r\n");
				success = false;
			}
	}
	else if(device->device_msg_type == MASSAGE_TYPE_CMD && rx_payload.data[6] == CMD_BOND)
	{
		NRF_TIMER0->TASKS_STOP = 1;
		nrf_gpio_pin_set(LED_0);
		nrf_delay_ms(200);
		nrf_gpio_pin_clear(LED_0);
		nrf_delay_ms(100);
		nrf_gpio_pin_set(LED_0);
		nrf_delay_ms(200);
		nrf_gpio_pin_clear(LED_0);
		NRF_TIMER0->TASKS_START = 1;
		
		if(device->device_type == DEVICE_TYPE_GERKON) // Gerkon type
		{
			Serial_Print((uint8_t*)"\r\nAdd new device\r\n");
			sprintf((char*)textbuff, "BOND,TYPE:GERKON,ID:0x%.8X,\r\n", device->device_id);
			Serial_Print(textbuff);
			memset(textbuff, 0, 128);
			success = true;
		}
	}
	else
	{
		Serial_Print((uint8_t*)"Unknown cmd!\r\n");
		success = false;
	}
	return success;
}
/****************************************************************************/
/****************************TEST FUNCTIONS**********************************/
/****************************************************************************/
bool RF24_ChangeChannel(uint8_t new_ch)
{
		if(RF24_Listen(STOP))
		{
			//Serial_Print((uint8_t*)"Listen stopted!\r\n");
			if(RF24_Init(RX))
			{
				//Serial_Print((uint8_t*)"RX mode success!\r\n");
				if(RF24_SetChannell(new_ch))
				{
					//Serial_Print((uint8_t*)"Channel changed successfuly!\r\n");
					if(RF24_Listen(START))
					{
						//Serial_Print((uint8_t*)"Listen new channel started!\r\n");
					}
				}
			}
		}
}
/****************************************************************************/
void RF24_ScanRSSI(void)
{
	uint32_t err_code;
	uint8_t i = 0;
	int16_t	j =0;
	uint8_t textbuff[32];
	int16_t rssi; // MAX_CHANNEL = 0...100
	int16_t rssi_mem = 0;
	uint32_t timeout = 30000;
	
	RF24_Listen(STOP);
	
	uint8_t base_addr_0[4] = {0xFF, 0xFF, 0xFF, 0xFF}; 
	uint8_t base_addr_1[4] = {0xFF, 0xFF, 0xFF, 0xFF}; 
	uint8_t addr_prefix[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
	
	nrf_esb_config.payload_length						= 32;
	nrf_esb_config.mode             				= NRF_ESB_MODE_PRX;
	nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_1MBPS;
	nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB;
	nrf_esb_config.crc	 										= NRF_ESB_CRC_OFF;
	nrf_esb_config.selective_auto_ack       = false;
	
	if(nrf_esb_init(&nrf_esb_config) == NRF_SUCCESS)
	{
		err_code = nrf_esb_set_base_address_0(base_addr_0);
		err_code = nrf_esb_set_base_address_1(base_addr_1);
		err_code = nrf_esb_set_prefixes(addr_prefix, 8);
	}

	for(i=0; i<101; i++)
	{
		RF24_SetChannell(i);
		RF24_Listen(START);
		
		rssi = 0;
		
		while( NRF_RADIO->EVENTS_PAYLOAD == 0);
		NRF_RADIO->TASKS_RSSISTART = 1;
		while(NRF_RADIO->EVENTS_RSSIEND == 0);
		NRF_RADIO->TASKS_RSSISTOP = 1;
		
		rssi = (int16_t)(NRF_RADIO->RSSISAMPLE);

		sprintf((char*)textbuff, "FREE CHANNEL: %.3d, RSSI: %.3d\r\n", i, rssi);
		Serial_Print(textbuff);
		memset(textbuff, 0, 32);

		RF24_Listen(STOP);
	}
			
	Serial_Print("Done!\r\n");
		

	
	if(RF24_Init(RX))
	{
		if(RF24_SetChannell(14))
		{
			if(RF24_Listen(START))
			{
				Serial_Print((uint8_t*)"\r\nListen started\r\n");
			}
		}
	}
}
/****************************************************************************/
void RF24_DebugPrintPayload(void)
{
	uint8_t textbuff[128];
	memset(textbuff, 0, 128);
	sprintf(textbuff, "Pipe: %d\r\nRSSI: -%d dBm\r\n", rx_payload.pipe, rx_payload.rssi);
	Serial_Print(textbuff);
	memset(textbuff, 0, 128);
	sprintf(textbuff, "Data: 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", 
										rx_payload.data[0], rx_payload.data[1], 
										rx_payload.data[2], rx_payload.data[3], 
										rx_payload.data[4], rx_payload.data[5], 
										rx_payload.data[6], rx_payload.data[7], 
										rx_payload.data[8], rx_payload.data[9], 
										rx_payload.data[10],rx_payload.data[11], 
										rx_payload.data[12],rx_payload.data[13], 
										rx_payload.data[14],rx_payload.data[15]);
	Serial_Print(textbuff);
	memset(textbuff, 0, 128);
	Serial_Print((uint8_t*)"\r\n");
	sprintf(textbuff, "      0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x", 
										rx_payload.data[16], rx_payload.data[17], 
										rx_payload.data[18], rx_payload.data[19], 
										rx_payload.data[20], rx_payload.data[21], 
										rx_payload.data[22], rx_payload.data[23], 
										rx_payload.data[24], rx_payload.data[25], 
										rx_payload.data[26], rx_payload.data[27], 
										rx_payload.data[28], rx_payload.data[29], 
										rx_payload.data[30], rx_payload.data[31]);
	Serial_Print(textbuff);
	memset(textbuff, 0, 128);
	Serial_Print((uint8_t*)"\r\n");
}
/****************************************************************************/