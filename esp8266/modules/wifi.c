/*
 * wifi.c
 *
 *  Created on: Dec 30, 2014
 *      Author: Minh
 */
#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"

#include "user_config.h"
#include "cmd.h"

static ETSTimer WiFiLinker;
uint32_t wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
static void ICACHE_FLASH_ATTR wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;

	os_timer_disarm(&WiFiLinker);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{
		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 2000, 0);
	}
	else
	{
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{
			INFO("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{
			INFO("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{
			INFO("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else
		{
			INFO("STATION_IDLE\r\n");
		}

		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 500, 0);
	}
	if(wifiStatus != lastWifiStatus){
		lastWifiStatus = wifiStatus;
		if(wifiCb){
			ARGS args[1];
			args[0].len = 1;
			args[0].data = wifiStatus;
			CMD_Response(CMD_WIFI_CONNECT, 0, wifiCb, 1, args);
		}
	}
}

uint32_t ICACHE_FLASH_ATTR
WIFI_Connect(PACKET_CMD *cmd)
{
	struct station_config stationConf;
	ARGS *ssid, *pwd;
	ssid = &cmd->args;
	pwd = ssid + ssid->len + 2;

	INFO("WIFI_INIT\r\n");
	wifi_set_opmode(STATION_MODE);
	wifi_station_set_auto_connect(FALSE);
	if(cmd->argc != 2 || cmd->callback == 0)
		return 0xFFFFFFFF;

	wifiCb = cmd->callback;
	os_memset(&stationConf, 0, sizeof(struct station_config));

	os_memcpy(stationConf.ssid, &ssid->data, ssid->len);
	os_memcpy(stationConf.password, &pwd->data, pwd->len);

	INFO("WIFI: set ssid = %s, pass= %s\r\n", stationConf.ssid, stationConf.password);
	wifi_station_set_config(&stationConf);

	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);

	wifi_station_set_auto_connect(TRUE);
	wifi_station_connect();
	return 0;
}

