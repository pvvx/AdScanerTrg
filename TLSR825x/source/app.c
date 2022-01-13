#include <stdint.h>
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "vendor/common/blt_common.h"
#include "cmd_parser.h"
#include "battery.h"
#include "ble.h"
#include "app.h"
#include "scaning.h"
#include "flash_eep.h"

void app_enter_ota_mode(void);

#if BLE_SECURITY_ENABLE
RAM uint32_t pincode;
#endif

RAM wrk_data_t wrk;
RAM dev_cfg_t dev_cfg;
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
RAM uint16_t battery_mv;    // 2200..3300 mV
RAM volatile uint8_t send_measure; // measure complete
#define measurement_step_time 10 // sec
RAM uint32_t tim_measure; // timer measurements >= 10 sec
RAM uint32_t utc_time_sec;	// clock in sec (= 0 -> 1970-01-01 00:00:00)
#if USE_TIME_ADJUST
RAM uint32_t utc_time_tick_step = CLOCK_16M_SYS_TIMER_CLK_1S; // adjust time clock (in 1/16 us for 1 sec)
#else
#define utc_time_tick_step CLOCK_16M_SYS_TIMER_CLK_1S
#endif
RAM uint32_t utc_time_sec_tick;

uint8_t	dev1_MAC[6]; // [0] - lo, .. [6] - hi digits
uint8_t	dev2_MAC[6]; // [0] - lo, .. [6] - hi digits

//--- check battery
_attribute_ram_code_ void check_battery(uint16_t bmv) {
	battery_mv = get_battery_mv();
	if (battery_mv < bmv) {
		cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER,
			clock_time() + 120 * CLOCK_16M_SYS_TIMER_CLK_1S); // go deep-sleep 2 minutes
	}
#if	(BATT_SERVICE_ENABLE)
	battery_level = get_battery_level(battery_mv);
#endif
}
//------------------ user_init_normal -------------------
void user_init_normal(void) {
//this will get executed one time after power up
	check_battery(MIN_VBAT_MV); // 2.2V
	random_generator_init(); //must
	if(flash_supported_eep_ver(0x1001, 0x1000+SW_VERSION)) {
		flash_read_cfg(&dev1_MAC, DEV_MAC1_EID, sizeof(dev1_MAC));
		flash_read_cfg(&dev2_MAC, DEV_MAC2_EID, sizeof(dev2_MAC));
		if(flash_read_cfg(&dev_cfg, DEV_CFG_EID, sizeof(dev_cfg)) != sizeof(dev_cfg))
			memcpy(&dev_cfg, &def_dev_cfg, sizeof(dev_cfg));
#if USE_TIME_ADJUST
		if(flash_read_cfg(&utc_time_tick_step, EEP_ID_TIM, sizeof(utc_time_tick_step)) != sizeof(utc_time_tick_step))
			utc_time_tick_step = CLOCK_16M_SYS_TIMER_CLK_1S;
#endif
		flash_read_cfg(&bindkey1, EEP_ID_KEY1, sizeof(bindkey1));
		flash_read_cfg(&bindkey2, EEP_ID_KEY2, sizeof(bindkey2));
#if BLE_SECURITY_ENABLE
		if(flash_read_cfg(&pincode, EEP_ID_PCD, sizeof(pincode)) != sizeof(pincode))
			pincode = 0;
#endif
	} else {
		memcpy(&dev_cfg, &def_dev_cfg, sizeof(dev_cfg));
#if USE_TIME_ADJUST
			utc_time_tick_step = CLOCK_16M_SYS_TIMER_CLK_1S;
#endif
#if BLE_SECURITY_ENABLE
			pincode = 0;
#endif
	}
	init_ble();
	start_adv_scanning();
//	bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_EXIT, &suspend_exit_cb);
//	bls_app_registerEventCallback(BLT_EV_FLAG_SUSPEND_ENTER, &suspend_enter_cb);
}

//------------------ user_init_deepRetn -------------------
_attribute_ram_code_ void user_init_deepRetn(void) {
//after sleep this will get executed
	blc_ll_initBasicMCU();
	rf_set_power_level_index(MY_RF_POWER);
	blc_ll_recoverDeepRetention();
#if (OTA_SERVICE_ENABLE)
	bls_ota_registerStartCmdCb(app_enter_ota_mode);
#endif
}

//----------------------- main_loop()
_attribute_ram_code_ void main_loop(void) {
	blt_sdk_main_loop();
	while(clock_time() -  utc_time_sec_tick > utc_time_tick_step) {
		utc_time_sec_tick += utc_time_tick_step;
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
#endif
		if (utc_time_sec - tim_measure >= measurement_step_time) {
#if	(BATT_SERVICE_ENABLE)
			send_measure = 1;
#endif
			tim_measure = utc_time_sec;
			check_battery(MIN_VBAT_MV);
		}
		scan_task();
#if	(OTA_SERVICE_ENABLE)
	}
#endif
	// bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
	// bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
}
