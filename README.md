esp_bridge
================

This is source code for esp8266 support bridge for arduino or any MCU using SLIP protocol via Serial port. 
###Library for arduino: [https://github.com/tuanpmt/espduino](https://github.com/tuanpmt/espduino)

If you want using only ESP8266, you can find the **Native MQTT client** library for ESP8266 work well here: 
[https://github.com/tuanpmt/esp_mqtt](https://github.com/tuanpmt/esp_mqtt)

Warning
==============
This project **only execute commands from other MCU** via Serial port (like arduino).

Features
========
- Rock Solid wifi network client for Arduino (of course need to test more and resolve more issues :v)
- **More reliable** than AT COMMAND library (Personal comments)
- **Firmware applications written on ESP8266 can be read out completely. For security applications, sometimes you should use it as a Wifi client (network client) and other MCU with Readout protection.**
- It can be combined to work in parallel with the AT-COMMAND library
- MQTT module: 
    + MQTT client run stable as Native MQTT client (esp_mqtt)
    + Support subscribing, publishing, authentication, will messages, keep alive pings and all 3 QoS levels (it should be a fully functional client).
    + **Support multiple connection (to multiple hosts).**
    + Support SSL
    + Easy to setup and use
- REST module:
    + Support method GET, POST, PUT, DELETE
    + setContent type, set header, set User Agent
    + Easy to used API
    + Support SSL
- WIFI module:


Installations
========

You can found here for instructions: [https://github.com/tuanpmt/espduino](https://github.com/tuanpmt/espduino)

Compile
=======

```bash
git clone --recursive https://github.com/tuanpmt/esp_bridge
cd esp_bridge
make all
```

Limited
========
It is not completely tested. Welcome your submitting issues.

Contributing
==========
There is very much needed module for this library, welcome your pull request:

Donations
==================
Invite me to a coffee
[![Donate](https://www.paypalobjects.com/en_US/GB/i/btn/btn_donateCC_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=JR9RVLFC4GE6J)

Authors:
=====
[Tuan PM](https://twitter.com/tuanpmt)


**LICENSE - "MIT License"**

Copyright (c) 2014-2015 Tuan PM, [https://twitter.com/tuanpmt](https://twitter.com/tuanpmt)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.