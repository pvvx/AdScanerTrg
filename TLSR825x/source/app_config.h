#pragma once

#define SW_VERSION 0x20	 // BCD format (0x34 -> '3.4')

#define DEVICE_INFO_SERVICE_ENABLE 	1 // = 1 enable Device Information Characteristics
#define BATT_SERVICE_ENABLE			1
#define SPP_SERVICE_ENABLE			1
#define ADV_SERVICE_ENABLE			1
#define OTA_SERVICE_ENABLE			1

#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		250  //ms

#define USE_TIME_ADJUST		1 // = 1 time correction enabled
#define USE_BINDKEY			1

#define BLE_SECURITY_ENABLE 1
#define BLE_HOST_SMP_ENABLE BLE_SECURITY_ENABLE

#ifdef PCB_LYWSD03MMC
//// LYWSD03MMC B1.4:
#define HW_VERSION 0x10	 // BCD format (0x56 -> '5.6')
// GPIO_PA5 - DM, free, pcb mark "reset" (TRG)
// GPIO_PA6 - DP, free, pcb mark "P8" (RDS)
// GPIO_PB7 - free, pcb mark "B1" (ADC)
// GPIO_PC4 - free, pcb mark "P9" (LM)
// GPIO_PD2 - CS/PWM, free
#define GPIO_OUT_TH		GPIO_PA5
#define PA5_DATA_OUT		0
#define PA5_OUTPUT_ENABLE	1
#define PA5_FUNC			AS_GPIO

#define GPIO_OUT_RDS	GPIO_PA6
#define PA6_DATA_OUT		0
#define PA6_OUTPUT_ENABLE	1
#define PA6_FUNC			AS_GPIO

#define GPIO_OUT_LM		GPIO_PC4
#define PC4_DATA_OUT		0
#define PC4_OUTPUT_ENABLE	1
#define PC4_FUNC			AS_GPIO
#else
//// TB-03F-KIT
#define HW_VERSION 0x11	 // BCD format (0x56 -> '5.6')
// PC2,3,4 - LED1,2,3
#define GPIO_OUT_TH		GPIO_PC2
#define PC2_DATA_OUT		0
#define PC2_OUTPUT_ENABLE	1
#define PC2_FUNC			AS_GPIO

#define GPIO_OUT_RDS	GPIO_PC3
#define PC3_DATA_OUT		0
#define PC3_OUTPUT_ENABLE	1
#define PC3_FUNC			AS_GPIO

#define GPIO_OUT_LM		GPIO_PC4
#define PC4_DATA_OUT		0
#define PC4_OUTPUT_ENABLE	1
#define PC4_FUNC			AS_GPIO

#endif

#define CLOCK_SYS_CLOCK_HZ  	24000000 // 16000000, 24000000, 32000000, 48000000
enum{
	CLOCK_SYS_CLOCK_1S = CLOCK_SYS_CLOCK_HZ,
	CLOCK_SYS_CLOCK_1MS = (CLOCK_SYS_CLOCK_1S / 1000),
	CLOCK_SYS_CLOCK_1US = (CLOCK_SYS_CLOCK_1S / 1000000),
};

#define pm_wait_ms(t) cpu_stall_wakeup_by_timer0(t*CLOCK_SYS_CLOCK_1MS);
#define pm_wait_us(t) cpu_stall_wakeup_by_timer0(t*CLOCK_SYS_CLOCK_1US);

#define RAM _attribute_data_retention_ // short version, this is needed to keep the values in ram after sleep

#include "vendor/common/default_config.h"

