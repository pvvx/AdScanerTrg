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
	int16_t		illuminance_threshold;
	int16_t		illuminance_hysteresis;
	uint16_t	motion_timer;
	struct {
		uint8_t 	reserved:	1;
	} flg;
} dev_cfg_t, * pdev_cfg_t;

#define DEV_CFG_EID 0xDECF
extern dev_cfg_t dev_cfg;

#define EEP_ID_KEY1 (0xBEAC) // EEP ID bkey
#define EEP_ID_KEY2 (0xBEAD) // EEP ID bkey

#define DEV_MAC1_EID 0xACCC
#define DEV_MAC2_EID 0xBCCC
extern uint8_t	dev1_MAC[6]; // [0] - lo, .. [6] - hi digits
extern uint8_t	dev2_MAC[6]; // [0] - lo, .. [6] - hi digits

typedef struct __attribute__((packed)) _wrk_data_t {
	int16_t			temp; // x0.01°
	int16_t			humi; // x0.01%
	uint16_t		illuminance; // lx ?
	uint16_t		motion_timer; // in sec
	uint8_t			motion_event; // Motion event
	uint8_t			light_on; 	// brightness jump up = 1, jump low = 0.
	struct {
		uint8_t 	rds_output	:	1; // Trigger Reed Switch
		uint8_t 	trg_output	:	1; // Trigger temp/humi output value
		uint8_t		lm_output	:	1; // Trigger light output value
		uint8_t 	temp_event  :	1; // Temperature trigger event
		uint8_t 	humi_event  :	1; // Humidity trigger event
		uint8_t 	light_event	:	1; // Light trigger event
	} flg;
} wrk_data_t, * pwrk_data_t;
extern wrk_data_t wrk;


typedef struct __attribute__((packed)) _ad_struct_head_t {
	uint8_t size;
	uint8_t type;
} ad_struct_head_t, * pad_struct_head_t;

typedef struct __attribute__((packed)) _ad_uuid16_t {
	uint8_t size;
	uint8_t type;
	uint16_t uuid16;
    uint8_t data[1]; // 1 - for check min length
} ad_uuid16_t, * pad_uuid16_t;

typedef struct __attribute__((packed)) _ad_wrk_t {
	uint8_t size;
	uint8_t type;
	uint16_t uuid16;
	wrk_data_t data;
} ad_wrk_t, * pad_wrk_t;

typedef struct __attribute__((packed)) _ad_flag_t {
	uint8_t size;
	uint8_t type;
	uint8_t flg;
} ad_flag_t, * pad_flag_t;

typedef struct __attribute__((packed)) _adv_buf_t {
	ad_flag_t	flag;
	ad_wrk_t	wrk;
}adv_buf_t;
extern adv_buf_t adv_buf;

extern u32 utc_time_sec;
extern u8 ble_connected;

void set_adv_data(void);
void user_init(void);
void main_loop(void);

#endif /* APP_H_ */
