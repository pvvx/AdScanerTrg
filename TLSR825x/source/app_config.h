#pragma once

#define SW_VERSION 0x21	 // BCD format (0x34 -> '3.4')

#define BLE_DEVICE_ENABLE	1

#if (BLE_DEVICE_ENABLE)
#define DEVICE_INFO_SERVICE_ENABLE 	1 // = 1 enable Device Information Characteristics
#define BATT_SERVICE_ENABLE			1
#define SPP_SERVICE_ENABLE			1
#define ADV_SERVICE_ENABLE			1
#define OTA_SERVICE_ENABLE			1
#else
#define DEVICE_INFO_SERVICE_ENABLE 	0 // = 1 enable Device Information Characteristics
#define BATT_SERVICE_ENABLE			0
#define SPP_SERVICE_ENABLE			0
#define ADV_SERVICE_ENABLE			0
#define OTA_SERVICE_ENABLE			0
#endif

#define UART_PRINT_DEBUG_ENABLE 0

#define MODULE_WATCHDOG_ENABLE		0	// WDT not use
#define WATCHDOG_INIT_TIMEOUT		250  // ms

#define USE_TIME_ADJUST		1 // = 1 time correction enabled

#define BLE_SECURITY_ENABLE 0
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

#if UART_PRINT_DEBUG_ENABLE
#define PRINT_BAUD_RATE 1500000 // real 1000000
#define DEBUG_INFO_TX_PIN	GPIO_PA7 // SWS
#define PA7_DATA_OUT		1
#define PA7_OUTPUT_ENABLE	1
#define PULL_WAKEUP_SRC_PA7 PM_PIN_PULLUP_1M
#define PA7_FUNC		AS_GPIO
#endif // UART_PRINT_DEBUG_ENABLE

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

#if UART_PRINT_DEBUG_ENABLE
#define PRINT_BAUD_RATE 1500000 // real 1000000
#define DEBUG_INFO_TX_PIN	GPIO_PB1
#define PB1_DATA_OUT		1
#define PB1_OUTPUT_ENABLE	1
#define PULL_WAKEUP_SRC_PB1 PM_PIN_PULLUP_1M
#define PB1_FUNC		AS_GPIO
#endif // UART_PRINT_DEBUG_ENABLE

#endif

/* Power 3.3V, RX RF + TX ADV 1 sec, max:
 * 48MHz 7.3 mA
 * 24MHz 6.6 mA
 * 24MHz 6.2 mA
 * 16MHz 5.8 mA
 */
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

