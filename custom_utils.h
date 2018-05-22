#ifndef CUSTOM_UTILS_H
#define CUSTOM_UTILS_H


#include <stdint.h>
#include <stdbool.h>

void GetIP(uint8_t *text, uint8_t *ip);
void GetSN(uint8_t *text, uint8_t *sn);
void GetGW(uint8_t *text, uint8_t *gw);
void GetHOST(uint8_t *text, uint8_t *hst);
void GetPORT(uint8_t *text, uint16_t *rp);
#endif /* CUSTOM_UTILS_H */