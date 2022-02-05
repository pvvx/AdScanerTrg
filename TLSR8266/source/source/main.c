/********************************************************************************************************
 * @file     main.c
 *
 * @brief    for TLSR chips
 *
 * @author	 BLE Group
 * @date     May. 12, 2018
 *
 * @par      Copyright (c) Telink Semiconductor (Shanghai) Co., Ltd.
 *           All rights reserved.
 *
 *			 The information contained herein is confidential and proprietary property of Telink
 * 		     Semiconductor (Shanghai) Co., Ltd. and is available under the terms
 *			 of Commercial License Agreement between Telink Semiconductor (Shanghai)
 *			 Co., Ltd. and the licensee in separate contract or the terms described here-in.
 *           This heading MUST NOT be removed from this file.
 *
 * 			 Licensees are granted free, non-transferable use of the information in this
 *			 file under Mutual Non-Disclosure Agreement. NO WARRENTY of ANY KIND is provided.
 *
 *******************************************************************************************************/
#include "app.h"
#include "proj/mcu/watchdog_i.h"
#if	(USE_FLOADER)
#include "lfloader.h"
#endif

#if	(USE_UART_DEV)
	const dev_uart_cfg_t def_cfg_uart = {
			.baud = UART_BAUD/100, // x100
			.stop = 0,
			.parity = 0
	};
#endif

_attribute_ram_code_ void irq_handler(void)
{
	irq_blt_sdk_handler();
#if (USE_USB_CDC)
	USB_IrqHandle(reg_irq_src);
#endif
}

int main (void) {
#if USE_EXT_32K_CRYSTAL
	blc_pm_select_external_32k_crystal();
#else
	blc_pm_select_internal_32k_crystal();
#endif
	cpu_wakeup_init(CRYSTAL_TYPE);

	clock_init();

	gpio_init();

#if	(USE_FLOADER)
	// Срабатывает при старте, если уровень на SWS "0". Для работы необходимо записать usbfloader.bin в 0x71000 или uartfloader.bin в 0x72800
	load_floader();
#endif

	///NOTE:This function must be placed before the following function rf_drv_init().
	blc_app_loadCustomizedParameters();  //load customized freq_offset cap value and tp value

	ble_init();

#if (USE_USB_CDC)
	/* Initialize usb cdc */
	usb_init();
#endif

#if	(USE_UART_DEV)
	uart_init((dev_uart_cfg_t *)&def_cfg_uart);
#endif

	user_init();

    irq_enable();

   	while (1) {
#if (MODULE_WATCHDOG_ENABLE)
   		wd_clear(); //clear watch dog
#endif
		blt_sdk_main_loop();
  		main_loop();
    }
}
