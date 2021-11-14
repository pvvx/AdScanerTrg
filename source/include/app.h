/*
 * app.h
 *
 *      Author: pvvx
 */

#ifndef APP_H_
#define APP_H_

#include "ble.h"
#if (USE_USB_CDC)
#include "usb.h"
#endif
#if (USE_UART_DEV)
#include "uart_dev.h"
#endif

typedef struct __attribute__((packed)) _dev_cfg_t {
	int16_t		temp_threshold; // x0.01°, Set temp threshold
	int16_t		temp_hysteresis; // Set temp hysteresis, -327.67..327.67 °
	int16_t		humi_threshold; // x0.01%, Set humi threshold
	int16_t		humi_hysteresis; // Set humi hysteresis, -327.67..327.67 %
	int16_t		light_threshold;
	int16_t		light_hysteresis;
	uint16_t	motion_timer;
	struct {
		uint8_t 	motion_low_level:	1; // Low-level motion enable switch
	} flg;
} dev_cfg_t, * pdev_cfg_t;

#define DEV_CFG_EID 0xDECF
extern dev_cfg_t dev_cfg;

#define DEV_MAC1_EID 0xACCC
#define DEV_MAC2_EID 0xBCCC
extern uint8_t	dev1_MAC[6]; // [0] - lo, .. [6] - hi digits
extern uint8_t	dev2_MAC[6]; // [0] - lo, .. [6] - hi digits

typedef struct __attribute__((packed)) _wrk_data_t {
	int16_t			temp; // x0.01°
	int16_t			humi; // x0.01%
	uint16_t		light;
	uint16_t		motion_timer; // in sec
	uint8_t			motion_level; // close (=1) or distant (=0) movement (level low/hi)
	uint8_t			brightness_jump; // brightness jump up = 1, jump low = 0.
	struct {
		uint8_t 	rds_output	:	1; // Trigger Reed Switch
		uint8_t 	trg_output	:	1; // Trigger temp/humi output value
		uint8_t		lm_output	:	1; // Trigger light output value
		uint8_t 	temp_event  :	1; // Temperature trigger event
		uint8_t 	humi_event  :	1; // Humidity trigger event
		uint8_t 	light_event	:	1; // Light trigger event
//		uint8_t 	motion_event:	1; // Motion event
	} flg;
} wrk_data_t, * pwrk_data_t;

extern wrk_data_t wrk;

void user_init(void);
void main_loop(void);

#endif /* APP_H_ */
