/*
 * mqtt_app.c
 *
 *  Created on: Feb 28, 2015
 *      Author: Minh
 */

#include "mqtt_app.h"
#include "osapi.h"
#include "mem.h"
#include "debug.h"
uint32_t connectedCb = 0, disconnectCb = 0, publishedCb = 0, dataCb = 0;

void mqttConnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    MQTT_CALLBACK *callback = (MQTT_CALLBACK*)client->user_data;
    INFO("MQTT: Connected\r\n");
    INFO("Callback data: %d, %d, %d, %d\r\n",
    			callback->connectedCb,
    			callback->disconnectedCb,
    			callback->publishedCb,
    			callback->dataCb);
    uint16_t crc = CMD_ResponseStart(CMD_MQTT_EVENTS, callback->connectedCb, 0, 0);
    CMD_ResponseEnd(crc);
}

void mqttDisconnectedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    MQTT_CALLBACK *cb = (MQTT_CALLBACK*)client->user_data;
    INFO("MQTT: Disconnected\r\n");
    uint16_t crc = CMD_ResponseStart(CMD_MQTT_EVENTS, cb->disconnectedCb, 0, 0);
	CMD_ResponseEnd(crc);
}

void mqttPublishedCb(uint32_t *args)
{
    MQTT_Client* client = (MQTT_Client*)args;
    MQTT_CALLBACK *cb = (MQTT_CALLBACK*)client->user_data;
    INFO("MQTT: Published\r\n");
    uint16_t crc = CMD_ResponseStart(CMD_MQTT_EVENTS, cb->publishedCb, 0, 0);
    CMD_ResponseEnd(crc);
}

void mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len)
{
	uint16_t crc = 0;


	MQTT_Client* client = (MQTT_Client*)args;
	MQTT_CALLBACK *cb = (MQTT_CALLBACK*)client->user_data;

	crc = CMD_ResponseStart(CMD_MQTT_EVENTS, cb->dataCb, 0, 2);
	crc = CMD_ResponseBody(crc, (uint8_t*)topic, topic_len);
	crc = CMD_ResponseBody(crc, (uint8_t*)data, data_len);
	CMD_ResponseEnd(crc);

}
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Setup(PACKET_CMD *cmd)
{
	REQUEST req;
	MQTT_Client *client;
	uint8_t *client_id, *user_data, *pass_data;
	uint16_t len;
	uint32_t keepalive, clean_seasion, cb_data;
	MQTT_CALLBACK *callback;


	CMD_Request(&req, cmd);
	if(CMD_GetArgc(&req) != 9)
		return 0;

	client = (MQTT_Client*)os_zalloc(sizeof(MQTT_Client));

	if(client == NULL)
		return 0;

	os_memset(client, 0, sizeof(MQTT_Client));

	/*Get client id*/
	len = CMD_ArgLen(&req);
	client_id = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, client_id);
	client_id[len] = 0;

	/*Get username*/
	len = CMD_ArgLen(&req);
	user_data = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, user_data);
	user_data[len] = 0;

	/*Get password*/
	len = CMD_ArgLen(&req);
	pass_data = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, pass_data);
	pass_data[len] = 0;

	CMD_PopArgs(&req, (uint8_t*)&keepalive);
	CMD_PopArgs(&req, (uint8_t*)&clean_seasion);


	/*init client*/
	INFO("MQTT: clientid = %s, user = %s, pass = %s, keepalive = %d, session = %d\r\n", client_id, user_data, pass_data, keepalive, clean_seasion);
	MQTT_InitClient(client, client_id, user_data, pass_data, keepalive, clean_seasion);

	callback = (MQTT_CALLBACK*)os_zalloc(sizeof(MQTT_CALLBACK));


	CMD_PopArgs(&req, (uint8_t*)&cb_data);
	callback->connectedCb = cb_data;
	CMD_PopArgs(&req, (uint8_t*)&cb_data);
	callback->disconnectedCb = cb_data;
	CMD_PopArgs(&req, (uint8_t*)&cb_data);
	callback->publishedCb = cb_data;
	CMD_PopArgs(&req, (uint8_t*)&cb_data);
	callback->dataCb = cb_data;


	client->user_data = callback;

	client->connectedCb = mqttConnectedCb;
	client->disconnectedCb = mqttDisconnectedCb;
	client->publishedCb = mqttPublishedCb;
	client->dataCb = mqttDataCb;

	os_free(client_id);
	os_free(user_data);
	os_free(pass_data);

	return (uint32_t)client;
}
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Lwt(PACKET_CMD *cmd)
{
	REQUEST req;
	MQTT_Client *client;
	CMD_Request(&req, cmd);
	uint16_t len;
	uint8_t *topic, *message;
	uint32_t qos, retain, client_ptr;
	if(CMD_GetArgc(&req) != 5)
		return 0;

	/* Get client*/
	CMD_PopArgs(&req, (uint8_t*)&client_ptr);
	client = (MQTT_Client*)client_ptr;
	INFO("MQTT: lwt client addr = %d\r\n", client_ptr);

	/*Get topic*/
	if(client->connect_info.will_topic)
		os_free(client->connect_info.will_topic);
	len = CMD_ArgLen(&req);
	client->connect_info.will_topic = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, client->connect_info.will_topic);
	client->connect_info.will_topic[len] = 0;

	/*Get message*/
	if(client->connect_info.will_message)
		os_free(client->connect_info.will_message);
	len = CMD_ArgLen(&req);
	client->connect_info.will_message = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, client->connect_info.will_message);
	client->connect_info.will_message[len] = 0;

	CMD_PopArgs(&req, (uint8_t*)&qos);
	CMD_PopArgs(&req, (uint8_t*)&retain);
	client->connect_info.will_qos = qos;
	client->connect_info.will_retain = retain;
	INFO("MQTT: lwt topic = %s, message = %s, qos = %d, retain = %d\r\n",
			client->connect_info.will_topic,
			client->connect_info.will_message,
			client->connect_info.will_qos,
			client->connect_info.will_retain);
	return 1;

}
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Connect(PACKET_CMD *cmd)
{
	MQTT_Client *client;
	REQUEST req;
	uint16_t len;
	uint32_t security;

	CMD_Request(&req, cmd);
	if(CMD_GetArgc(&req) != 4)
		return 0;

	CMD_PopArgs(&req, (uint8_t*)&client);

	/*Get host name*/
	len = CMD_ArgLen(&req);

	if(client->host)
		os_free(client->host);
	client->host = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, client->host);
	client->host[len] = 0;

	CMD_PopArgs(&req, (uint8_t*)&client->port);
	CMD_PopArgs(&req, (uint8_t*)&security);
	client->security = security;
	MQTT_Connect(client);
	return 1;
}
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Disconnect(PACKET_CMD *cmd)
{
	MQTT_Client *client;
	REQUEST req;
	uint16_t len;


	CMD_Request(&req, cmd);
	if(CMD_GetArgc(&req) != 1)
		return 0;
	CMD_PopArgs(&req, (uint8_t*)&client);

	MQTT_Disconnect(client);
	return 1;
}

uint32_t ICACHE_FLASH_ATTR MQTTAPP_Publish(PACKET_CMD *cmd)
{
	MQTT_Client *client;
	REQUEST req;
	uint16_t len;
	uint8_t *topic, *data;
	uint32_t qos = 0, retain = 0, data_len;

	CMD_Request(&req, cmd);
	if(CMD_GetArgc(&req) != 6)
		return 0;
	CMD_PopArgs(&req, (uint8_t*)&client);

	/*Get topic*/
	len = CMD_ArgLen(&req);

	topic = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, topic);
	topic[len] = 0;


	/*Get data*/
	len = CMD_ArgLen(&req);

	data = (uint8_t*)os_zalloc(len);
	CMD_PopArgs(&req, data);

	/*Get data length*/
	CMD_PopArgs(&req, (uint8_t*)&data_len);

	CMD_PopArgs(&req, (uint8_t*)&qos);
	CMD_PopArgs(&req, (uint8_t*)&retain);

	MQTT_Publish(client, topic, data, data_len, qos, retain);
	os_free(topic);
	os_free(data);
	return 1;

}
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Subscribe(PACKET_CMD *cmd)
{
	MQTT_Client *client;
	REQUEST req;
	uint16_t len;
	uint8_t *topic;
	uint32_t qos = 0;

	CMD_Request(&req, cmd);
	if(CMD_GetArgc(&req) != 3)
		return 0;
	CMD_PopArgs(&req, (uint8_t*)&client);

	/*Get topic*/
	len = CMD_ArgLen(&req);

	topic = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, topic);
	topic[len] = 0;
	CMD_PopArgs(&req, (uint8_t*)&qos);

	INFO("MQTT: topic = %s, qos = %d \r\n", topic, qos);
	MQTT_Subscribe(client, topic, qos);
	os_free(topic);
	return 1;
}

