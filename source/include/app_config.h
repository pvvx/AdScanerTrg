/*
 * app_config.h
 *
 *  Created on: 10.11.2019
 *      Author: pvvx
 */
#ifndef _APP_CFG_H_
#define _APP_CFG_H_

#include "proj/mcu/config.h"

/* Enable C linkage for C++ Compilers: */
#if defined(__cplusplus)
extern "C" {
#endif

// Project devices:
#define USE_BLE 		1
#define USE_USB_CDC 	0
#define USE_UART_DEV 	0
#define USE_I2C_DEV 	0
#define USE_SPI_DEV 	0
#define USE_ADC_DEV 	0
#define USE_DAC_DEV 	0

#define USE_FLOADER	1 // Срабатывает при старте, если уровень на SWS "0". Для работы необходимо записать usbfloader.bin в 0x71000 или uartfloader.bin в 0x72800

#define UART_BAUD	115200 // 300..

////////// BLE product  Information  ////////////
#define DEV_NAME		"ADSCANER" // max name = 16 char!
#define DEV_NAME_SIZE 	8
#define DEV_VERSION 	0x1001

////////// USB product  Information  ////////////
#define STRING_VENDOR        L"Telink"
#define STRING_PRODUCT       L"ADSCANER"
#define STRING_SERIAL        L"1234567"

#define BLE_MASTER				0 // =0  slave -> set lib: liblt_8266_mod.a, =1 master -> set lib: liblt_8266_master_1_conn.a
/////////////////// BLE Service ////////////////////////////
#define BATT_SERVICE_ENABLE		0
#define SPP_SERVICE_ENABLE		1
#define ADV_SERVICE_ENABLE		1
#define OTA_SERVICE_ENABLE		1	// Для работы необходимо записать usbfloader.bin в 0x73000

/////////////////// WatchDog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		500  //ms

/////////////////// Board  //////////////////////////////
#define BOARD_JDY_10		1
#define BOARD_E104_BT05		2
#define BOARD_E104_BT10		3

#if BLE_MASTER
#define	USE_SYS_TICK_PER_US
#endif

#if	(BOARD == BOARD_JDY_10)
#include "board_jdy_10.h"
// выход, управляемый внешним термометром
#define GPIO_OUT_TH					GPIO_PC0
//#define PC0_FUNC                	AS_GPIO
#define PC0_INPUT_ENABLE        	1
#define PC0_OUTPUT_ENABLE       	1
//#define PC0_DATA_OUT				0
//#define PC0_DATA_STRENGTH       	1
// выход, управляемый внешним герконом
#define GPIO_OUT_RDS				GPIO_PE6
#define PE6_INPUT_ENABLE        	1
#define PE6_OUTPUT_ENABLE       	1
// выход, управляемый внешним датчиком света и движения
#define GPIO_OUT_LM					GPIO_PF0
#define PF0_INPUT_ENABLE        	1
#define PF0_OUTPUT_ENABLE       	1
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal
#elif (BOARD == BOARD_E104_BT05)
#include "board_e104_bt05.h"
// выход, управляемый внешним термометром
#define GPIO_OUT_TH					GPIO_PA1 // PWM1
//#define PC0_FUNC                	AS_GPIO
#define PA1_INPUT_ENABLE        	1
#define PA1_OUTPUT_ENABLE       	1
//#define PC0_DATA_OUT				0
//#define PC0_DATA_STRENGTH       	1
// выход, управляемый внешним герконом
#define GPIO_OUT_RDS				GPIO_PE4 // LINK
#define PE4_INPUT_ENABLE        	1
#define PE4_OUTPUT_ENABLE       	1
// выход, управляемый внешним датчиком света и движения
#define GPIO_OUT_LM					GPIO_PA5 // PWM2
#define PA5_INPUT_ENABLE        	1
#define PA5_OUTPUT_ENABLE       	1
#define CRYSTAL_TYPE			XTAL_16M		//  extern 16M crystal
#elif (BOARD == BOARD_E104_BT10)
#include "board_e104_bt10.h"
// выход, управляемый внешним термометром
#define GPIO_OUT_TH					GPIO_PC0
//#define PC0_FUNC                	AS_GPIO
#define PC0_INPUT_ENABLE        	1
#define PC0_OUTPUT_ENABLE       	1
//#define PC0_DATA_OUT				0
//#define PC0_DATA_STRENGTH       	1
// выход, управляемый внешним герконом
#define GPIO_OUT_RDS				GPIO_PE6
#define PE6_INPUT_ENABLE        	1
#define PE6_OUTPUT_ENABLE       	1
// выход, управляемый внешним датчиком света и движения
#define GPIO_OUT_LM					GPIO_PE4
#define PE4_INPUT_ENABLE        	1
#define PE4_OUTPUT_ENABLE       	1
#define CRYSTAL_TYPE			XTAL_16M		//  extern 16M crystal
#else
#undef BOARD
#define BOARD BOARD_JDY_10
#warning "BOARD = JDY-10!"
#include "board_jdy_10.h"
#define CRYSTAL_TYPE			XTAL_12M		//  extern 12M crystal
#endif



/////////////////// set default   ////////////////
#include "vendor/common/default_config.h"

/* Disable C linkage for C++ Compilers: */
#if defined(__cplusplus)
}
#endif

#endif // _APP_CFG_H_
