# TLSR825x_AdScanerTrg

GPIO_TH - Switching on at a given temperature or temperatures with hysteresis. Supported devices: LYWSD03MMC, CGG1-M, MHO-C401, LYWSD02, ...

GPIO_RS - Duplication of the connected reed switch in LYWSD03MMC.

GPIO_LM - Output for turning on light by a CGPR1 sensor. The sensor must be turned on in Qingping mode.

#### Building the firmware

1. Go to [wiki.telink-semi.cn](http://wiki.telink-semi.cn/wiki/IDE-and-Tools/IDE-for-TLSR8-Chips/) and get the IDE for TLSR8 Chips.
2. Clone https://github.com/Ai-Thinker-Open/Telink_825X_SDK
3. Install the IDE and import the project
4. Change 'Linked resource' and 'C/C++ Build/Build command' 
5. Compile the project

