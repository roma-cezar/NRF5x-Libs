#ifndef RF24_H
#define RF24_H

#include <stdint.h>
#include <stdbool.h>

#if defined (NRF51822)

#elif defined (NRF52832)

#endif


#define POWER_OFF 			0 
#define POWER_ON  			1 

#define RX 			0 
#define TX 			1 

#define STOP 		0 
#define START 	1 

#define DEVICE_TYPE_GATEWAY			0xFF
#define DEVICE_TYPE_GERKON			0x00
#define DEVICE_TYPE_RELAY				0x01
#define DEVICE_TYPE_PROTECHKA		0x02

#define PIPE_TYPE_GERKON					0
#define PIPE_TYPE_RELAY						1
#define PIPE_TYPE_PROTECHKA				2

#define MASSAGE_TYPE_TELE   0x00 // telemerty
#define MASSAGE_TYPE_CMD    0xFF // command

#define CMD_BOND 						0x34 // 
#define CMD_OK 							0x33 // 
#define CMD_BOND_OK					0x32 // 


#if defined (GATEWAY)
	#define GATEWAY_ID					0x0A2A3A4A
#else
	#define DEVICE_ID						0xFFFFFFF1
#endif

#if defined (FREERTOS)
	typedef struct{
			char ucData[32];
	}RadioMessage;
#endif


typedef struct
{
	uint8_t  device_msg_type;
	uint8_t  device_type;
	uint32_t device_id;
	uint32_t gateway_id;
	uint16_t device_vbat;
	uint8_t  device_freq_n;
	uint8_t  device_lost_packs;
	uint8_t  device_data_len;
	uint8_t  device_data[21]; // 21 bytes
} device_load_t;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event);

bool radio_power(bool state);
bool radio_reset(void);
bool radio_config(bool state);
bool radio_set_channel(uint8_t ch);
bool radio_change_rx_channel(uint8_t ch);
bool radio_listen(uint8_t state);
bool radio_send_payload(device_load_t *device);
bool radio_send_bond(device_load_t *device);
bool radio_send_ok(device_load_t *device);
bool radio_send_bond_ok(device_load_t *device);

void radio_print_payload(void);

#if defined (FREERTOS)
	bool GatewayParseData(device_load_t *device, RadioMessage *msgtorx);
	bool GatewayParseCmd(device_load_t *device, RadioMessage *msgtorx);
#else 
	bool GatewayParseData(device_load_t *device);
	bool GatewayParseCmd(device_load_t *device);
#endif	

#endif /* RF24_H */