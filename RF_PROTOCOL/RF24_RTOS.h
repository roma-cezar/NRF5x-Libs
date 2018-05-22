#ifndef RF24_H
#define RF24_H


#include <stdint.h>
#include <stdbool.h>

#if defined (NRF51822)
#elif defined (NRF52832)
#endif

#define STATE_RX 						0 
#define STATE_TX 						1 

#define STATE_STOP 					0 
#define STATE_START 				1 

#define CMD_BOND 						0x34 // dialog completed
#define CMD_OK 							0x33 // dialog completed
#define CMD_SET_FREQ 				0x32 // set new frequancy channel
#define CMD_RES_LOST 				0x31 // reset lost packet counter
#define CMD_SCAN 						0x30 // reboot device
#define CMD_REBOOT 					0x29 // reboot device

#define MASSAGE_TYPE_TELE   0x00 // telemerty
#define MASSAGE_TYPE_CMD    0xFF // command

#define PIPE_TYPE_GERKON	0
#define PIPE_TYPE_RELAY		1

#define DEVICE_TYPE_GATEWAY	0xFF
#define DEVICE_TYPE_GERKON	0
#define DEVICE_TYPE_RELAY		1

#define WAIT  for(uint16_t i=0; i<1024; i++)


#define  DEVICE_LIST_SIZE  6

typedef struct 
{
    char ucData[32];
}RadioMessage;

typedef struct
{
	uint8_t  device_msg_type;
	uint8_t  device_type;
	uint32_t device_id;
	uint16_t device_vbat;
	uint8_t  device_freq_n;
	uint8_t  device_lost_packs;
	uint8_t  device_data_len;
	uint8_t  device_data[21]; // 21 bytes
} rf24_device_load_t;

void nrf_esb_event_handler(nrf_esb_evt_t const * p_event);

//void RF24_ScanRSSI(uint8_t ch);
//bool RF24_ChangeChannel(rf24_device_load_t *device);

bool RF24_SetChannell(uint8_t ch);
bool RF24_GatewayProcess(void);
bool RF24_GatewayParseData(rf24_device_load_t *device);
bool RF24_DeviceProcess(rf24_device_load_t *device);
bool RF24_Init(uint8_t state);
bool RF24_Listen(uint8_t state);
bool RF24_SendMessage(rf24_device_load_t *device);
bool RF24_SendOk(rf24_device_load_t *device);
bool RF24_SendBond(rf24_device_load_t *device);
void RF24_DebugPrintPayload(void);
bool IsDeviceInList(uint32_t device_id, uint8_t list_size);
uint8_t IsDeviceNumber(uint32_t device_id, uint8_t list_size);
#endif /* RF24_H */