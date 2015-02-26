/**
 * \file
 *       ESP8266 bridge arduino library 
 * \author
 *       Tuan PM <tuanpm@live.com>
 */
#ifndef _ARDUINO_WIFI_H_
#define _ARDUINO_WIFI_H_
#include <stdint.h>
#include <HardwareSerial.h>
#include <Arduino.h>
#include "FP.h"
#include "crc16.h"
#include "ringbuf.h"

enum WIFI_STATUS{
  STATION_IDLE = 0,
  STATION_CONNECTING,
  STATION_WRONG_PASSWORD,
  STATION_NO_AP_FOUND,
  STATION_CONNECT_FAIL,
  STATION_GOT_IP
};

typedef struct{
  uint8_t *buf;
  uint16_t bufSize;
  uint16_t dataLen;
  uint8_t isEsc;
  uint8_t isBegin;
}PROTO;

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

class RESPONSE {
private:
  uint16_t arg_index;
  PACKET_CMD* cmd;
public:
  RESPONSE(void *response);
  uint16_t getArgc();
};

class ESP
{
public:
  ESP(Stream *serial, Stream* debug, int chip_pd);
  ESP(Stream *serial, int chip_pd);

  FP<void, void*> wifiCb;

  void wifiConnect(String ssid, String password);
  void process();
private:
  Stream *_serial;
  Stream *_debug;
  boolean _debugEn;
  PROTO _proto;
  uint8_t _protoBuf[1024];
  int _chip_pd;
  uint32_t returnCmd;
  boolean isReturn;

  void init();
  void INFO(String info);
  void protoCompletedCb(void);
};
#endif
