/*
 * cmd.h
 *
 *  Created on: Jan 9, 2015
 *      Author: Minh
 */

#ifndef USER_CMD_H_
#define USER_CMD_H_
#include "typedef.h"

#define CMD_TASK_QUEUE_SIZE 1
#define CMD_TASK_PRIO		0

typedef enum
{
	LOAD_CFG = 0,
	SAVE_CFG,
	RST,
	SEND,
	SCAN_WIFI,
	WEB_SERVER
}CMD;

typedef struct __attribute((__packed__)) {
	uint8_t cmd;
	uint16_t len;
	uint8_t data[];
}PACKET_TYPE;

void CMD_Init();
void CMD_Input(U8 data);
#endif /* USER_CMD_H_ */
