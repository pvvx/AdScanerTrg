/*
 * scaning.c
 *
 *  Created on: 20.11.2021
 *      Author: pvvx
 */
#include "app.h"
#include "ble.h"
#include "cmd_parser.h"

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

// Service 0xfe95 Xiaomi Inc.

//	https://iot.mi.com/new/doc/embedded-development/ble/object-definition
enum { // mijia ble version 5, General events
	MI_DATA_EV_Base					=0x0000,
	MI_DATA_EV_Connect				=0x0001,
	MI_DATA_EV_SimplrPair			=0x0002,
	MI_DATA_EV_Motion				=0x0003,
	MI_DATA_EV_KeepAway				=0x0004,
	MI_DATA_EV_LockObsolete			=0x0005,
	MI_DATA_EV_FingerPrint			=0x0006,
	MI_DATA_EV_Door					=0x0007,
	MI_DATA_EV_Armed				=0x0008,
	MI_DATA_EV_GestureController	=0x0009,
	MI_DATA_EV_BodyTemp				=0x000a,
	MI_DATA_EV_Lock					=0x000b,
	MI_DATA_EV_Flooding				=0x000c,
	MI_DATA_EV_Smoke				=0x000d,
	MI_DATA_EV_Gas					=0x000e,
	MI_DATA_EV_MovingWithLight		=0x000f, // Someone is moving (with light)
	MI_DATA_EV_ToothbrushIncident	=0x0010,
	MI_DATA_EV_CatEyeIncident		=0x0011,
	MI_DATA_EV_WeighingEvent		=0x0012,
	MI_DATA_EV_Button				=0x1001
} MI_DATA_EV;

enum { // mijia ble version 5
	MI_DATA_ID_Sleep				=0x1002,
	MI_DATA_ID_RSSI					=0x1003,
	MI_DATA_ID_Temperature			=0x1004,
	MI_DATA_ID_Humidity				=0x1006,
	MI_DATA_ID_LightIlluminance		=0x1007,
	MI_DATA_ID_SoilMoisture			=0x1008,
	MI_DATA_ID_SoilECvalue			=0x1009,
	MI_DATA_ID_Power				=0x100A,
	MI_DATA_ID_TempAndHumidity		=0x100D,
	MI_DATA_ID_Lock					=0x100E,
	MI_DATA_ID_Gate					=0x100F,
	MI_DATA_ID_Formaldehyde			=0x1010,
	MI_DATA_ID_Bind					=0x1011,
	MI_DATA_ID_Switch				=0x1012,
	MI_DATA_ID_RemAmCons			=0x1013, // Remaining amount of consumables
	MI_DATA_ID_Flooding				=0x1014,
	MI_DATA_ID_Smoke				=0x1015,
	MI_DATA_ID_Gas					=0x1016,
	MI_DATA_ID_NoOneMoves			=0x1017,
	MI_DATA_ID_LightIntensity		=0x1018,
	MI_DATA_ID_DoorSensor			=0x1019,
	MI_DATA_ID_WeightAttributes		=0x101A,
	MI_DATA_ID_NoOneMovesOverTime 	=0x101B, // No one moves over time
	MI_DATA_ID_SmartPillow			=0x101C
} MI_DATA_ID;

// All data little-endian
typedef struct __attribute__((packed)) _adv_xiaomi_t {
	uint8_t		size;	// = ?
	uint8_t		uid;	// = 0x16, 16-bit UUID
	uint16_t	UUID;	// = 0xfe95, GATT Service
#if 0
	union { // Frame Control
		struct __attribute__((packed)) {
			uint16_t Factory: 		1; //0001 reserved text
			uint16_t Connected: 	1; //0002 reserved text
			uint16_t Central: 		1; //0004 Keep
			uint16_t isEncrypted: 	1; //0008 0: The package is not encrypted; 1: The package is encrypted
			uint16_t MACInclude: 	1; //0010 0: Does not include the MAC address; 1: includes a fixed MAC address (the MAC address is included for iOS to recognize this device and connect)
			uint16_t CapabilityInclude: 	1;  //0020 0: does not include Capability; 1: includes Capability. Before the device is bound, this bit is forced to 1
			uint16_t ObjectInclude:	1; //0040 0: does not contain Object; 1: contains Object
			uint16_t Mesh: 			1; //0080 0: does not include Mesh; 1: includes Mesh. For standard BLE access products and high security level access, this item is mandatory to 0. This item is mandatory for Mesh access to 1.
			uint16_t registered:	1; //0100 0: The device is not bound; 1: The device is registered and bound.
			uint16_t solicited:		1; //0200 0: No operation; 1: Request APP to register and bind. It is only valid when the user confirms the pairing by selecting the device on the developer platform, otherwise set to 0. The original name of this item was bindingCfm, and it was renamed to solicited "actively request, solicit" APP for registration and binding
			uint16_t AuthMode:		2; //0c00 0: old version certification; 1: safety certification; 2: standard certification; 3: reserved
			uint16_t version:		4; //f000 Version number (currently v5)
		} b; // bits
		uint16_t	word;	// Frame Control
	} ctrID; // Frame Control
#else
	uint16_t	ctrID;	// Frame Control
#endif
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

u32 off_tisk_th;
u32 off_tisk_lm;

#include "aes_ccm.h"

/* Encrypted mijia beacon structs */
typedef struct __attribute__((packed)) _mi_beacon_nonce_t{
    uint8_t  mac[6];
	uint16_t pid;
	union {
		struct {
			uint8_t  cnt;
			uint8_t  ext_cnt[3];
		};
		uint32_t cnt32;
    };
} mi_beacon_nonce_t, * pmi_beacon_nonce_t;

/* Encrypted pvvx beacon structs */
typedef struct __attribute__((packed)) _adv_cust_head_t {
	uint8_t		size;		//@0 = 11
	uint8_t		uid;		//@1 = 0x16, 16-bit UUID https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/
	uint16_t	UUID;		//@2..3 = GATT Service 0x181A Environmental Sensing (little-endian) (or 0x181C 'User Data'?)
	uint8_t		counter;	//@4 0..0xff Measurement count, Serial number, used for de-duplication, different event or attribute reporting requires different Frame Counter
} adv_cust_head_t, * padv_cust_head_t;

typedef struct __attribute__((packed)) _adv_cust_data_t {
	int16_t		temp;		//@0
	uint16_t	humi;		//@2
	uint8_t		bat;		//@4
	trigger_flg_t	flags;	//@5
} adv_pvvx_data_t, * padv_pvvx_data_t;

typedef struct __attribute__((packed)) _adv_pvvx_enc_t {
	adv_cust_head_t head;	//@0
	adv_pvvx_data_t data;   //@5
	uint8_t		mic[4];		//@11
} adv_pvvx_enc_t, * padv_pvvx_enc_t;

/* Encrypted atc beacon structs
 * https://github.com/pvvx/ATC_MiThermometer/issues/94#issuecomment-842846036 */
typedef struct __attribute__((packed)) _adv_atc_data_t {
	uint8_t		temp;		//@0
	uint8_t		humi;		//@1
	uint8_t		bat;		//@2
} adv_atc_data_t, * padv_atc_data_t;

typedef struct __attribute__((packed)) _adv_atc_enc_t {
	adv_cust_head_t head;	//@0
	adv_atc_data_t data;	//@5
	uint8_t		mic[4];		//@8..11
} adv_atc_enc_t, * padv_atc_enc_t;

/* Encrypted atc/custom nonce */
typedef struct __attribute__((packed)) _enc_beacon_nonce_t{
    uint8_t  MAC[6];
    adv_cust_head_t head;
} enc_beacon_nonce_t;

const uint8_t ccm_aad = 0x11;
uint8_t bindkey1[16]; // for MAC1
uint8_t bindkey2[16]; // for MAC2

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
void set_lm_out(uint8_t motion_event) {
	if(dev_cfg.illuminance_hysteresis) {
		if(wrk.flg.light_event) { // lm_event on
			if(dev_cfg.illuminance_hysteresis < 0) {
				if(wrk.illuminance > dev_cfg.illuminance_threshold - dev_cfg.illuminance_hysteresis) {
					wrk.flg.light_event = false;
				}
			} else {
				if(wrk.illuminance < dev_cfg.illuminance_threshold - dev_cfg.illuminance_hysteresis) {
					wrk.flg.light_event = false;
				}
			}
		} else { // lm_event off
			if(dev_cfg.illuminance_hysteresis < 0) {
				if(wrk.illuminance < dev_cfg.illuminance_threshold + dev_cfg.illuminance_hysteresis) {
					wrk.flg.light_event = true;
				}
			} else {
				if(wrk.illuminance > dev_cfg.illuminance_threshold + dev_cfg.illuminance_hysteresis) {
					wrk.flg.light_event = true;
				}
			}
		}
	} else wrk.flg.light_event = false;

	if(wrk.motion_timer) { // свет включен по событию движения
		if(motion_event && dev_cfg.motion_timer) { // обнаружено движение и включен опрос движения
			wrk.flg.lm_output = true;	// включить LM
			wrk.motion_timer = dev_cfg.motion_timer; // обновить таймер
		} // не менять состояние выхода LM
	} else { // свет ещё не включен по событию движения
		if(motion_event && dev_cfg.motion_timer) { // обнаружено движение и включен опрос движения
			if(wrk.flg.light_event // темно?
				|| dev_cfg.illuminance_hysteresis == 0) {  // или не задано слежение за освещением?
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
void filter_xiaomi_ad(padv_xiaomi_t p, uint8_t *mac) {
	int len = p->size;
	uint8_t * pb;
	uint8_t * pmac;
	if(len > sizeof(adv_xiaomi_t)- 6) {
		uint8_t * pbkey = NULL;
		if(p->ctrID & 0x0010) { // MAC presents
			len -= sizeof(adv_xiaomi_t) - 4;
			pmac = p->MAC;
			pb = &p->cap;
		} else if(len > sizeof(adv_xiaomi_t)) {
			len -= sizeof(adv_xiaomi_t) - 4 - 6;
			pmac = mac;
			pb = p->MAC;
		} else
			return;
		if(memcmp(dev1_MAC, mac, sizeof(dev1_MAC)) == 0)
			pbkey = bindkey1;
		else if(memcmp(dev2_MAC, mac, sizeof(dev2_MAC)) == 0)
			pbkey = bindkey2;
		else
			return;
		if(((p->ctrID & (0x40 | 0x80)) == 0x40)) { // No Mesh, Data presents
			if(p->ctrID & 0x20) { // includes Capability
				if (*pb++ & 0x20) {
					pb += 2;
					len -= 2;
				}
				len --;
			}
			if((p->ctrID & 0x08) && len > 3+3+4) { // Data encrypted, len > size (min_data[3], ext_cnt[3], mic[4])
				mi_beacon_nonce_t beacon_nonce;
				memcpy(&beacon_nonce.mac, pmac, sizeof(beacon_nonce.mac));
				len -= 3+4; // - size (ext_cnt[3], mic[4])
				beacon_nonce.pid = p->devID;
				beacon_nonce.cnt = p->counter;
				beacon_nonce.ext_cnt[0] = pb[len];
				beacon_nonce.ext_cnt[1] = pb[len+1];
				beacon_nonce.ext_cnt[2] = pb[len+2];
				if(aes_ccm_auth_decrypt((const unsigned char *)pbkey,
						(uint8_t*)&beacon_nonce, sizeof(beacon_nonce),
						&ccm_aad, sizeof(ccm_aad),
						pb, len, // crypt_data
						pb, // decrypt data
						(uint8_t *)&pb[len+3], 4)) { // &mic: &crypt_data[len + size (ext_cnt[3])]
					return;
				}
			}
			// send_debug(pb, len);
			padv_struct_xiaomi_t ps = (padv_struct_xiaomi_t)pb;
			while(ps->size + 3 <= len) {
				if((ps->id == MI_DATA_EV_Motion)&&(ps->size >= 1)) { // Motion
					set_lm_out(ps->data_ub[0]);
				} else if((ps->id == MI_DATA_EV_MovingWithLight)&&(ps->size >= 3)) { // Moving With Light 0f0003 540f00 / 0f0003 620e00
					wrk.illuminance = ps->data_us[0];
					wrk.motion_event = 1;
					set_lm_out(1);
				} else if((ps->id == MI_DATA_ID_Temperature)&&(ps->size >= 2)) { // Temperature
					wrk.temp = ps->data_is[0]*10; // in 0.1 C
					set_th_out();
				} else if((ps->id == MI_DATA_ID_Humidity)&&(ps->size >= 2)) { // Humidity
					wrk.humi = ps->data_is[0]*10;  // in 0.1 %
					set_th_out();
				} else if((ps->id == MI_DATA_ID_LightIlluminance)&&(ps->size >= 3)) { // Light Illuminance 071003 af1500
					wrk.illuminance = ps->data_us[0];  // in ?
					set_lm_out(0);
				} else if((ps->id == MI_DATA_ID_TempAndHumidity)&&(ps->size >= 4)) { // Temp + Humi
					wrk.temp = ps->data_is[0]*10; // in 0.1 C
					wrk.humi = ps->data_is[1]*10; // in 0.1 %
					set_th_out();
				} else if((ps->id == MI_DATA_ID_NoOneMoves)&&(ps->size >= 4)) { // No one moves over time / 171004 3c000000 / 171004 78000000 / 1710042c010000 / 171004 58020000
					wrk.motion_event = ps->data_uw == 0;
					set_lm_out(wrk.motion_event);
				} else if((ps->id == MI_DATA_ID_LightIntensity)&&(ps->size >= 1)) { // Light on/off, Light Intensity 18100101 / 18100100
					wrk.light_on = ps->data_ub[0];
				}
				len -= ps->size + 3;
				ps = (padv_struct_xiaomi_t)((uint32_t)ps + ps->size + 3);
			}
		}
	}
}

_attribute_ram_code_
__attribute__((optimize("-Os")))
void filter_qingping_ad(padv_qingping_t p, uint8_t *mac) {
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
					wrk.motion_event = ps->data_ub[0];
					wrk.illuminance = ps->data_uw >> 8;
					set_lm_out(wrk.motion_event);
				} else if(ps->id_size == 0x0409) { // Light
					wrk.illuminance = ps->data_uw & 0x00ffffff;
					set_lm_out(0);
				} else if(ps->id_size == 0x0111) { // Light on/off
					wrk.light_on = ps->data_ub[0];
				}
				len -= ps->size + 2;
				ps = (padv_struct_qingping_t)((uint32_t)ps + ps->size + 2);
			}
		}
	}
}

_attribute_ram_code_
__attribute__((optimize("-Os")))
void filter_custom_ad(adv_custom_t *p, uint8_t *mac) {
	if((p->size == sizeof(adv_custom_t) - 1)
	 && ((memcmp(dev1_MAC, p->MAC, sizeof(dev1_MAC)) == 0)||(memcmp(dev2_MAC, p->MAC, sizeof(dev2_MAC)) == 0))) {
		wrk.temp = p->temperature;
		wrk.humi = p->humidity;
		set_th_out();
		//new_trg = p->flags.trg_output;
		//new_rds = p->flags.rds_input;
		//gpio_write(GPIO_OUT_TH, p->flags.trg_output);
		wrk.flg.rds_output = p->flags.rds_input;
		gpio_write(GPIO_OUT_RDS, p->flags.rds_input);
	}
	else if(p->size == sizeof(adv_pvvx_enc_t) - 1) {
		uint8_t * pbkey = NULL;
		if(memcmp(dev1_MAC, mac, sizeof(dev1_MAC)) == 0)
			pbkey = bindkey1;
		else if(memcmp(dev2_MAC, mac, sizeof(dev2_MAC)) == 0)
			pbkey = bindkey2;
		if(pbkey != NULL) {
				enc_beacon_nonce_t beacon_nonce;
				// unsigned int len = p->size - 4 - (sizeof(adv_cust_head_t)-1); // - mic[4] - head
				padv_pvvx_enc_t pp = (padv_pvvx_enc_t) p;
				memcpy(&beacon_nonce.MAC, mac, sizeof(beacon_nonce.MAC));
				beacon_nonce.head = pp->head; // memcpy(&beacon_nonce.head, pp->head, sizeof(adv_cust_head_t));
				// uint8_t decrypt_data[16];
				if(aes_ccm_auth_decrypt((const unsigned char *)pbkey,
						(uint8_t*)&beacon_nonce, sizeof(beacon_nonce),
						&ccm_aad, sizeof(ccm_aad),
						(uint8_t *)&pp->data, sizeof(adv_pvvx_data_t), // len crypt_data
						(uint8_t *)&pp->data, // decrypt data
						(uint8_t *)&pp->mic, 4)) { // &mic: &crypt_data[len + size (ext_cnt[3])]
					return;
				}
				//ps = (padv_struct_xiaomi_t)&decrypt_data;
				wrk.temp = pp->data.temp;
				wrk.humi = pp->data.humi;
				set_th_out();
				//new_trg = pp->data.flags.trg_output;
				//new_rds = pp->data.flags.rds_input;
				//gpio_write(GPIO_OUT_TH, p->flags.trg_output);
				wrk.flg.rds_output = pp->data.flags.rds_input;
				gpio_write(GPIO_OUT_RDS, p->flags.rds_input);
		}
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
								if (advNotifyCCC // Notify on?
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
									filter_custom_ad((adv_custom_t *)pd, pa->mac);
								} else if((pd->uuid16) == 0xfe95) { // GATT Service: Xiaomi Inc.
									filter_xiaomi_ad((adv_xiaomi_t *)pd, pa->mac);
								} else if((pd->uuid16) == 0xfdcd) { // GATT Service: Qingping Technology (Beijing) Co., Ltd.
									filter_qingping_ad((adv_qingping_t *)pd, pa->mac);
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
uint32_t tisk_scan_task;

#define	OUT_OFF_TIMEOUT	(15*60) // 15 minutes

_attribute_ram_code_
__attribute__((optimize("-Os")))
void scan_task(void) {
	if(tisk_scan_task != utc_time_sec) {
		// new sec
		tisk_scan_task = utc_time_sec;
		if(wrk.motion_timer)
			wrk.motion_timer--;
		if (utc_time_sec - off_tisk_th >= OUT_OFF_TIMEOUT) {
			gpio_write(GPIO_OUT_TH, 0);
			gpio_write(GPIO_OUT_RDS, 0);
			wrk.flg.rds_output = 0;
			wrk.flg.trg_output = 0;
			off_tisk_th = utc_time_sec;
		}
		if (utc_time_sec - off_tisk_lm >= OUT_OFF_TIMEOUT) {
			gpio_write(GPIO_OUT_LM, 0);
			wrk.flg.lm_output = 0;
			off_tisk_lm = utc_time_sec;
		}
#if (SPP_SERVICE_ENABLE)
		if(memcmp(&adv_buf.wrk.data, &wrk, sizeof(wrk))) {
			set_adv_data();
			if (SppNotifyCCC // Notify on?
					&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) { // Connect?
				SppDataBuffer[0] = CMD_ID_INFO;
				memcpy(&SppDataBuffer[1], &wrk, sizeof(wrk));
				bls_att_pushNotifyData(SPP_Server2Client_DP_H, (u8 *)&SppDataBuffer, sizeof(wrk) + 1);
			}
		}
#endif
	}
#if (ADV_SERVICE_ENABLE)
	u8 *p = my_fifo_get(&ad_fifo);
	if(p) {
		if (advNotifyCCC // Notify on?
				&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) { // Connect?
			if(bls_att_pushNotifyData(ADV_DP_H, p+1, p[0]) == BLE_SUCCESS)
				my_fifo_pop(&ad_fifo);
		} else
			my_fifo_pop(&ad_fifo);
	}
#endif // ADV_SERVICE_ENABLE
}

