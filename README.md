AdScanerTrg - relay control from external BLE sensors for temperature, humidity, lighting, motion, reed switch. A repeater of BLE advertising packages.

Used SoC TLSR8266 or TLSR825x (module: JDY-10, E104-BT05, TB-03F, TB-04, ...)

### Controls three relay outputs:
1. Switching on at a given temperature and / or humidity with hysteresis.
2. Switch the lamp at a given level of illumination and / or motion detection.
3. Repeater of the state of the reed switch [LYWSD03MMC fw:pvvx](https://github.com/pvvx/ATC_MiThermometer#reed-switch-on-gpio-pa6-label-on-the-p8-pin).

### Supported BLE sensors devices: LYWSD03MMC, CGG1-M, MHO-C401, LYWSD02, ...

In connection mode, it has the ability to work as a repeater - scanner BLE advertising from other devices.

TelinkOTA - https://pvvx.github.io/UBIA/TelinkOTA.html

Install the Advertising type: "custom" on the custom firmware 
(https://github.com/pvvx/ATC_MiThermometer),
Reed Switch (GPIO PA6 - label on the "P8" pin for LYWSD03MMC).

![AdScanerTrgHtml.gif](https://raw.githubusercontent.com/pvvx/pvvx.github.io/master/AdScanerTrg/img/AdScanerTrgHtml.gif)


Run [AdScanerTrg2.html](https://pvvx.github.io/AdScanerTrg/AdScanerTrg2.html) 
and configure the MAC, bindkey of the sensors devices

* Implementation on E104-BT05-TB and ESP01Relay:

![Implementation on E104-BT05-TB and ESP01Relay](https://raw.githubusercontent.com/pvvx/pvvx.github.io/master/AdScanerTrg/img/AdScanerTrg-E104-BT05.gif)




