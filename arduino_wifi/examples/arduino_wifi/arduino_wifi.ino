/**
 * \file
 *       ESP8266 bridge for Arduino
 * \author
 *       Tuan PM <tuanpm@live.com>
 */
 
#include <arduino_wifi.h>
#include <SoftwareSerial.h>

SoftwareSerial debugPort(2, 3); // RX, TX

ESP esp(&Serial, &debugPort, 13);

void wifiCb(void* response)
{
	RESPONSE res(response);
	if(res.getArgc() == 1) {
		
	}
}

void setup() {
	Serial.begin(115200);
	debugPort.begin(9600);

	esp.wifiCb.attach(&wifiCb);
	esp.wifiConnect("DVES_HOME", "dongvietdves");

}

void loop() {
	esp.process();
}