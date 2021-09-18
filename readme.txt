AdScanerTrg - Trigger and reed switch status receiver on TLSR826x.

Used JDY-10 module (TLSR8266).

Firmware files:

1. 8266_jdy_10.bin - the main project file.
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
configure the TRG settings: Control GPIO PA5 (marked "reset"),
and Reed Switch (GPIO PA6 - label on the "P8" pin for LYWSD03MMC).

Run [AdScanerTrg.html](https://pvvx.github.io/AdScanerTrg/AdScanerTrg.html) 
and configure the MAC of the sensor device to repeat 
the "Temperature or Humidity Trigger" on the GPIO:

PC0 - the output value of the GPIO_TRG pin (Temp/Hum trigger)
PE6 - input value of the GPIO_RDS pin (Reed Switch)

If sensor data is not received within 120 seconds, then PC0 and PE6 are set to "0".




