/*
 * cmd.h
 *
 *  Created on: Jan 9, 2015
 *      Author: Minh
 */

#ifndef USER_CMD_H_
#define USER_CMD_H_


#define CMD_TASK_QUEUE_SIZE 1
#define CMD_TASK_PRIO		1

typedef struct __attribute((__packed__)) {
	uint16_t len;
	uint8_t data;
} ARGS;

typedef struct __attribute((__packed__)) {
	uint16_t cmd;
	uint32_t callback;
	uint32_t _return;
	uint16_t argc;
	ARGS args;
}PACKET_CMD;

typedef struct {
	PACKET_CMD *cmd;
	uint32_t arg_num;
	uint8_t *arg_ptr;
}REQUEST;

typedef enum
{
	CMD_NULL = 0,
	CMD_RESET,
	CMD_IS_READY,
	CMD_WIFI_CONNECT,
	CMD_MQTT_SETUP,
	CMD_MQTT_CONNECT,
	CMD_MQTT_DISCONNECT,
	CMD_MQTT_PUBLISH,
	CMD_MQTT_SUBSCRIBE,
	CMD_MQTT_LWT,
	CMD_MQTT_EVENTS,
	CMD_REST_SETUP,
	CMD_REST_REQUEST,
	CMD_REST_SETHEADER,
	CMD_REST_EVENTS
}CMD_NAME;

typedef uint32_t (*cmdfunc_t)(PACKET_CMD *cmd);

typedef struct {
	CMD_NAME  	sc_name;
	cmdfunc_t	sc_function;
} CMD_LIST;


void CMD_Init();
void CMD_Input(uint8_t data);

uint16_t CMD_ResponseStart(uint16_t cmd, uint32_t callback, uint32_t _return, uint16_t argc);
uint16 CMD_ResponseBody(uint16_t crc_in, uint8_t* data, uint16_t len);
uint16_t CMD_ResponseEnd(uint16_t crc);

void CMD_Response(uint16_t cmd, uint32_t callback, uint32_t _return, uint16_t argc, ARGS* args[]);
void CMD_Request(REQUEST *req, PACKET_CMD* cmd);
uint32_t CMD_GetArgc(REQUEST *req);
int32_t CMD_PopArgs(REQUEST *req, uint8_t *data);
uint16_t CMD_ArgLen(REQUEST *req);
#endif /* USER_CMD_H_ */
