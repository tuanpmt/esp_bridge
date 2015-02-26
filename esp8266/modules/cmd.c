/*
 * cmd.c
 *
 *  Created on: Jan 9, 2015
 *      Author: Minh
 */
#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "driver/uart.h"
#include "cmd.h"
#include "user_config.h"
#include "wifi.h"
#include "mqtt.h"
#include "ringbuf.h"
#include "proto.h"
#include "debug.h"
#include "crc16.h"




const CMD_LIST commands[] =
{
	{CMD_WIFI_CONNECT, NULL},
	{CMD_NULL, NULL}
};

os_event_t    	cmdRecvQueue[CMD_TASK_QUEUE_SIZE];
RINGBUF 		rxRb;
uint8_t			rxBuf[256];

PROTO_PARSER 	rxProto;
uint8_t 		protoRxBuf[2048];


static void ICACHE_FLASH_ATTR
CMD_Task(os_event_t *events);



ICACHE_FLASH_ATTR
void CMD_ProtoWrite(uint8_t data)
{
	switch(data){
	case 0x7D:
	case 0x7E:
	case 0x7F:
		uart0_write(0x7D);
		uart0_write(data ^ 0x20);
		break;
	default:
		uart0_write(data);
	}
}
ICACHE_FLASH_ATTR
void CMD_ProtoWriteBuf(uint8_t *data, uint32_t len)
{
	while(len--){
		CMD_ProtoWrite(*data++);
	}
}
ICACHE_FLASH_ATTR
void CMD_Response(uint16_t cmd, uint32_t _return, uint32_t callback, uint16_t argc, ARGS args[])
{
	uint16_t i = argc, j = 0, len = 2 + 4 + 4 + 2;
	uint8_t *data_send, *data_ptr = NULL;
	while(i --)
		len += args[i].len + 2;
	data_send = (uint8_t *)os_zalloc(len);
	data_ptr = data_send + 2;	/* reserved for checksum */

	*(uint16_t*)data_ptr = cmd;
	data_ptr += 2;

	*(uint32_t*)data_ptr = _return;
	data_ptr += 4;

	*(uint32_t*)data_ptr = callback;
	data_ptr += 4;

	*(uint16_t*)data_ptr = argc;
	data_ptr += 2;

	i = 0;
	while(argc--){
		*(uint16_t*)data_ptr = args[i].len;
		data_ptr += 2;
		j = args[i].len;
		uint8_t *data = &args[i].data;
		while(j --){
			*(uint8_t*)data_ptr = *data ++;
			data_ptr ++;
		}
		i ++;
	}

	*(uint16_t*)data_send = crc16_data(data_send + 2, len - 2, 0);

	uart0_write(0x7E);
	CMD_ProtoWriteBuf((uint8_t*)data_send, len);
	uart0_write(0x7F);

	os_free(data_send);

}
LOCAL uint32_t ICACHE_FLASH_ATTR
CMD_Exec(const CMD_LIST *scp, PACKET_CMD *packet)
{
	uint32_t ret;
	while (scp->sc_name != CMD_NULL){
		if(scp->sc_name == packet->cmd) {
			ret = scp->sc_function(packet);
			CMD_Response(packet->cmd, ret, 0, 0, NULL);
			return ret;
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
