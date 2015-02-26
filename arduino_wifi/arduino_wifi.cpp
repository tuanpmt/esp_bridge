/**
 * \file
 *       ESP8266 bridge arduino library 
 * \author
 *       Tuan PM <tuanpm@live.com>
 */
  
#include "arduino_wifi.h"

RESPONSE::RESPONSE(void * response)
{
  cmd = (PACKET_CMD*)response;
}

uint16_t RESPONSE::getArgc()
{
  return cmd->argc;
}

void ESP::protoCompletedCb(void)
{
  PACKET_CMD *cmd = (PACKET_CMD*)_proto.buf;
  FP<void, void*> *fp;
  uint16_t checksum = crc16_data(_proto.buf + 2, _proto.dataLen - 2, 0);
  
  if(checksum != cmd->checksum) {
    INFO("Invalid checksum \r\n");
    return;
  }
  if(cmd->callback != 0){
    fp = (FP<void, void*>*)cmd->callback;
    if(fp->attached())
      (*fp)((void*)cmd);
  } else {
    if(cmd->argc == 1) {
      uint32_t *_ret = (uint32_t*)&cmd->args.data;
      isReturn = true;
      returnCmd = *_ret;
    }
    
  }
}

void ESP::wifiConnect(String ssid, String password)
{

}

void ESP::init()
{
  _proto.buf = _protoBuf;
  _proto.bufSize = sizeof(_protoBuf);
  _proto.dataLen = 0;
  _proto.isEsc = 0;
  pinMode(_chip_pd, OUTPUT);

}

ESP::ESP(Stream *serial, int chip_pd):
_serial(serial), _chip_pd(_chip_pd)
{
  _debugEn = false;
  init();
}

ESP::ESP(Stream *serial, Stream* debug, int chip_pd):
_serial(serial), _debug(debug), _chip_pd(_chip_pd)
{
    _debugEn = true;
    init();
}

void ESP::INFO(String info)
{
  if(_debugEn)
    _debug->println(info);
}

void ESP::process()
{
  char value;
  while(_serial->available()) {
    value = _serial->read();
    switch(value){
    case 0x7D:
      _proto.isEsc = 1;
      break;
    
    case 0x7E:
      _proto.dataLen = 0;
      _proto.isEsc = 0;
      _proto.isBegin = 1;
      break;
    
    case 0x7F:
      protoCompletedCb();
      _proto.isBegin = 0;
      break;
    
    default:
      if(_proto.isBegin == 0) break;
      
      if(_proto.isEsc){
        value ^= 0x20;
        _proto.isEsc = 0;
      }
        
      if(_proto.dataLen < _proto.bufSize)
        _proto.buf[_proto.dataLen++] = value;
        
      break;
  }
  }
}