/*
 * scaning.c
 *
 *  Created on: 20.11.2021
 *      Author: pvvx
 */
#include <stdint.h>
#include "tl_common.h"
#include "ble.h"
#include "cmd_parser.h"
//#include "app.h"

#if (ADV_SERVICE_ENABLE)
#define MYFIFO_BLK_SIZE		40 // min 1+6+31 = 38 bytes
#if (MTU_DATA_SIZE < MYFIFO_BLK_SIZE)
#error "MTU_DATA_SIZE < MYFIFO_BLK_SIZE !"
#endif
MYFIFO_INIT(ad_fifo, MYFIFO_BLK_SIZE, 8); 	// 40*8 = 320 bytes + sizeof(my_fifo_t)
#endif

typedef struct __attribute__((packed)) _ad_head_t {
    u8 len;
    u8 type;
    u16 uuid16;
    u8 data[1]; // 1 - for check min_length
} ad_head_t, * pad_head_t;

// GPIO_TRG pin (marking "reset" on circuit board) flags:
typedef struct __attribute__((packed)) _trigger_flg_t {
	uint8_t 	rds_input	:	1; // Reed Switch, input
	uint8_t 	trg_output	:	1; // GPIO_TRG pin output value (pull Up/Down)
	uint8_t 	trigger_on	:	1; // Output GPIO_TRG pin is controlled according to the set parameters threshold temperature or humidity
	uint8_t 	temp_event	:	1; // Temperature trigger event
	uint8_t 	humi_event	:	1; // Humidity trigger event
}trigger_flg_t;

// GATT Service 0x181A Environmental Sensing
// All data little-endian
typedef struct __attribute__((packed)) _adv_custom_t {
	uint8_t		size;	// = 18
	uint8_t		uid;	// = 0x16, 16-bit UUID
	uint16_t	UUID;	// = 0x181A, GATT Service 0x181A Environmental Sensing
	uint8_t		MAC[6]; // [0] - lo, .. [6] - hi digits
	int16_t		temperature; // x 0.01 degree
	uint16_t	humidity; // x 0.01 %
	uint16_t	battery_mv; // mV
	uint8_t		battery_level; // 0..100 %
	uint8_t		counter; // measurement count
	trigger_flg_t	flags;
} adv_custom_t, * padv_custom_t;

// GATT Service 0xfe95 Xiaomi Inc.
// All data little-endian
typedef struct __attribute__((packed)) _adv_xiaomi_t {
	uint8_t		size;	// = ?
	uint8_t		uid;	// = 0x16, 16-bit UUID
	uint16_t	UUID;	// = 0xfe95, GATT Service
	uint16_t    ctrID;
	uint16_t    devID;
	uint8_t		counter;
	uint8_t		MAC[6]; // [0] - lo, .. [6] - hi digits
	union __attribute__((packed)) {
		uint8_t		cap;	// Capability
		uint8_t		data[3]; //
	};
} adv_xiaomi_t, * padv_xiaomi_t;

typedef struct __attribute__((packed)) _adv_struct_xiaomi_t {
	uint16_t		id;
	uint8_t			size;
	union __attribute__((packed)) {
		uint8_t		data_ub[4];
		uint16_t	data_us[2];
		uint32_t	data_uw;
		int8_t		data_ib[4];
		int16_t		data_is[2];
		int32_t		data_iw;
	};
} adv_struct_xiaomi_t, * padv_struct_xiaomi_t;

// GATT Service 0xfdcd Qingping Technology (Beijing) Co., Ltd.
// All data little-endian

typedef struct __attribute__((packed)) _adv_qingping_t {
	uint8_t		size;	// = ?
	uint8_t		uid;	// = 0x16, 16-bit UUID
	uint16_t	UUID;	// = 0xfdcd, GATT Service
	uint8_t 	hlen;	// bit0..5 - strut size, bit6 - Event
	uint8_t 	device_id;
	uint8_t		MAC[6]; // [0] - lo, .. [6] - hi digits
	uint8_t		data[2]; // 2 - for check min_length
} adv_qingping_t, * padv_qingping_t;

typedef struct __attribute__((packed)) _adv_struct_qingping_t {
	union __attribute__((packed)) {
		struct __attribute__((packed)) {
			uint8_t		id;
			uint8_t 	size;
		};
		uint16_t	id_size;
	};
	union __attribute__((packed)) {
		uint8_t		data_ub[4];
		uint16_t	data_us[2];
		uint32_t	data_uw;
		int8_t		data_ib[4];
		int16_t		data_is[2];
		int32_t		data_iw;
	};
} adv_struct_qingping_t, * padv_struct_qingping_t;

RAM u32 off_tisk_th;
RAM u32 off_tisk_lm;

_attribute_ram_code_
__attribute__((optimize("-Os")))
void set_th_out(void) {
	if(dev_cfg.temp_hysteresis) {
		if(wrk.flg.temp_event) { // temp_out on
			if(dev_cfg.temp_hysteresis < 0) {
				if(wrk.temp > dev_cfg.temp_threshold - dev_cfg.temp_hysteresis) {
					wrk.flg.temp_event = false;
				}
			} else {
				if(wrk.temp < dev_cfg.temp_threshold - dev_cfg.temp_hysteresis) {
					wrk.flg.temp_event = false;
				}
			}
		} else { // temp_out off
			if(dev_cfg.temp_hysteresis < 0) {
				if(wrk.temp < dev_cfg.temp_threshold + dev_cfg.temp_hysteresis) {
					wrk.flg.temp_event = true;
				}
			} else {
				if(wrk.temp > dev_cfg.temp_threshold + dev_cfg.temp_hysteresis) {
					wrk.flg.temp_event = true;
				}
			}
		}
	} else wrk.flg.temp_event = false;
	if(dev_cfg.humi_hysteresis) {
		if(wrk.flg.humi_event) { // humi_event on
			if(dev_cfg.humi_hysteresis < 0) {
				if(wrk.humi > dev_cfg.humi_threshold - dev_cfg.humi_hysteresis) {
					// humi > threshold
					wrk.flg.humi_event = false;
				}
			} else { // hysteresis > 0
				if(wrk.humi < dev_cfg.humi_threshold - dev_cfg.humi_hysteresis) {
					// humi < threshold
					wrk.flg.humi_event = false;
				}
			}
		} else { // humi_event off
			if(dev_cfg.humi_hysteresis < 0) {
				if(wrk.humi < dev_cfg.humi_threshold + dev_cfg.humi_hysteresis) {
					// humi < threshold
					wrk.flg.humi_event = true;
				}
			} else { // hysteresis > 0
				if(wrk.humi > dev_cfg.humi_threshold + dev_cfg.humi_hysteresis) {
					// humi > threshold
					wrk.flg.humi_event = true;
				}
			}
		}
	} else wrk.flg.humi_event = false;
	if(dev_cfg.temp_hysteresis || dev_cfg.humi_hysteresis) {
		wrk.flg.trg_output = (wrk.flg.humi_event || wrk.flg.temp_event);
	}
	off_tisk_th = utc_time_sec; //clock_time();
	gpio_write(GPIO_OUT_TH, wrk.flg.trg_output);
}

_attribute_ram_code_
__attribute__((optimize("-Os")))
void set_lm_out(int motion_event) {
	if(dev_cfg.light_hysteresis) {
		if(wrk.flg.light_event) { // lm_event on
			if(dev_cfg.light_hysteresis < 0) {
				if(wrk.light > dev_cfg.light_threshold - dev_cfg.light_hysteresis) {
					wrk.flg.light_event = false;
				}
			} else {
				if(wrk.light < dev_cfg.light_threshold - dev_cfg.light_hysteresis) {
					wrk.flg.light_event = false;
				}
			}
		} else { // lm_event off
			if(dev_cfg.light_hysteresis < 0) {
				if(wrk.light < dev_cfg.light_threshold + dev_cfg.light_hysteresis) {
					wrk.flg.light_event = true;
				}
			} else {
				if(wrk.light > dev_cfg.light_threshold + dev_cfg.light_hysteresis) {
					wrk.flg.light_event = true;
				}
			}
		}
	} else wrk.flg.light_event = false;

	if(wrk.motion_timer) { // свет включен по событию движения
		if(motion_event && dev_cfg.motion_timer && wrk.motion_level) { // обнаружено движение и включен опрос движения
			wrk.flg.lm_output = true;	// включить LM
			wrk.motion_timer = dev_cfg.motion_timer; // обновить таймер
		} // не менять состояние выхода LM
	} else { // свет ещё не включен по событию движения
		if(motion_event && dev_cfg.motion_timer && wrk.motion_level) { // обнаружено движение и включен опрос движения
			// #TODO wrk.motion == 0 -> не стартовать включение?
			if(wrk.flg.light_event // темно?
				|| dev_cfg.light_hysteresis == 0) {  // или не задано слежение за освещением?
					wrk.flg.lm_output = true;	// включить LM
					wrk.motion_timer = dev_cfg.motion_timer; // установить таймер
			} else
					wrk.flg.lm_output = false;	// не включать LM
		} else // работа только по освещению
			if(dev_cfg.motion_timer) // включен опрос движения
				wrk.flg.lm_output = false;	// не включать LM
			else
				wrk.flg.lm_output = wrk.flg.light_event;
	}
	off_tisk_lm = utc_time_sec; //clock_time();
	gpio_write(GPIO_OUT_LM, wrk.flg.lm_output);
}

_attribute_ram_code_
__attribute__((optimize("-Os")))
void filter_xiaomi_ad(padv_xiaomi_t p) {
	padv_struct_xiaomi_t ps = (padv_struct_xiaomi_t) &p->cap;
	int len = p->size;
	if(len > sizeof(adv_xiaomi_t)) {
		if(((memcmp(dev1_MAC, p->MAC, 6) == 0)||(memcmp(dev2_MAC, p->MAC, 6) == 0))
				&&((p->ctrID & 0x88) == 0) // No Mesh, No Data encrypted
				&&((p->ctrID & 0x50) == 0x50) // MAC and Data  presents
		) {
			len -= sizeof(adv_xiaomi_t) - 4;
			if(p->ctrID & 0x20) { // includes Capability
				ps = (padv_struct_xiaomi_t) &p->data[1];
				len -= 1;
				if(p->cap & 0x20) {
					ps = (padv_struct_xiaomi_t) &p->data[3];
					len -= 2;
				}
			}
			while((ps->size + 3) <= len) {
				// ..70205B04BE6E4883012EE709061002CC01
				if(ps->id == 0x1004) { // Temp
					wrk.temp = ps->data_is[0]*10; // in 0.1 C
					set_th_out();
				} else if(ps->id == 0x1006) { // Humi
					wrk.humi = ps->data_is[0]*10;  // in 0.1 %
					set_th_out();
				} else if(ps->id == 0x1007) { // light
					wrk.light = ps->data_is[0];  // in ?
					set_lm_out(0);
//				} else if(ps->id_size == 0x100a) { // Batt %
//					exdev_batt = ps->data_us[0];  // in %
				} else if(ps->id == 0x100d) { // Temp + Humi
					wrk.temp = ps->data_is[0]*10; // in 0.1 C
					wrk.humi = ps->data_is[1]*10; // in 0.1 %
					set_th_out();
				}
				len -= ps->size + 3;
				ps = (padv_struct_xiaomi_t)((uint32_t)ps + ps->size + 3);
			}
		}
	}
}

_attribute_ram_code_
__attribute__((optimize("-Os")))
void filter_qingping_ad(padv_qingping_t p) {
	padv_struct_qingping_t ps = (padv_struct_qingping_t) &p->data;
	int len = p->size;
	// ..0812 005E60342D58 0201 64 0F01 7D 0904 8C120000
	if(len > sizeof(adv_qingping_t) && (p->hlen & 0x1f) == 0x08) {
		if((memcmp(&dev2_MAC, &p->MAC, 6) == 0)||(memcmp(&dev1_MAC, &p->MAC, 6) == 0)) {
			len -= 11;
			while((ps->size + 2) <= len) {
				if(ps->id_size == 0x0401) { // Temp + Humi
					wrk.temp = ps->data_is[0]*10; // in 0.1 C
					wrk.humi = ps->data_is[1]*10; // in 0.1 %
					set_th_out();
//				} else if(ps->id_size == 0x0102) { // Batt %
//					exdev_battt = ps->data_us[0];  // in %
//				} else if(ps->id_size == 0x0207) { // Pressure
//					extdev_pressure = ps->data_us[0];  // in 0.01
//				} else if(ps->id_size == 0x010f) { // Count
				} else if(ps->id_size == 0x0408) { // Motion + Light
					wrk.motion_level = ps->data_ub[0];
					wrk.light = ps->data_uw >> 8;
					set_lm_out(1);
				} else if(ps->id_size == 0x0409) { // Light
					wrk.light = ps->data_uw & 0x00ffffff;
					set_lm_out(0);
				} else if(ps->id_size == 0x0111) { // Light on/off
					wrk.brightness_jump = ps->data_ub[0];
				}
				len -= ps->size + 2;
				ps = (padv_struct_qingping_t)((uint32_t)ps + ps->size + 2);
			}
		}
	}
}

_attribute_ram_code_
__attribute__((optimize("-Os")))
void filter_custom_ad(adv_custom_t *p) {
	if((p->size == sizeof(adv_custom_t) - 1)
	 && ((memcmp(dev1_MAC, p->MAC, 6) == 0)||(memcmp(dev2_MAC, p->MAC, 6) == 0))) {
		wrk.temp = p->temperature;
		wrk.humi = p->humidity;
		set_th_out();
		//new_trg = p->flags.trg_output;
		//new_rds = p->flags.rds_input;
		//gpio_write(GPIO_OUT_TH, p->flags.trg_output);
		wrk.flg.rds_output = p->flags.rds_input;
		gpio_write(GPIO_OUT_RDS, p->flags.rds_input);
	}
}

//////////////////////////////////////////////////////////
// scan event call back
//////////////////////////////////////////////////////////
_attribute_ram_code_
__attribute__((optimize("-Os")))
int scanning_event_callback(u32 h, u8 *p, int n) {
	if (h & HCI_FLAG_EVENT_BT_STD) { // ble controller hci event
		if ((h & 0xff) == HCI_EVT_LE_META) {
			//----- hci le event: le adv report event -----
			if (p[0] == HCI_SUB_EVT_LE_ADVERTISING_REPORT) { // ADV packet
				//after controller is set to scan state, it will report all the adv packet it received by this event
				event_adv_report_t *pa = (event_adv_report_t *) p;
				u32 adlen = pa->len;
				u8 rssi = pa->data[adlen];
				if (adlen && adlen < 32 && rssi != 0) { // rssi != 0
					u32 i = 0;
					while(adlen) {
						pad_uuid16_t pd = (pad_uuid16_t) &pa->data[i];
						u32 len = pd->size + 1;
						if(len <= adlen) {
							if(len >= sizeof(ad_uuid16_t) && pd->type == GAP_ADTYPE_SERVICE_DATA_UUID_16BIT) {
#if (ADV_SERVICE_ENABLE)
								advDataValue++;
								if (advDataCCC // Notify on?
										&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) {
									u8 *p = my_fifo_wptr(&ad_fifo);
									if(p) {
										p[0] = len + 1 + 6;
										p[1] = rssi;
										memcpy(p + 2, pa->mac, 6);
										memcpy(p + 2 + 6, pd, len);
										my_fifo_next(&ad_fifo);
									}
								}
#endif
								if((pd->uuid16) == 0x181A) { // GATT Service 0x181A Environmental Sensing, ATC custom FW
									filter_custom_ad((adv_custom_t *)pd);
								} else if((pd->uuid16) == 0xfe95) { // GATT Service: Xiaomi Inc.
									filter_xiaomi_ad((adv_xiaomi_t *)pd);
								} else if((pd->uuid16) == 0xfdcd) { // GATT Service: Qingping Technology (Beijing) Co., Ltd.
									filter_qingping_ad((adv_qingping_t *)pd);
								}
							}
						} else
							break;
						adlen -= len;
						i += len;
					}
				}
			}
		}
	}
	return 0;
}

//////////////////////////////////////////////////////////
// scan task
//////////////////////////////////////////////////////////
RAM uint32_t tisk_scan_task;

_attribute_ram_code_
__attribute__((optimize("-Os")))
void scan_task(void) {
	if(tisk_scan_task != utc_time_sec) {
		// new sec
		tisk_scan_task = utc_time_sec;
		if(wrk.motion_timer)
			wrk.motion_timer--;

		if (utc_time_sec - off_tisk_th >= 60) {
			gpio_write(GPIO_OUT_TH, 0);
			gpio_write(GPIO_OUT_RDS, 0);
			wrk.flg.rds_output = 0;
			wrk.flg.trg_output = 0;
			off_tisk_th = utc_time_sec;
		}
		if (utc_time_sec - off_tisk_lm >= 60) {
			gpio_write(GPIO_OUT_LM, 0);
			wrk.flg.lm_output = 0;
			off_tisk_lm = utc_time_sec;
		}
		if(memcmp(&adv_buf.wrk.data, &wrk, sizeof(wrk))) {
			set_adv_data();
			if (sppDataCCC // Notify on?
					&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) { // Connect?
				sppDataBuffer[0] = CMD_ID_INFO;
				memcpy(&sppDataBuffer[1], &wrk, sizeof(wrk));
				bls_att_pushNotifyData(SPP_Server2Client_DP_H, (u8 *)&sppDataBuffer, sizeof(wrk) + 1);
			}
		}
	}
#if (ADV_SERVICE_ENABLE)
	u8 *p = my_fifo_get(&ad_fifo);
	if(p) {
		if (advDataCCC // Notify on?
				&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) { // Connect?
			if(bls_att_pushNotifyData(ADV_DP_H, p+1, p[0]) == BLE_SUCCESS)
				my_fifo_pop(&ad_fifo);
		} else
			my_fifo_pop(&ad_fifo);
	}
#endif // ADV_SERVICE_ENABLE
}

