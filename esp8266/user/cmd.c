/*
 * cmd.c
 *
 *  Created on: Jan 9, 2015
 *      Author: Minh
 */
#include "user_interface.h"
#include "osapi.h"
#include "driver/uart.h"
#include "cmd.h"
#include "typedef.h"
#include "ringbuf.h"
#include "proto.h"
#include "config.h"
#include "debug.h"
#include "user_config.h"
#include "wifi.h"

os_event_t    cmdRecvQueue[CMD_TASK_QUEUE_SIZE];
RINGBUF rxRb;
PROTO_PARSER rxProto;
U8	rxBuf[256];
U8 	protoBuf[1024];

static void ICACHE_FLASH_ATTR
CMD_Task(os_event_t *events);

void CMD_Back(uint8_t cmd, uint8_t status)
{
	U8 backBuf[7];
	U8 i;
	backBuf[0] = 0x7E;
	backBuf[1] = cmd;
	backBuf[2] = 1;
	backBuf[3] = 0;
	backBuf[4] = status;
	backBuf[5] = 0x7F;
	for(i=0;i<6;i++)
		uart0_write_char(backBuf[i]);
}

void protoCompletedCb()
{
	uint16_t add;
	PACKET_TYPE *packet;
	U8  sendBuf[1024];
	packet = (PACKET_TYPE*)&protoBuf[0];
	os_memset(sendBuf, 0, sizeof(sendBuf));
	INFO("Packet call back %d,%d\r\n",packet->cmd,packet->len);

}

void ICACHE_FLASH_ATTR
CMD_Init()
{
	RINGBUF_Init(&rxRb, rxBuf, sizeof(rxBuf));
	PROTO_InitParser(&rxProto, protoCompletedCb, protoBuf, sizeof(protoBuf));

	system_os_task(CMD_Task, CMD_TASK_PRIO, cmdRecvQueue, CMD_TASK_QUEUE_SIZE);
	system_os_post(CMD_TASK_PRIO, 0, 0);
}

void ICACHE_FLASH_ATTR
CMD_Input(U8 data)
{
	RINGBUF_Put(&rxRb, data);
	system_os_post(CMD_TASK_PRIO, 0, 0);
}

static void ICACHE_FLASH_ATTR ///////
CMD_Task(os_event_t *events)
{
	U8 c;
	while(RINGBUF_Get(&rxRb, &c) == 0){
		PROTO_ParseByte(&rxProto, c);
		//os_printf("%X", c);
	}
}
