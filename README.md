## Tiva C libraries

### Current Support

* FreeRTOS

* Some peripheral of tiva c: uart, gpio

* tiva_log: like esp_log

* tiva_gps: using module neo 6m to test, parser NMEA contents

* tiva_sim: Using sim800l, can send message, and read message

### How to use:

* Import this library to your project 

* Config FreeRTOS library in tiva project [link](http://shukra.dese.iisc.ernet.in/edwiki/EmSys:FreeRTOS_on_the_EK-TM4C123GXL_LaunchPad_Board?fbclid=IwAR3PYq2__oJFvtGavRspdTy6U0JYU94AtMVmmtPIO7N06WsqIpHKMdeER0o) 

* Define paths to header files you want to build (in properties of project)

* import driverlib of TivaWare.

### Examples

* [tiva-tracker](https://github.com/loctranthanh/tiva-tracker)
