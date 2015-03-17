/*
 * api.c
 *
 *  Created on: Mar 4, 2015
 *      Author: Minh
 */
#include "rest.h"

#include "user_interface.h"
#include "osapi.h"
#include "mem.h"
#include "driver/uart.h"
#include "cmd.h"
#include "user_config.h"
#include "espconn.h"
#include "os_type.h"
#include "debug.h"

void ICACHE_FLASH_ATTR
tcpclient_discon_cb(void *arg)
{

	struct espconn *pespconn = (struct espconn *)arg;
	REST_CLIENT* client = (REST_CLIENT *)pespconn->reverse;


}


void ICACHE_FLASH_ATTR
tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
	uint8_t currentLineIsBlank = 0;
	uint8_t httpBody = 0;
	uint8_t inStatus = 0;
	char statusCode[4];
	int i = 0, j;
	uint32_t code = 0;
	uint16_t crc;

	struct espconn *pCon = (struct espconn*)arg;
	REST_CLIENT *client = (REST_CLIENT *)pCon->reverse;

	for(j=0 ;j<len; j++){
		char c = pdata[j];

		if(c == ' ' && !inStatus){
			inStatus = 1;
		}
		if(inStatus && i < 3 && c != ' '){
			statusCode[i] = c;
			i++;
		}
		if(i == 3){
			statusCode[i] = '\0';
			code = atoi(statusCode);
		}
		 if(httpBody){
			 //only write response if its not null
			 uint32_t body_len = len - j;
			 INFO("REST: status = %d, body_len = %d\r\n",code, body_len);
			 if(body_len == 0){
				 crc = CMD_ResponseStart(CMD_REST_EVENTS, client->resp_cb, code, 0);
			 } else {
				 crc = CMD_ResponseStart(CMD_REST_EVENTS, client->resp_cb, code, 1);
				 crc = CMD_ResponseBody(crc, &pdata[j], body_len);
			 }
			 CMD_ResponseEnd(crc);
			 break;
		}
		else
		{
			if (c == '\n' && currentLineIsBlank) {
				httpBody = true;
			}
			if (c == '\n') {
				// you're starting a new line
				currentLineIsBlank = true;
			}
			else if (c != '\r') {
				// you've gotten a character on the current line
				currentLineIsBlank = false;
			}
		}
	}
	if(client->security)
		espconn_secure_disconnect(client->pCon);
	else
		espconn_disconnect(client->pCon);

}
void ICACHE_FLASH_ATTR
tcpclient_sent_cb(void *arg)
{
	struct espconn *pCon = (struct espconn *)arg;
	REST_CLIENT* client = (REST_CLIENT *)pCon->reverse;
	INFO("REST: Sent\r\n");
}

void ICACHE_FLASH_ATTR
tcpclient_connect_cb(void *arg)
{
	struct espconn *pCon = (struct espconn *)arg;
	REST_CLIENT* client = (REST_CLIENT *)pCon->reverse;

	espconn_regist_disconcb(client->pCon, tcpclient_discon_cb);
	espconn_regist_recvcb(client->pCon, tcpclient_recv);////////
	espconn_regist_sentcb(client->pCon, tcpclient_sent_cb);///////

	if(client->security){
		espconn_secure_sent(client->pCon, client->data, client->data_len);
	}
	else{
		espconn_sent(client->pCon, client->data, client->data_len);
	}
}
void ICACHE_FLASH_ATTR
tcpclient_recon_cb(void *arg, sint8 errType)
{
	struct espconn *pCon = (struct espconn *)arg;
	REST_CLIENT* client = (REST_CLIENT *)pCon->reverse;

}
LOCAL void ICACHE_FLASH_ATTR
rest_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *pConn = (struct espconn *)arg;
	REST_CLIENT* client = (REST_CLIENT *)pConn->reverse;
	if(ipaddr == NULL)
	{
		INFO("REST DNS: Found, but got no ip, try to reconnect\r\n");
		return;
	}

	INFO("REST DNS: found ip %d.%d.%d.%d\n",
			*((uint8 *) &ipaddr->addr),
			*((uint8 *) &ipaddr->addr + 1),
			*((uint8 *) &ipaddr->addr + 2),
			*((uint8 *) &ipaddr->addr + 3));

	if(client->ip.addr == 0 && ipaddr->addr != 0)
	{
		os_memcpy(client->pCon->proto.tcp->remote_ip, &ipaddr->addr, 4);
		if(client->security){
			espconn_secure_connect(client->pCon);
		}
		else {
			espconn_connect(client->pCon);
		}
		INFO("REST: connecting...\r\n");
	}
}
uint32_t ICACHE_FLASH_ATTR REST_Setup(PACKET_CMD *cmd)
{
	REQUEST req;
	REST_CLIENT *client;
	uint8_t *rest_host;
	uint16_t len;
	uint32_t port, security;

	CMD_Request(&req, cmd);
	if(CMD_GetArgc(&req) != 3)
		return 0;

	len = CMD_ArgLen(&req);
	rest_host = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, rest_host);
	rest_host[len] = 0;

	client = (REST_CLIENT*)os_zalloc(sizeof(REST_CLIENT));
	os_memset(client, 0, sizeof(REST_CLIENT));
	if(client == NULL)
		return 0;

	CMD_PopArgs(&req, (uint8_t*)&port);

	CMD_PopArgs(&req, (uint8_t*)&security);

	client->resp_cb = cmd->callback;

	client->host = rest_host;
	client->port = port;
	client->security = security;
	client->ip.addr = 0;

	client->data = (uint8_t*)os_zalloc(1024);

	client->header = (uint8_t*)os_zalloc(4);
	client->header[0] = 0;

	client->content_type = (uint8_t*)os_zalloc(22);
	os_sprintf(client->content_type, "x-www-form-urlencoded");
	client->content_type[21] = 0;

	client->user_agent = (uint8_t*)os_zalloc(17);
	os_sprintf(client->user_agent, "ESPDRUINO@tuanpmt");
	client->user_agent[16] = 0;

	client->pCon = (struct espconn *)os_zalloc(sizeof(struct espconn));
	client->pCon->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));

	client->pCon->type = ESPCONN_TCP;
	client->pCon->state = ESPCONN_NONE;
	client->pCon->proto.tcp->local_port = espconn_port();
	client->pCon->proto.tcp->remote_port = client->port;

	client->pCon->reverse = client;

	return (uint32_t)client;
}
uint32_t ICACHE_FLASH_ATTR REST_SetHeader(PACKET_CMD *cmd)
{
	REQUEST req;
	REST_CLIENT *client;
	uint16_t len;
	uint32_t header_index, client_ptr = 0;

	CMD_Request(&req, cmd);

	if(CMD_GetArgc(&req) != 3)
		return 0;

	/* Get client*/
	CMD_PopArgs(&req, (uint8_t*)&client_ptr);
	client = (REST_CLIENT*)client_ptr;

	CMD_PopArgs(&req, (uint8_t*)&header_index);
	len = CMD_ArgLen(&req);

	switch(header_index) {
	case HEADER_GENERIC:
		if(client->header)
			os_free(client->header);
		client->header = (uint8_t*)os_zalloc(len + 1);
		CMD_PopArgs(&req, (uint8_t*)client->header);
		client->header[len] = 0;
		INFO("Set header: %s\r\n", client->header);
		break;
	case HEADER_CONTENT_TYPE:
		if(client->content_type)
			os_free(client->content_type);
		client->content_type = (uint8_t*)os_zalloc(len + 1);
		CMD_PopArgs(&req, (uint8_t*)client->content_type);
		client->content_type[len] = 0;
		INFO("Set content_type: %s\r\n", client->content_type);
		break;
	case HEADER_USER_AGENT:
		if(client->user_agent)
			os_free(client->user_agent);
		client->user_agent = (uint8_t*)os_zalloc(len + 1);
		CMD_PopArgs(&req, (uint8_t*)client->user_agent);
		client->user_agent[len] = 0;
		INFO("Set user_agent: %s\r\n", client->user_agent);
		break;
	}
	return 1;

}
uint32_t ICACHE_FLASH_ATTR REST_Request(PACKET_CMD *cmd)
{

	REQUEST req;
	REST_CLIENT *client;
	uint16_t len, realLen = 0;
	uint32_t client_ptr;
	uint8_t *method, *path, *body = NULL;

	CMD_Request(&req, cmd);

	if(CMD_GetArgc(&req) <3)
		return 0;

	/* Get client*/
	CMD_PopArgs(&req, (uint8_t*)&client_ptr);
	client = (REST_CLIENT*)client_ptr;

	//method
	len = CMD_ArgLen(&req);
	method = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, method);
	method[len] = 0;


	//path
	len = CMD_ArgLen(&req);
	path = (uint8_t*)os_zalloc(len + 1);
	CMD_PopArgs(&req, path);
	path[len] = 0;

	//body
	if(CMD_GetArgc(&req) == 3){
		realLen = 0;
		len = 0;
	} else {
		CMD_PopArgs(&req, (uint8_t*)&realLen);

		len = CMD_ArgLen(&req);
		body = (uint8_t*)os_zalloc(len + 1);
		CMD_PopArgs(&req, body);
		body[len] = 0;
	}

	client->pCon->state = ESPCONN_NONE;

	INFO("REQ: method: %s, path: %s\r\n", method, path);

	client->data_len = os_sprintf(client->data, "%s %s HTTP/1.1\r\n"
												"Host: %s\r\n"
												"%s"
												"Content-Length: %d\r\n"
												"Connection: close\r\n"
												"Content-Type: %s\r\n"
												"User-Agent: %s\r\n\r\n",
												method, path,
												client->host,
												client->header,
												realLen,
												client->content_type,
												client->user_agent);

	if(realLen > 0){
		os_memcpy(client->data + client->data_len, body, realLen);
		client->data_len += realLen;
		os_sprintf(client->data + client->data_len, "\r\n\r\n");
		client->data_len += 4;
	}

	client->pCon->state = ESPCONN_NONE;
	espconn_regist_connectcb(client->pCon, tcpclient_connect_cb);
	espconn_regist_reconcb(client->pCon, tcpclient_recon_cb);

	if(UTILS_StrToIP(client->host, &client->pCon->proto.tcp->remote_ip)) {
		INFO("REST: Connect to ip  %s:%d\r\n",client->host, client->port);
		if(client->security){
			espconn_secure_connect(client->pCon);
		}
		else {
			espconn_connect(client->pCon);
		}
	}
	else {
		INFO("REST: Connect to domain %s:%d\r\n", client->host, client->port);
		espconn_gethostbyname(client->pCon, client->host, &client->ip, rest_dns_found);
	}
	os_free(method);
	os_free(path);
	if(body) os_free(body);
	return 1;
}
