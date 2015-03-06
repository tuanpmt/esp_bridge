/*
 * mqtt_app.h
 *
 *  Created on: Feb 28, 2015
 *      Author: Minh
 */

#ifndef MODULES_MQTT_APP_H_
#define MODULES_MQTT_APP_H_

#include "mqtt.h"
#include "cmd.h"
typedef struct {
	uint32_t connectedCb;
	uint32_t disconnectedCb;
	uint32_t publishedCb;
	uint32_t dataCb;
}MQTT_CALLBACK;
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Connect(PACKET_CMD *cmd);
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Disconnect(PACKET_CMD *cmd);
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Setup(PACKET_CMD *cmd);
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Publish(PACKET_CMD *cmd);
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Subscribe(PACKET_CMD *cmd);
uint32_t ICACHE_FLASH_ATTR MQTTAPP_Lwt(PACKET_CMD *cmd);

#endif /* MODULES_MQTT_APP_H_ */
