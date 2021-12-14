/*
 * cmd_parser.c
 *
 *  Created on: 13.11.2021
 *      Author: pvvx
 */
#include <stdint.h>
#include "tl_common.h"
#include "ble.h"
#include "cmd_parser.h"
#include "scaning.h"
#include "flash_eep.h"

#if SPP_SERVICE_ENABLE

__attribute__((optimize("-Os"))) int onSppReceiveData(void * p) {
	rf_packet_att_data_t *req = (rf_packet_att_data_t*) p;
	uint32_t len = req->l2cap - 3;
	if(len) {
		uint8_t cmd = req->dat[0];
		len--;
		sppDataBuffer[0] = cmd;
		sppDataBuffer[1] = 0; // no err?
		uint32_t olen = 1;
		if (cmd == CMD_ID_INFO) {
			memcpy(&sppDataBuffer[1], &wrk, sizeof(wrk));
			olen = sizeof(wrk);
		} else if (cmd == CMD_ID_UTC_TIME) { // Get/set utc time
			if(--len > sizeof(utc_time_sec)) len = sizeof(utc_time_sec);
			if(len)
				memcpy(&utc_time_sec, &req->dat[1], len);
			memcpy(&sppDataBuffer[1], &utc_time_sec, sizeof(utc_time_sec));
			olen = sizeof(utc_time_sec);
#if USE_TIME_ADJUST
		} else if (cmd == CMD_ID_TADJUST) { // Get/set adjust time clock delta (in 1/16 us for 1 sec)
			if(len > 2) {
				int16_t delta = req->dat[1] | (req->dat[2] << 8);
				utc_time_tick_step = CLOCK_16M_SYS_TIMER_CLK_1S + delta;
				flash_write_cfg(&utc_time_tick_step, EEP_ID_TIM, sizeof(utc_time_tick_step));
			}
			memcpy(&sppDataBuffer[1], &utc_time_tick_step, sizeof(utc_time_tick_step));
			olen = sizeof(utc_time_tick_step);
#endif
#if USE_BINDKEY
		} else if ((cmd == CMD_ID_BKEY1)||(cmd == CMD_ID_BKEY2)) { // Get/set beacon bindkey1
			uint8_t * pk = bindkey1;
			if(cmd&1)
				pk = bindkey2;
			if(len == 16) {
				memcpy(pk, &req->dat[1], 16);
				flash_write_cfg(pk, EEP_ID_KEY1+(cmd&1), 16);
			}
			if(flash_read_cfg(pk, EEP_ID_KEY1+(cmd&1), 16) == 16) {
				memcpy(&sppDataBuffer[1], pk, 16);
				olen = 16;
			} else { // No bindkey1 in EEP!
				sppDataBuffer[1] = 0xff;
			}
#endif
		} else if (cmd == CMD_ID_MAC1) { // Get/set MAC1
			if(len == sizeof(dev1_MAC)) {
				memcpy(dev1_MAC, &req->dat[1], sizeof(dev1_MAC));
				flash_write_cfg(&dev1_MAC, DEV_MAC1_EID, sizeof(dev1_MAC));
			}
			memcpy(&sppDataBuffer[1], dev1_MAC, sizeof(dev1_MAC));
			olen = sizeof(dev1_MAC);
		} else if (cmd == CMD_ID_MAC2) { // Get/set MAC2
			if(len == sizeof(dev1_MAC)) {
				memcpy(&dev2_MAC, &req->dat[1], sizeof(dev2_MAC));
				flash_write_cfg(&dev2_MAC, DEV_MAC2_EID, sizeof(dev2_MAC));
			}
			memcpy(&sppDataBuffer[1], dev2_MAC, sizeof(dev2_MAC));
			olen = sizeof(dev2_MAC);
		} else if (cmd == CMD_ID_CFG) { // Get/set device config
			if(len == sizeof(dev_cfg)) {
				memcpy(&dev_cfg, &req->dat[1], sizeof(dev_cfg));
				flash_write_cfg(&dev_cfg, DEV_CFG_EID, sizeof(dev_cfg));
			}
			memcpy(&sppDataBuffer[1], &dev_cfg, sizeof(dev_cfg));
			olen = sizeof(dev_cfg);
		} else if (cmd == CMD_ID_MTU && len > 1) { // Request Mtu Size Exchange
			if(req->dat[1] > ATT_MTU_SIZE)
				sppDataBuffer[1] = blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, req->dat[1]);
			else
				sppDataBuffer[1] = 0xff;
			olen = 2;
		} else if (cmd == CMD_ID_REBOOT) { // Set Reboot on disconnect
			ble_connected |= 0x80; // reset device on disconnect
			olen = 2;
		}
		if(olen)
			bls_att_pushNotifyData(SPP_Server2Client_DP_H, sppDataBuffer, olen + 1);
	}
	return 0;
}

#endif
