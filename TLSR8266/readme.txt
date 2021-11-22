
Used JDY-10 or E104-BT05 module (TLSR8266).

Firmware files:

1. 8266_jdy_10.bin, 8266_e104_bt05.bin - the main project file.
2. floader.bin - file of project TlsrComProg-Uartfloader. 
   Used to download firmware via UART. It is activated if, 
   at module startup, the SWS pin is shorted to GND.
3. ota_72000.bin - part for firmware on BLE OTA

Binary files for Firmware:

| Filename        | Flash addr |
|-----------------|------------|
| 8266_jdy_10.bin |  0x000000  |
| ota_72000.bin   |  0x072000  |
| floader.bin     |  0x072800  |

TelinkOTA - https://pvvx.github.io/UBIA/TelinkOTA.html

Install the Advertising type: "custom" on the custom firmware 
(https://github.com/pvvx/ATC_MiThermometer),
Reed Switch (GPIO PA6 - label on the "P8" pin for LYWSD03MMC).

Run [AdScanerTrg.html](https://pvvx.github.io/AdScanerTrg/AdScanerTrg.html) 
and configure the MAC of the sensors devices

JDY-10 pins:
PC0 - GPIO_TH: output controlled by an external temperature and humidity sensor (LYWSD03MMC, CGG1-M, MHO-C401, LYWSD02, ...)
PE6 - GPIO_RS: output controlled by external reed switch (LYWSD03MMC with Reed Switch)
PF0 - GPIO_LM: output controlled by an external motion and light sensor (CGPR1 in Qingping Mode)

E104-BT05 pins:
PA1 - GPIO_TH: output controlled by an external temperature and humidity sensor (LYWSD03MMC, CGG1-M, MHO-C401, LYWSD02, ...)
PE4 - GPIO_RS: output controlled by external reed switch (LYWSD03MMC with Reed Switch)
PA5 - GPIO_LM: output controlled by an external motion and light sensor (CGPR1 in Qingping Mode)


If sensor data is not received within 60 seconds, then GPIO_xx are set to "0".

* Implementation on E104-BT05-TB and ESP01Relay:

![Implementation on E104-BT05-TB and ESP01Relay](https://raw.githubusercontent.com/pvvx/pvvx.github.io/master/AdScanerTrg/img/AdScanerTrg-E104-BT05.gif)




