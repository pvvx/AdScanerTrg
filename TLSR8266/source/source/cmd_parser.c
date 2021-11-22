/*
 * cmd_parser.c
 *
 *  Created on: 13.11.2021
 *      Author: pvvx
 */
#include "app.h"
#include "cmd_parser.h"
#include "flash_eep.h"

#if SPP_SERVICE_ENABLE

__attribute__((optimize("-Os"))) void cmd_parser(void * p) {
	rf_packet_att_data_t *req = (rf_packet_att_data_t*) p;
	uint32_t len = req->l2cap - 3;
	if(len) {
		uint8_t cmd = req->dat[0];
		len--;
		SppDataBuffer[0] = cmd;
		SppDataBuffer[1] = 0; // no err?
		uint32_t olen = 1;
		if (cmd == CMD_ID_INFO) {
			memcpy(&SppDataBuffer[1], &wrk, sizeof(wrk));
			olen = sizeof(wrk);
		} else if (cmd == CMD_ID_MAC1) { // Get/set MAC1
			if(len == sizeof(dev1_MAC)) {
				memcpy(dev1_MAC, &req->dat[1], sizeof(dev1_MAC));
				flash_write_cfg(&dev1_MAC, DEV_MAC1_EID, sizeof(dev1_MAC));
			}
			memcpy(&SppDataBuffer[1], dev1_MAC, sizeof(dev1_MAC));
			olen = sizeof(dev1_MAC);
		} else if (cmd == CMD_ID_MAC2) { // Get/set MAC2
			if(len == sizeof(dev1_MAC)) {
				memcpy(&dev2_MAC, &req->dat[1], sizeof(dev2_MAC));
				flash_write_cfg(&dev2_MAC, DEV_MAC2_EID, sizeof(dev2_MAC));
			}
			memcpy(&SppDataBuffer[1], dev2_MAC, sizeof(dev2_MAC));
			olen = sizeof(dev2_MAC);
		} else if (cmd == CMD_ID_CFG) { // Get/set device config
			if(len == sizeof(dev_cfg)) {
				memcpy(&dev_cfg, &req->dat[1], sizeof(dev_cfg));
				flash_write_cfg(&dev_cfg, DEV_CFG_EID, sizeof(dev_cfg));
			}
			memcpy(&SppDataBuffer[1], &dev_cfg, sizeof(dev_cfg));
			olen = sizeof(dev_cfg);
		}
		if(olen)
			bls_att_pushNotifyData(SPP_Server2Client_INPUT_DP_H, SppDataBuffer, olen + 1);
	}
}

#endif
