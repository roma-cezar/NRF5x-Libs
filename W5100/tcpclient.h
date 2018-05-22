#ifndef TCPCLIENT_H
#define TCPCLIENT_H


#include <stdint.h>
#include <stdbool.h>
#include "socket.h"
#include "w5100.h"

bool tcpclient_open(SOCKET s, uint8_t protocol, uint8_t* ip, uint16_t port);

bool tcpclient_close(SOCKET s);

bool tcpclient_send(SOCKET s, uint8_t *data);

bool tcpclient_sendbytes(SOCKET s, uint8_t *data, uint8_t size);

bool tcpclient_recivedata(SOCKET s, uint8_t *data, uint16_t len, void (*handler)(uint8_t*, uint8_t));

void process_tcpclient(SOCKET s);

void ArrayToStr(char *output, uint8_t *input, uint8_t len);
#endif /* TCPCLIENT_H */