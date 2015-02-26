/*
 * wifi.h
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */

#ifndef USER_WIFI_H_
#define USER_WIFI_H_
#include "os_type.h"
#include "cmd.h"
typedef void (*WifiCallback)(uint8_t);
uint32_t ICACHE_FLASH_ATTR
WIFI_Connect(PACKET_CMD *cmd);


#endif /* USER_WIFI_H_ */
