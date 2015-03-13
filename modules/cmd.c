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
#include "mqtt_app.h"
#include "rest.h"

#define SLIP_START	0x7E
#define SLIP_END	0x7F
#define SLIP_REPL	0x7D
#define SLIP_ESC(x)	(x ^ 0x20)

static void ICACHE_FLASH_ATTR
CMD_Task(os_event_t *events);
uint32_t ICACHE_FLASH_ATTR CMD_Reset(PACKET_CMD *cmd);
uint32_t ICACHE_FLASH_ATTR CMD_IsReady(PACKET_CMD *cmd);
const CMD_LIST commands[] =
{
	{CMD_RESET, CMD_Reset},
	{CMD_IS_READY, CMD_IsReady},
	{CMD_WIFI_CONNECT, WIFI_Connect},
	{CMD_MQTT_SETUP, MQTTAPP_Setup},
	{CMD_MQTT_CONNECT, MQTTAPP_Connect},
	{CMD_MQTT_DISCONNECT, MQTTAPP_Disconnect},
	{CMD_MQTT_PUBLISH, MQTTAPP_Publish},
	{CMD_MQTT_SUBSCRIBE, MQTTAPP_Subscribe},
	{CMD_MQTT_LWT, MQTTAPP_Lwt},

	{CMD_REST_SETUP, REST_Setup},
	{CMD_REST_REQUEST, REST_Request},
	{CMD_REST_SETHEADER, REST_SetHeader},
	{CMD_NULL, NULL}
};

os_event_t    	cmdRecvQueue[CMD_TASK_QUEUE_SIZE];
RINGBUF 		rxRb;
uint8_t			rxBuf[256];

PROTO_PARSER 	rxProto;
uint8_t 		protoRxBuf[2048];



uint32_t ICACHE_FLASH_ATTR CMD_Reset(PACKET_CMD *cmd)
{
	INFO("CMD: RESET\r\n");
	system_restart();
	return 1;
}
uint32_t ICACHE_FLASH_ATTR CMD_IsReady(PACKET_CMD *cmd)
{
	INFO("CMD: Check ready\r\n");
	return 1;
}


ICACHE_FLASH_ATTR
void CMD_ProtoWrite(uint8_t data)
{
	switch(data){
	case SLIP_START:
	case SLIP_END:
	case SLIP_REPL:
		uart0_write(SLIP_REPL);
		uart0_write(SLIP_ESC(data));
		break;
	default:
		uart0_write(data);
	}
}
ICACHE_FLASH_ATTR
void CMD_ProtoWriteBuf(uint8_t *data, uint32_t len)
{
	uint8_t* data_send = data;
	while(len--){
		CMD_ProtoWrite(*data_send++);
	}
}
ICACHE_FLASH_ATTR
uint16_t CMD_ResponseStart(uint16_t cmd, uint32_t callback, uint32_t _return, uint16_t argc)
{
  uint16_t crc = 0;
  uart0_write(SLIP_START);
  CMD_ProtoWriteBuf((uint8_t*)&cmd, 2);
  crc = crc16_data((uint8_t*)&cmd, 2, crc);
  CMD_ProtoWriteBuf((uint8_t*)&callback, 4);
  crc = crc16_data((uint8_t*)&callback, 4, crc);
  CMD_ProtoWriteBuf((uint8_t*)&_return, 4);
  crc = crc16_data((uint8_t*)&_return, 4, crc);
  CMD_ProtoWriteBuf((uint8_t*)&argc, 2);
  crc = crc16_data((uint8_t*)&argc, 2, crc);
  return crc;
}
ICACHE_FLASH_ATTR
uint16 CMD_ResponseBody(uint16_t crc_in, uint8_t* data, uint16_t len)
{
  uint8_t temp = 0;
  uint16_t pad_len = len;
  while(pad_len % 4 != 0)
    pad_len++;

  CMD_ProtoWriteBuf((uint8_t*)&pad_len, 2);
  crc_in = crc16_data((uint8_t*)&pad_len, 2, crc_in);
  while(len --){
	  CMD_ProtoWrite(*data);
    crc_in = crc16_data((uint8_t*)data, 1, crc_in);
    data ++;
    if(pad_len > 0) pad_len --;
  }

  while(pad_len --){
	  CMD_ProtoWrite(temp);
    crc_in = crc16_data((uint8_t*)&temp, 1, crc_in);
  }
  return crc_in;
}
ICACHE_FLASH_ATTR
uint16_t CMD_ResponseEnd(uint16_t crc)
{
	CMD_ProtoWriteBuf((uint8_t*)&crc, 2);
	uart0_write(SLIP_END);
	return 0;
}

LOCAL uint32_t ICACHE_FLASH_ATTR
CMD_Exec(const CMD_LIST *scp, PACKET_CMD *packet)
{
	uint32_t ret;
	uint16_t crc = 0;
	while (scp->sc_name != CMD_NULL){
		if(scp->sc_name == packet->cmd) {
			ret = scp->sc_function(packet);
			if(packet->_return){
				INFO("CMD: Response return value: %d, cmd: %d\r\n", ret, packet->cmd);
				crc = CMD_ResponseStart(packet->cmd, 0, ret, 0);
				CMD_ResponseEnd(crc);
			}

			return ret;
		}
		scp++;
	}
	return 0;
}

static void ICACHE_FLASH_ATTR
protoCompletedCb()
{
	uint16_t crc = 0, argc, len, resp_crc, argn = 0;
	uint8_t *data_ptr;
	PACKET_CMD *packet;
	packet = (PACKET_CMD*)protoRxBuf;

	data_ptr = (uint8_t*)&packet->args ;
	crc = crc16_data((uint8_t*)&packet->cmd, 12, crc);
	argc = packet->argc;

	INFO("CMD: %d, cb: %d, ret: %d, argc: %d\r\n", packet->cmd, packet->callback, packet->_return, packet->argc);

	while(argc--){
		len = *((uint16_t*)data_ptr);
		INFO("Arg[%d], len: %d:", argn++, len);
		crc = crc16_data(data_ptr, 2, crc);
		data_ptr += 2;
		crc = crc16_data(data_ptr, len, crc);
		while(len --){
		  INFO("%02X-", *data_ptr);
		  data_ptr ++;
		}
		INFO("\r\n\r\n");
	}
	resp_crc =  *(uint16_t*)data_ptr;
	INFO("Read CRC: %04X, calculated crc: %04X\r\n", resp_crc, crc);

	if(crc != resp_crc) {

		INFO("ESP: Invalid CRC\r\n");

		INFO("");
		return;
	}
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
void ICACHE_FLASH_ATTR CMD_Request(REQUEST *req, PACKET_CMD* cmd)
{
	req->cmd = cmd;
	req->arg_num = 0;
	req->arg_ptr = (uint8_t*)&cmd->args;
}
uint32_t ICACHE_FLASH_ATTR CMD_GetArgc(REQUEST *req)
{
	return req->cmd->argc;
}
int32_t ICACHE_FLASH_ATTR CMD_PopArgs(REQUEST *req, uint8_t *data)
{
	uint16_t length;

	if(req->arg_num >= req->cmd->argc)
		return -1;

	length = *(uint16_t*)req->arg_ptr;

	req->arg_ptr += 2;

	while(length --){
	*data ++ = *req->arg_ptr ++;
	}

	req->arg_num ++;
	return 0;
}
uint16_t CMD_ArgLen(REQUEST *req)
{
	return *(uint16_t*)req->arg_ptr;
}
