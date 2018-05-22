#include "nrf.h"
/**/

#define FLASH_PAGE_SIZE         ((uint32_t*)0x00000020)   /* FLASH Page Size 32 bit*/
#define FLASH_USER_START_ADDR   ((uint32_t*)0x0003FC00)   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ((uint32_t*)0x00040000)   /* End @ of user Flash area */

void flash_page_erase(uint32_t * page_address);
void flash_write(uint32_t *address, uint32_t *value, uint16_t size);
void flash_read(uint32_t *address, uint32_t *data, uint16_t size);
/**/