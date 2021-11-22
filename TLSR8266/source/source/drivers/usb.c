/*
 * usb.c
 *
 *  Created on: 26.04.2021
 *      Author: pvvx
 */
#include "proj/tl_common.h"
#if USE_USB_CDC
#include "usb.h"
#include "proj_lib/pm.h" // reference usb_dp_pullup_en()

u8 usb_pwd = 0; // Events: USB_SET_CTRL_UART DTR Off, USB_PWDN, USB_RESET
u8 usb_pwup = 1; // Events: USB_SET_CTRL_UART DTR On

usb_buf_t usb_buf; // usb cdc buffers
//#define usb_buf_rx ((unsigned char *)&usb_buf.rx)
//#define usb_buf_tx ((unsigned char *)&usb_buf.tx)

unsigned char read_pkt[USB_RX_BUF_LEN]; // приемный буфер
u8 rx_len = 0; // flag - пришла команда в read_pkt
//unsigned char send_pkt[USB_TX_BUF_LEN]; // буфер отправки
//u8 tx_len = 0; // flag - есть данные для передачи в send_pkt

/* rxFunc rx callback function
 * Called from Irq (!) */
_attribute_ram_code_ void USBCDC_RxCb(unsigned char *data, unsigned int len){
	if (len) { // есть данные?
#ifdef USB_LED_RX
		USB_LED_RX();
#endif
		if(rx_len == 0
			&& data
			&& len <= sizeof(read_pkt)) {
			rx_len = len;
			memcpy(read_pkt, data, len);
		}
		USBCDC_RxBufSet(usb_buf_rx); // назначить новый буфер (в данном приложении единственный)
	}
}

/* Initialize usb cdc */
void usb_init(void) {
	USB_Init();
	USBCDC_RxBufSet(usb_buf_rx);
	USBCDC_CBSet(USBCDC_RxCb, NULL); // CDC_TxDoneCb);
	usb_dp_pullup_en(1);
}

#endif // #if USE_USB_CDC
