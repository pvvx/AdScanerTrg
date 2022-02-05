/*
 * app.c
 *
 *      Author: pvvx
 */
#include "app.h"
#include "scaning.h"
#include "cmd_parser.h"
#include "flash_eep.h"


uint8_t	dev1_MAC[6]; // [0] - lo, .. [6] - hi digits
uint8_t	dev2_MAC[6]; // [0] - lo, .. [6] - hi digits

u32 utc_time_sec, utc_time_sec_tick;
volatile u8 send_measure; // measure complete
#define measurement_step_time 10 // sec
u32 tim_measure; // timer measurements >= 10 sec
#define UPDATE_CONN_PARAM CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_1S
gap_periConnectParams_t my_updateConnParameters = { UPDATE_CONN_PARAM };
u8 ble_connected = 0;

wrk_data_t wrk;
dev_cfg_t dev_cfg;

adv_buf_t adv_buf = {
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

const dev_cfg_t def_dev_cfg = {
		.temp_threshold = 2200, // x0.01°, Set temp threshold
		.temp_hysteresis = -1, // Set temp hysteresis, -327.67..327.67 °
		.humi_threshold = 5000, // x0.01%, Set humi threshold
		.humi_hysteresis = 0, // Set humi hysteresis, -327.67..327.67 %
		.illuminance_threshold = 200,
		.illuminance_hysteresis = -50,
		.motion_timer = 15,
		.flg.reserved = 0
};

#if	(BATT_SERVICE_ENABLE)
uint16_t battery_mv;    // 2200..3300 mV
//--- check battery
void check_battery(uint16_t bmv) {
	battery_mv = get_battery_mv();
	if (battery_mv < bmv) {
		cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER,
			clock_time() + 120 * CLOCK_16M_SYS_TIMER_CLK_1S); // go deep-sleep 2 minutes
	}
	battery_level = get_battery_level(battery_mv);
}
#endif


void set_adv_data(void) {
	memcpy(&adv_buf.wrk.data, &wrk, sizeof(wrk));
	bls_ll_setAdvData((u8 *)&adv_buf, sizeof(adv_buf));
}

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

void user_init() {
	if(flash_supported_eep_ver(0x1001, DEV_VERSION)) {
		flash_read_cfg(&dev1_MAC, DEV_MAC1_EID, sizeof(dev1_MAC));
		flash_read_cfg(&dev2_MAC, DEV_MAC2_EID, sizeof(dev2_MAC));
		if(flash_read_cfg(&dev_cfg, DEV_CFG_EID, sizeof(dev_cfg)) != sizeof(dev_cfg))
			memcpy(&dev_cfg, &def_dev_cfg, sizeof(dev_cfg));
		flash_read_cfg(&bindkey1, EEP_ID_KEY1, sizeof(bindkey1));
		flash_read_cfg(&bindkey2, EEP_ID_KEY2, sizeof(bindkey2));
	} else {
		memcpy(&dev_cfg, &def_dev_cfg, sizeof(dev_cfg));
	}
	bls_app_registerEventCallback(BLT_EV_FLAG_CONNECT, &ble_connect_callback);
	bls_app_registerEventCallback(BLT_EV_FLAG_TERMINATE, &ble_disconnect_callback);
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
	u32 tt = clock_time();
	while(tt -  utc_time_sec_tick > CLOCK_SYS_CLOCK_1S) {
		utc_time_sec_tick += CLOCK_SYS_CLOCK_1S;
		utc_time_sec++; // + 1 sec
	}

#if	(OTA_SERVICE_ENABLE)
	if (!ota_is_working) {
#endif
#if	(BATT_SERVICE_ENABLE)
		if(send_measure) {
			if (batteryValueInCCC && (blc_ll_getCurrentState() & BLS_LINK_STATE_CONN))
					ble_send_battery();
			send_measure = 0;
		}
		if (utc_time_sec - tim_measure >= measurement_step_time) {
			send_measure = 1;
			tim_measure = utc_time_sec;
			check_battery(MIN_VBAT_MV);
		}
#endif
		scan_task();
#if	(OTA_SERVICE_ENABLE)
	}
#endif
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
}

#if SPP_SERVICE_ENABLE

int onSppSendData(void *par) {
	(void) par; // rf_packet_att_write_t * p
	//read(SppDataBuffer, 0, sizeof(SppDataBuffer));
	return 0;
}
#endif
