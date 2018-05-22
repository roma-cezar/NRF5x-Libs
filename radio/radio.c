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

#include "radio.h"
#include "FLASH.h"


#if defined (FREERTOS)
	#include "FreeRTOS.h"
	#include "queue.h"
	QueueHandle_t xQueueRF;
	RadioMessage myMessageToSend;
	RadioMessage myMessageToRecieve;
#endif
/****************************************************************************/
nrf_esb_payload_t 				rx_payload;
nrf_esb_payload_t        	tx_payload;
nrf_esb_config_t 					nrf_esb_config;
device_load_t 				    device_load;

bool device_send_flag = false;
bool device_bond_flag = false;
bool device_bond_enable = false;


bool rx_done_flag = false;
bool tx_done_flag = false;
bool tx_fail_flag = false;

uint8_t current_channel = 0;
uint8_t radio_channel_1 = 5;
uint8_t radio_channel_2 = 55;
uint8_t radio_channel_3 = 95;

#define TIMEOUT 10000UL
uint8_t timeout = 0;
/****************************************************************************/
void nrf_esb_event_handler(nrf_esb_evt_t const * p_event)
{
		__disable_irq();
		uint32_t err_code;
    switch (p_event->evt_id)
    {
        case NRF_ESB_EVENT_TX_SUCCESS:
					  timeout = TIMEOUT;
						while(timeout--){__NOP;}
						tx_done_flag = true;
						#ifdef GATEWAY
								//Serial_Print((uint8_t*)"DEBUG: TX SUCCESS EVENT\r\n");	
						#endif
            break;
        case NRF_ESB_EVENT_TX_FAILED:
					  timeout = TIMEOUT;
						while(timeout--){__NOP;}
						tx_fail_flag = true;	
						(void) nrf_esb_flush_tx();
						#ifdef GATEWAY
								//Serial_Print((uint8_t*)"DEBUG: TX FAILED EVENT\r\n");
						#endif	
            break;
        case NRF_ESB_EVENT_RX_RECEIVED:
						timeout = TIMEOUT;
						while(timeout--){__NOP;}
						if(nrf_esb_read_rx_payload(&rx_payload) == NRF_SUCCESS)
            {
                if(rx_payload.length > 0)
                {
										rx_done_flag = true;	
                    #if defined (FREERTOS)
												if( xQueueRF != 0  && uxQueueSpacesAvailable(xQueueRF) != 0 )
												{
														memcpy(myMessageToRecieve.ucData, rx_payload.data, 32);
														if( xQueueSendToBackFromISR( xQueueRF, &myMessageToRecieve, 0 ) != pdPASS )
														{
																Serial_Print("DEBUG: Radio Queue add failed!\r\n");
														}
														else
														{
																portYIELD();
														}
												}
												else
												{
														Serial_Print("DEBUG: Radio Queue no free space!\r\n");
												}
                    #endif
											
                }
								
								#ifdef GATEWAY												
										Serial_Print((uint8_t*)"DEBUG: RX SUCCESS EVENT\r\n");
								#endif	
            }
            break;
    }
}
/****************************************************************************/
bool radio_reset(void)
{
	bool success = false;
	uint32_t err_code;
	err_code = nrf_esb_disable();
	while(!nrf_esb_is_idle());
	if(err_code == NRF_SUCCESS) 
		success = true;
	else
		success = false;
	return success;
}
/****************************************************************************/
bool radio_power(bool state)
{
	if(state == POWER_ON)
		NRF_RADIO->POWER = 1;
	else if(state == POWER_OFF)
		NRF_RADIO->POWER = 0;
	else
		return false;
	
	return true;
}
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
bool radio_config(bool state)
{
	bool success = false;
	uint32_t err_code;
	
	
	NRF_RADIO->POWER = RADIO_POWER_POWER_Enabled << RADIO_POWER_POWER_Pos;
	if(!nrf_esb_is_idle()) // если не idle
		nrf_esb_disable(); // останавливаем
		while(!nrf_esb_is_idle());
	
	uint8_t base_addr_0[4] = {0xE7, 0xE7, 0xE7, 0xE7}; // gercons channel
	uint8_t base_addr_1[4] = {0xC2, 0xC2, 0xC2, 0xC2}; // 
	uint8_t addr_prefix[8] = {0xE7, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8 };

	nrf_esb_config_t nrf_esb_config         = NRF_ESB_DEFAULT_CONFIG;
	if(state == RX)
	{
		nrf_esb_config.mode             			= NRF_ESB_MODE_PRX;
	}
	else if(state == TX)
	{
		nrf_esb_config.mode             			= NRF_ESB_MODE_PTX;
	}
	//nrf_esb_config.payload_length						= 32;
	//nrf_esb_config.tx_mode									= NRF_ESB_TXMODE_MANUAL;
	nrf_esb_config.tx_output_power					= NRF_ESB_TX_POWER_4DBM;	
	nrf_esb_config.bitrate                  = NRF_ESB_BITRATE_250KBPS;
	nrf_esb_config.protocol                 = NRF_ESB_PROTOCOL_ESB_DPL;
	nrf_esb_config.crc	 										= NRF_ESB_CRC_16BIT;
	nrf_esb_config.event_handler            = nrf_esb_event_handler;
	nrf_esb_config.retransmit_count					= 6;
	nrf_esb_config.retransmit_delay					= 750;
	nrf_esb_config.selective_auto_ack       = false;

	if(nrf_esb_init(&nrf_esb_config) == NRF_SUCCESS)
	{
		nrf_esb_set_base_address_0(base_addr_0);
		nrf_esb_set_base_address_1(base_addr_1);
		nrf_esb_set_prefixes(addr_prefix, 8);
		success = true;
	}
	else {
		success = false;
	}
	nrf_delay_ms(1);
	return success;
}
/****************************************************************************/
bool radio_set_channel(uint8_t ch)
{		
	bool success = false;
	if(ch <= 125 && nrf_esb_set_rf_channel(ch) == NRF_SUCCESS) // 0 to 100
	{
		success = true;
	}
	else {
		success = false;
	}
	return success;
}
/****************************************************************************/
bool radio_change_rx_channel(uint8_t ch)
{
	radio_listen(STOP);
	//nrf_esb_suspend();
	radio_set_channel(ch);
	radio_listen(START);
}
/****************************************************************************/
bool radio_listen(uint8_t state)
{
	bool success = false;
	uint32_t err_code;
	if(state == START){
			err_code = nrf_esb_start_rx();
	}
	else if(state == STOP){
			err_code = nrf_esb_stop_rx();
	}
	if(err_code == NRF_SUCCESS) 
		success = true;
	else
		success = false;
	return success;
}
/****************************************************************************/
bool radio_send_payload(device_load_t *device)
{
	bool success        = 	false;
	uint8_t 	i 				= 	0;
	uint32_t 	err_code 	= 	0;
	
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
	tx_payload.length = 32;
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

	// Загружаем данные в fifo передатчика
	if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
	{
		success = true;
	}	
	else{
		success = false;
	} 
	return success;
}
/****************************************************************************/
bool radio_send_bond(device_load_t *device)
{
	bool success = false;
	uint32_t err_code;
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
	tx_payload.length = 32;
	tx_payload.data[0] = MASSAGE_TYPE_CMD;	// type	
	tx_payload.data[1] = device->device_type;	// type
	tx_payload.data[2] = (uint8_t)(device->device_id>>24);
	tx_payload.data[3] = (uint8_t)(device->device_id>>16);
	tx_payload.data[4] = (uint8_t)(device->device_id>>8);
	tx_payload.data[5] = (uint8_t)(device->device_id);
	tx_payload.data[6] = CMD_BOND;	// BOND	
	
	if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
	{
		success = true;
	}
	else{
		success = false;
	} 
	return success;	
}
/****************************************************************************/
#ifdef GATEWAY
bool radio_send_ok(device_load_t *device)
{
	bool success = false;
	uint32_t err_code;
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
	tx_payload.length = 32;
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
//		#ifdef DEBUG
//			Serial_Print((uint8_t*)"DEBUG: OK\r\n");
//		#endif
		success = true;
	}	
	return success;
}
#endif
/****************************************************************************/
#ifdef GATEWAY
bool radio_send_bond_ok(device_load_t *device)
{
	bool success = false;
	uint32_t err_code;
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
	tx_payload.length = 32;
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
	tx_payload.data[10] = CMD_BOND_OK;	// BOND OK	
	if(nrf_esb_write_payload(&tx_payload) == NRF_SUCCESS)
	{
//		#ifdef DEBUG
//			Serial_Print((uint8_t*)"DEBUG: PAIRING OK\r\n");
//		#endif
		success = true;
	}	
	return success;	
}
#endif
/****************************************************************************/
#ifdef GATEWAY
#if defined (FREERTOS)
	bool GatewayParseData(device_load_t *device, RadioMessage *msgtorx)
	{
		uint8_t device_number=0;
		bool success = false;
		uint8_t textbuff[128];
		uint8_t gerkon_state;

		uint32_t gateway_id;

		device->device_msg_type = msgtorx->ucData[0];
		device->device_type = msgtorx->ucData[1];
		device->device_id	= (((uint32_t)msgtorx->ucData[2])<<24) |
												(((uint32_t)msgtorx->ucData[3])<<16) |
												(((uint32_t)msgtorx->ucData[4])<<8)  |
												(((uint32_t)msgtorx->ucData[5]));

		// считаем ID шлюза, к которому обращается устройство
		gateway_id	= (((uint32_t)msgtorx->ucData[6])<<24) |
									(((uint32_t)msgtorx->ucData[7])<<16) |
									(((uint32_t)msgtorx->ucData[8])<<8)  |
									(((uint32_t)msgtorx->ucData[9]));
		// если устройство привязано к нам
		if(gateway_id == GATEWAY_ID)
		{
				if(device->device_type == DEVICE_TYPE_GERKON) // Gerkon type
				{
					device->device_vbat = (((uint16_t)(msgtorx->ucData[10]))<<8) | ((uint16_t)(msgtorx->ucData[11]));
					device->device_freq_n = msgtorx->ucData[12];
					device->device_lost_packs = msgtorx->ucData[13];
					device->device_data_len = msgtorx->ucData[14];
					memcpy(device->device_data, &msgtorx->ucData[15], device->device_data_len);
					
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
					#ifdef DEBUG
						Serial_Print((uint8_t*)"DEBUG: Unknown device ID!\r\n");
					#endif
					success = false;
				}
		}
		else
		{
			#ifdef DEBUG
				Serial_Print((uint8_t*)"DEBUG: Unknown gateway's message!\r\n");
			#endif
			success = false;
		}

		return success;
	}
#else 
	bool GatewayParseData(device_load_t *device)
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
					#ifdef DEBUG
						Serial_Print((uint8_t*)"DEBUG: Unknown device ID!\r\n");
					#endif
					success = false;
				}
		}
		else
		{
			#ifdef DEBUG
				Serial_Print((uint8_t*)"DEBUG: Unknown gateway's message!\r\n");
			#endif
			success = false;
		}

		return success;
	}
#endif
#endif	
	
/****************************************************************************/
#if defined (FREERTOS)
bool GatewayParseCmd(device_load_t *device, RadioMessage *msgtorx)
{
	bool success = false;
	uint8_t textbuff[128];
	
	device->device_msg_type = msgtorx->ucData[0];
	device->device_type = msgtorx->ucData[1];
	device->device_id	= (((uint32_t)msgtorx->ucData[2])<<24) |
											(((uint32_t)msgtorx->ucData[3])<<16) |
											(((uint32_t)msgtorx->ucData[4])<<8)  |
											(((uint32_t)msgtorx->ucData[5]));
	

	if(device->device_type == DEVICE_TYPE_GERKON) // Gerkon type
	{
		#ifdef DEBUG
			Serial_Print((uint8_t*)"DEBUG: Add new device\r\n");
		#endif
		sprintf((char*)textbuff, "BOND,TYPE:GERKON,ID:0x%.8X,\r\n", device->device_id);
		Serial_Print(textbuff);
		memset(textbuff, 0, 128);
		success = true;
	}
	else
	{
		#ifdef DEBUG
			Serial_Print((uint8_t*)"DEBUG: Unknown device type!\r\n");
		#endif
		success = false;
	}

	return success;	
}	
#else 
bool GatewayParseCmd(device_load_t *device)
{
	bool success = false;
	uint8_t textbuff[128];
	
	device->device_msg_type = rx_payload.data[0];
	device->device_type = rx_payload.data[1];
	device->device_id	= (((uint32_t)rx_payload.data[2])<<24) |
											(((uint32_t)rx_payload.data[3])<<16) |
											(((uint32_t)rx_payload.data[4])<<8)  |
											(((uint32_t)rx_payload.data[5]));
	

	if(device->device_type == DEVICE_TYPE_GERKON) // Gerkon type
	{
		#ifdef DEBUG
			Serial_Print((uint8_t*)"DEBUG: Add new device\r\n");
		#endif
		sprintf((char*)textbuff, "BOND,TYPE:GERKON,ID:0x%.8X,\r\n", device->device_id);
		Serial_Print(textbuff);
		memset(textbuff, 0, 128);
		success = true;
	}
	else
	{
		#ifdef DEBUG
			Serial_Print((uint8_t*)"DEBUG: Unknown device type!\r\n");
		#endif
		success = false;
	}

	return success;	
}
#endif
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
/****************************************************************************/
void radio_print_payload(void)
{
	uint8_t textbuff[128];
	memset(textbuff, 0, 128);
	sprintf(textbuff, "Pipe: %d\r\nRSSI: -%d dBm\r\nRX Bytes: %d\r\n", rx_payload.pipe, rx_payload.rssi, rx_payload.length);
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