/*
 * cmd.h
 *
 *  Created on: Jan 9, 2015
 *      Author: Minh
 */

#ifndef USER_CMD_H_
#define USER_CMD_H_


#define CMD_TASK_QUEUE_SIZE 1
#define CMD_TASK_PRIO		0

typedef struct __attribute((__packed__)) {
	uint16_t len;
	uint8_t data;
} ARGS;

typedef struct __attribute((__packed__)) {
	uint16_t checksum;
	uint16_t cmd;
	uint32_t callback;
	uint16_t argc;
	ARGS args;
}PACKET_CMD;



void CMD_Init();
void CMD_Input(uint8_t data);



#endif /* USER_CMD_H_ */
