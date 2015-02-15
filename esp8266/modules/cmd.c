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
#include "user_config.h"
#include "wifi.h"
#include "mqtt.h"
#include "ringbuf.h"
#include "proto.h"
#include "debug.h"
typedef enum
{
	CMD_NULL = 0,
	CMD_WIFI_CONNECT,
	CMD_READ_MEM
}CMD_NAME;

typedef uint32_t (*cmdfunc_t)(PACKET_CMD *cmd);

typedef struct {
	CMD_NAME  	sc_name;
	cmdfunc_t	sc_function;
} CMD_LIST;

const CMD_LIST commands[] =
{
	{CMD_WIFI_CONNECT, NULL},
	{CMD_NULL, NULL}
};

os_event_t    	cmdRecvQueue[CMD_TASK_QUEUE_SIZE];
RINGBUF 		rxRb;
uint8_t			rxBuf[256];

PROTO_PARSER 	rxProto;
uint8_t 		protoRxBuf[2048], protoTxBuf[2048];


static void ICACHE_FLASH_ATTR
CMD_Task(os_event_t *events);


static uint32_t ICACHE_FLASH_ATTR
CMD_Exec(const CMD_LIST *scp, PACKET_CMD *packet)
{
	while (scp->sc_name != CMD_NULL){
		if(scp->sc_name == packet->cmd) {
			return scp->sc_function(packet);
		}
		scp++;
	}
	return 0;
}

static void ICACHE_FLASH_ATTR
protoCompletedCb()
{
	PACKET_CMD *packet;
	packet = (PACKET_CMD*)protoRxBuf;
	CMD_Exec(commands, packet);
}

void ICACHE_FLASH_ATTR
CMD_Init()
{
	RINGBUF_Init(&rxRb, rxBuf, sizeof(rxBuf));
	PROTO_Init(&rxProto, protoCompletedCb, protoRxBuf, sizeof(protoRxBuf));

	system_os_task(CMD_Task, CMD_TASK_PRIO, cmdRecvQueue, CMD_TASK_QUEUE_SIZE);
	system_os_post(CMD_TASK_PRIO, 0, 0);
}

void ICACHE_FLASH_ATTR
CMD_Input(uint8_t data)
{
	RINGBUF_Put(&rxRb, data);
	system_os_post(CMD_TASK_PRIO, 0, 0);
}

static void ICACHE_FLASH_ATTR
CMD_Task(os_event_t *events)
{
	uint8_t c;
	while(RINGBUF_Get(&rxRb, &c) == 0){
		PROTO_ParseByte(&rxProto, c);
	}

}
