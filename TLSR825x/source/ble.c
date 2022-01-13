#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "app_config.h"
#include "drivers/8258/gpio_8258.h"
#include "ble.h"
#include "vendor/common/blt_common.h"
#include "cmd_parser.h"
#include "app.h"
#include "scaning.h"

void bls_set_advertise_prepare(void *p); // add ll_adv.h

RAM uint8_t ble_connected; // bit 0 - connected, bit 1 - conn_param_update, bit 2 - paring success, bit 7 - reset of disconnect
uint8_t ota_is_working = 0;

// uint8_t send_buf[MTU_DATA_SIZE];

RAM uint8_t blt_rxfifo_b[64 * 8] = { 0 };
RAM my_fifo_t blt_rxfifo = { 64, 8, 0, 0, blt_rxfifo_b, };
RAM uint8_t blt_txfifo_b[40 * 16] = { 0 };
RAM my_fifo_t blt_txfifo = { 40, 16, 0, 0, blt_txfifo_b, };

RAM uint8_t mac_public[6];
// uint8_t mac_random_static[6];

RAM adv_name_t adv_name = {
		.size = BLE_NAME_SIZE + 1,
		.type = GAP_ADTYPE_LOCAL_NAME_COMPLETE, // Complete local name
		.name = BLE_NAME
};

#if (BLE_DEVICE_ENABLE)
RAM adv_buf_t adv_buf = {
		.flag.size = 2,
		.flag.type = GAP_ADTYPE_FLAGS,
			/*	Flags:
			 	bit0: LE Limited Discoverable Mode
				bit1: LE General Discoverable Mode
				bit2: BR/EDR Not Supported
				bit3: Simultaneous LE and BR/EDR to Same Device Capable (Controller)
				bit4: Simultaneous LE and BR/EDR to Same Device Capable (Host)
				bit5..7: Reserved
			 */
		.flag.flg = 0x06,
		.wrk.size = sizeof(wrk) + 3,
		.wrk.type = GAP_ADTYPE_SERVICE_DATA_UUID_16BIT,
		.wrk.uuid16 = CHARACTERISTIC_UUID_ADV
};
#endif

#if	(OTA_SERVICE_ENABLE)
void app_enter_ota_mode(void) {
	// chk_ota_clear(); //
#ifdef NO_CLR_OTA_AREA
	bls_ota_clearNewFwDataArea();
#endif
	ota_is_working = 1;
	bls_ota_setTimeout(45 * 1000000); // set OTA timeout  45 seconds
	bls_pm_setManualLatency(0);
}
#endif // OTA_SERVICE_ENABLE

void ble_disconnect_callback(uint8_t e, uint8_t *p, int n) {
	if(ble_connected & 0x80) // reset device on disconnect?
		start_reboot();
	bls_pm_setManualLatency(0); // ?
	ble_connected = 0;
	ota_is_working = 0;
}

void ble_connect_callback(uint8_t e, uint8_t *p, int n) {
	ble_connected = 1;
	bls_l2cap_requestConnParamUpdate (
			my_updateConnParameters.intervalMin,
			my_updateConnParameters.intervalMax,
			my_updateConnParameters.latency,
			my_updateConnParameters.timeout
			);
	// bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(1000);
}

int app_conn_param_update_response(u8 id, u16  result) {
	if(result == CONN_PARAM_UPDATE_ACCEPT)
		ble_connected |= 2;
	else if(result == CONN_PARAM_UPDATE_REJECT) {
		// bls_l2cap_requestConnParamUpdate(160, 160, 4, 300); // (200 ms, 200 ms, 1 s, 3 s)
	}
	return 0;
}

/*
 * bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &ev_adv_timeout);
 * blt_event_callback_t(): */
_attribute_ram_code_ void ev_adv_timeout(u8 e, u8 *p, int n) {
	(void) e; (void) p; (void) n;
	bls_ll_setAdvParam(MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
			ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0, NULL,
			BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
	bls_ll_setAdvEnable(1);
}

_attribute_ram_code_
void set_adv_data(void) {
	memcpy(&adv_buf.wrk.data, &wrk, sizeof(wrk));
	bls_ll_setAdvData((u8 *)&adv_buf, sizeof(adv_buf));
}

extern attribute_t my_Attributes[ATT_END_H];
const char* hex_ascii = { "0123456789ABCDEF" };

__attribute__((optimize("-Os")))
void ble_set_default_name(void) {
	//Set the BLE Name to the last three MACs the first ones are always the same
	uint8_t *p = adv_name.name;
	*p++ = 'B';
	*p++ = 'L';
	*p++ = 'E';
	*p++ = '_';
	*p++ = hex_ascii[mac_public[2] >> 4];
	*p++ = hex_ascii[mac_public[2] & 0x0f];
	*p++ = hex_ascii[mac_public[1] >> 4];
	*p++ = hex_ascii[mac_public[1] & 0x0f];
	*p++ = hex_ascii[mac_public[0] >> 4];
	*p = hex_ascii[mac_public[0] & 0x0f];
}

#if BLE_SECURITY_ENABLE
int app_host_event_callback(u32 h, u8 *para, int n) {
	(void) para; (void) n;
	uint8_t event = (uint8_t)h;
	if (event == GAP_EVT_SMP_TK_DISPALY) { // PK_Resp_Dsply_Init_Input
			//u32 *pinCode = (u32*) para;
			uint32_t * p = (uint32_t *)&smp_param_own.paring_tk[0];
			memset(p, 0, sizeof(smp_param_own.paring_tk));
			p[0] = pincode;
#if 0
	} else if (event == GAP_EVT_SMP_PARING_SUCCESS) {
		gap_smp_paringSuccessEvt_t* p = (gap_smp_paringSuccessEvt_t*)para;
		if(p->bonding && p->bonding_result)  // paring success ?
			ble_connected |= 4;
	} else if (event == GAP_EVT_SMP_PARING_FAIL) {
		//gap_smp_paringFailEvt_t * p = (gap_smp_paringFailEvt_t *)para;
	} else if (event == GAP_EVT_SMP_TK_REQUEST_PASSKEY) {
		//blc_smp_setTK_by_PasskeyEntry(pincode);
	} else if (event == GAP_EVT_SMP_PARING_BEAGIN) {
		//gap_smp_paringBeginEvt_t * p = (gap_smp_paringBeginEvt_t*)para;
		// ...
	} else if(event == GAP_EVT_SMP_TK_REQUEST_OOB) {
		//blc_smp_setTK_by_OOB();
	} else if(event == GAP_EVT_SMP_TK_NUMERIC_COMPARE) {
		//uint32_t * pin = (uint32_t*)para;
		//blc_smp_setNumericComparisonResult(*pin == pincode);
	} else if(event == GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE) {
#endif
	}
	return 0;
}
#endif

__attribute__((optimize("-Os")))
void init_ble(void) {
#if (BLE_DEVICE_ENABLE)
	////////////////// BLE stack initialization //////////////////////
#if 1
	uint8_t mac_random_static[6];
	blc_initMacAddress(CFG_ADR_MAC, mac_public, mac_random_static);
#else
	generateRandomNum(5, mac_public);
	mac_public[5] = 0xC0; 			// for random static
#endif
	/// if bls_ll_setAdvParam( OWN_ADDRESS_RANDOM ) ->  blc_ll_setRandomAddr(mac_random_static);
	ble_set_default_name();
	////// Controller Initialization  //////////
	blc_ll_initBasicMCU(); //must
	blc_ll_initStandby_module(mac_public); //must
	blc_ll_initAdvertising_module(mac_public); // adv module: 		 must for BLE slave,
	blc_ll_initConnection_module(); // connection module  must for BLE slave/master
	blc_ll_initSlaveRole_module(); // slave module: 	 must for BLE slave,
	blc_ll_initPowerManagement_module(); //pm module:      	 optional
	////// Host Initialization  //////////
	blc_gap_peripheral_init();
	my_att_init(); //gatt initialization
	blc_l2cap_register_handler(blc_l2cap_packet_receive);
	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
#if BLE_SECURITY_ENABLE
	if(pincode) {
		//bls_smp_configParingSecurityInfoStorageAddr(0x074000);
		//bls_smp_eraseAllParingInformation();
		//blc_smp_param_setBondingDeviceMaxNumber(SMP_BONDING_DEVICE_MAX_NUM); //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM
#if 0
		//set security level: "LE_Security_Mode_1_Level_2"
		blc_smp_setSecurityLevel(Unauthenticated_Paring_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Paring_with_Encryption)
		blc_smp_setParingMethods(LE_Secure_Connection);
		blc_smp_setSecurityParamters(Bondable_Mode, 1, 0, 0, IO_CAPABLITY_NO_IN_NO_OUT);
		//blc_smp_setEcdhDebugMode(debug_mode); //use debug mode for sniffer decryption
#elif 1
		//set security level: "LE_Security_Mode_1_Level_3"
		blc_smp_setSecurityLevel(Authenticated_Paring_with_Encryption); //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Paring_with_Encryption)
		blc_smp_setParingMethods(LE_Secure_Connection);
		blc_smp_enableAuthMITM(1);
		//blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
		blc_smp_setIoCapability(IO_CAPABILITY_DISPLAY_ONLY);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT
#else
		//set security level: "LE_Security_Mode_1_Level_4"
		blc_smp_setSecurityLevel(Authenticated_LE_Secure_Connection_Paring_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Paring_with_Encryption)
		blc_smp_setParingMethods(LE_Secure_Connection);
		blc_smp_setSecurityParamters(Bondable_Mode, 1, 0, 0, IO_CAPABILITY_DISPLAY_ONLY);

#endif
		//Smp Initialization may involve flash write/erase(when one sector stores too much information,
		//   is about to exceed the sector threshold, this sector must be erased, and all useful information
		//   should re_stored) , so it must be done after battery check
		//Notice:if user set smp parameters: it should be called after usr smp settings
		blc_smp_peripheral_init();
		// Hid device on android7.0/7.1 or later version
		// New paring: send security_request immediately after connection complete
		// reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request.
		//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
		blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
		blc_gap_registerHostEventHandler(app_host_event_callback);
		blc_gap_setEventMask(GAP_EVT_MASK_SMP_TK_DISPALY
#if 0
				| GAP_EVT_MASK_SMP_PARING_BEAGIN
				| GAP_EVT_MASK_SMP_TK_NUMERIC_COMPARE
				| GAP_EVT_MASK_SMP_PARING_SUCCESS
				| GAP_EVT_MASK_SMP_PARING_FAIL
				| GAP_EVT_MASK_SMP_TK_REQUEST_PASSKEY
				| GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE
				| GAP_EVT_MASK_SMP_TK_REQUEST_OOB
#endif
				);
	} else
#endif
	blc_smp_setSecurityLevel(No_Security);

	///////////////////// USER application initialization ///////////////////
	bls_ll_setScanRspData((uint8_t *) &adv_name, adv_name.size+1);
	rf_set_power_level_index(MY_RF_POWER);
	// bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &user_set_rf_power);
	bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, &ble_connect_callback);
	bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, &ble_disconnect_callback);

	///////////////////// Power Management initialization///////////////////
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
	blc_pm_setDeepsleepRetentionThreshold(50, 30);
	blc_pm_setDeepsleepRetentionEarlyWakeupTiming(240);
	blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);

#if	(OTA_SERVICE_ENABLE)
#ifndef NO_CLR_OTA_AREA
	bls_ota_clearNewFwDataArea();
#endif
	bls_ota_registerStartCmdCb(app_enter_ota_mode);
#endif // OTA_SERVICE_ENABLE
	blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);
#if (MTU_DATA_SIZE > ATT_MTU_SIZE)
	blc_att_setRxMtuSize(MTU_DATA_SIZE); 	// If not set RX MTU size, default is: 23 bytes, max 247?
#endif // MTU_DATA_SIZE
	set_adv_data();
	// bls_set_advertise_prepare(app_advertise_prepare_handler);
	ev_adv_timeout(0,0,0);
/*
	blc_ll_addAdvertisingInConnSlaveRole();  //adv in conn slave role
	blc_ll_setAdvParamInConnSlaveRole((u8 *)&adv_buf, sizeof(adv_buf), \
										(u8 *)&adv_name, adv_name.size+1, \
										ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
*/
#else // not BLE_DEVICE_ENABLE
	blc_ll_initBasicMCU(); //must
	blc_ll_initPowerManagement_module(); //pm module:      	 optional
	///////////////////// Power Management initialization///////////////////
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
	blc_pm_setDeepsleepRetentionThreshold(50, 30);
	blc_pm_setDeepsleepRetentionEarlyWakeupTiming(240);
	blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
#endif // BLE_DEVICE_ENABLE
}

void start_adv_scanning(void) {
	//scan setting
	blc_ll_initScanning_module(mac_public);
	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(HCI_LE_EVT_MASK_ADVERTISING_REPORT);
	blc_hci_registerControllerEventHandler(scanning_event_callback); //controller hci event to host all processed in this func
	//set scan parameter and scan enable
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,
							  OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable(BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

	blc_ll_addScanningInAdvState();  //add scan in adv state
	blc_ll_addScanningInConnSlaveRole();  //add scan in conn slave role
}

#if (SPP_SERVICE_ENABLE)
void send_debug(void *psoo, int len) {
	if (sppDataCCC // Notify on?
			&& (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN)) { // Connect?
		sppDataBuffer[0] = 0xff;
		memcpy(&sppDataBuffer[1], psoo, len);
		bls_att_pushNotifyData(SPP_Server2Client_DP_H, (u8 *)&sppDataBuffer, len + 1);
	}
}
#endif
