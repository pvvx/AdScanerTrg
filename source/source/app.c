/*
 * app.c
 *
 *      Author: pvvx
 */
#include "app.h"
#include "flash_eep.h"

//----------------------------- BLE stack
#define MYFIFO_BLK_SIZE		64
MYFIFO_INIT(ad_fifo, MYFIFO_BLK_SIZE, 8); 	// 64*8 512 bytes + headers

typedef struct __attribute__((packed)) _ad_head_t {
    u8 len;
    u8 type;
    u16 uuid16;
    u8 data[1];
} ad_head_t, * pad_head_t;

// GPIO_TRG pin (marking "reset" on circuit board) flags:
typedef struct __attribute__((packed)) _trigger_flg_t {
	uint8_t 	trg_input	:	1; // GPIO_TRG pin input value (real level)
	uint8_t 	trg_output	:	1; // GPIO_TRG pin output value (pull Up/Down)
	uint8_t 	trigger_on	:	1; // Output GPIO_TRG pin is controlled according to the set parameters threshold temperature or humidity
	uint8_t 	temp_out_on :	1; // Temperature trigger event
	uint8_t 	humi_out_on :	1; // Humidity trigger event
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

u32 off_tisk;

void filter_my_ad(adv_custom_t *p) {
	adv_custom_t * z = (adv_custom_t *) &SppDataBuffer;
	if(memcmp(p, z, 10) == 0) {
		//memcpy(&test_ad.temperature, &p->temperature, sizeof(test_ad) - 10);
		gpio_write(GPIO_OUT1, p->flags.trg_output);
		gpio_write(GPIO_OUT2, p->flags.trg_input);
		memcpy(&z->temperature, &p->temperature, 9);
		off_tisk = clock_time();
#if 0
		if(z->counter != p->counter) {
			z->counter = p->counter;
			u8 *f = my_fifo_wptr(&ad_fifo);
			if(f) {
				f[0] = 9;
				memcpy(f + 1, &p->temperature, 9);
				my_fifo_next(&ad_fifo);
			}
		}
#endif
	}
}
//////////////////////////////////////////////////////////
// scan event call back
//////////////////////////////////////////////////////////
int scanning_event_callback(u32 h, u8 *p, int n) {
	if (h & HCI_FLAG_EVENT_BT_STD) { // ble controller hci event
		if ((h & 0xff) == HCI_EVT_LE_META) {
			//----- hci le event: le adv report event -----
			if (p[0] == HCI_SUB_EVT_LE_ADVERTISING_REPORT) { // ADV packet
				event_adv_report_t *pa = (event_adv_report_t *) p;
				u32 adlen = pa->len;
				u8 rssi = pa->data[adlen];
				if (adlen && rssi != 0) { // rssi != 0
					u32 i = 0;
					while(adlen) {
						ad_head_t * pd = (ad_head_t *) &pa->data[i];
						u32 len = pd->len + 1;
						if(len <= adlen) {
							if(len >= sizeof(ad_head_t) && pd->type == 0x16) {
#if 1
								if (SppDataCCC // Notify on?
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
								filter_my_ad((adv_custom_t *)pd);
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

void user_init() {
	adv_custom_t * z = (adv_custom_t *) &SppDataBuffer;
	z->size = 0x12;
	z->uid = 0x16;
	z->UUID = 0x181A;
	if((!flash_supported_eep_ver(0x1234, 0x1234))
			|| (!flash_read_cfg(&z->MAC, 0xACCC, 6))) {
	}

	// Добавить постоянное сканирование ADVERTISING
	{
		//scan setting
		blc_ll_initScanning_module(tbl_mac);
		blc_hci_registerControllerEventHandler(scanning_event_callback); //controller hci event to host all processed in this func
		blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_ADVERTISING_REPORT);
		//set scan paramter and scan enable
		blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,
								  OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
		blc_ll_setScanEnable(BLC_SCAN_ENABLE, FILTER_DUP_DISABLE);
		blc_ll_addScanningInAdvState();  //add scan in adv state
		blc_ll_addScanningInConnSlaveRole();  //add scan in conn slave role
	}
}


void main_loop() {
#if 1
	u32 tt = clock_time();
	if (tt - off_tisk >= 120 * CLOCK_SYS_CLOCK_1S) {
		gpio_write(GPIO_OUT1, 0);
		gpio_write(GPIO_OUT2, 0);
		off_tisk = tt;
	}
#endif
	u8 *p = my_fifo_get(&ad_fifo);
	if(p) {
#if SPP_SERVICE_ENABLE
		if (SppDataCCC // Notify on?
				&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) { // Connect?
#if (MTU_DATA_SIZE < MYFIFO_BLK_SIZE)
#error "MTU_DATA_SIZE < MYFIFO_BLK_SIZE !"
#endif
			if(bls_att_pushNotifyData(SPP_Server2Client_INPUT_DP_H, p+1, p[0]) == BLE_SUCCESS)
				my_fifo_pop(&ad_fifo);
		} else
			my_fifo_pop(&ad_fifo);
#endif
#if USE_UART_DEV
		if(!UartTxBusy()) {
#if (UART_TX_RX_DMA_BUFFER_SIZE - 4 < MYFIFO_BLK_SIZE)
#error "UART_TX_RX_DMA_BUFFER_SIZE < MYFIFO_BLK_SIZE !"
#endif
			uart_tx_buff[0] = p[0] + 1; // size
			memcpy(&uart_tx_buff[4], p, p[0] + 1);
			uart_send(uart_tx_buff);
		}
#endif
#if USE_USB_CDC
		if(USBCDC_IsAvailable()) {
#if (USB_TX_BUF_LEN < MYFIFO_BLK_SIZE)
#error "USB_TX_BUF_LEN < MYFIFO_BLK_SIZE !"
#endif
			memcpy(usb_buf_tx, p, p[0] + 1);
			USBCDC_DataSend(usb_buf_tx, p[0] + 1);
		}
#endif
//		my_fifo_pop(&ad_fifo);
	}
}

#if SPP_SERVICE_ENABLE
int onSppReceiveData(void *par) {
	rf_packet_att_data_t *pp = par;
	u8 len = pp->l2cap - 3;
	if (len >= 6) {//&& len <= sizeof(SppDataBuffer)) {
		adv_custom_t * z = (adv_custom_t *) & SppDataBuffer;
		memcpy(z->MAC, pp->dat, 6);
		flash_write_cfg(&z->MAC, 0xACCC, 6);
	}
	return 0;
}

int onSppSendData(void *par) {
	(void) par; // rf_packet_att_write_t * p
	//read(SppDataBuffer, 0, sizeof(SppDataBuffer));
	return 0;
}
#endif
