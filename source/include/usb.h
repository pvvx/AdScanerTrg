/*
 * usb.h
 *
 *  Created on: 22.02.2020
 *      Author: pvvx
 */

#ifndef USB_H_
#define USB_H_

////////// USB user events ////////////
extern u8 usb_pwd;
extern u8 usb_pwup;
#define USB_RESET() usb_pwd = 1 // Events: USB_SET_CTRL_UART DTR Off, USB_PWDN, USB_RESET
#define USB_PWDN() usb_pwd = 1 // Events: USB_SET_CTRL_UART DTR Off, USB_PWDN, USB_RESET
#define USB_SET_CTRL_UART(a) { if(a & 1) usb_pwup = 1; else usb_pwd = 1; } // DTR рукопожатия на линии

////////// USB buffers ////////////
#ifndef USB_CDC_MAX_RX_BLK_SIZE
#define USB_CDC_MAX_RX_BLK_SIZE	64
#endif
#ifndef USB_CDC_MAX_TX_BLK_SIZE
#define USB_CDC_MAX_TX_BLK_SIZE	64
#endif

// Define USB rx/tx buffer
#define USB_RX_BUF_LEN    USB_CDC_MAX_RX_BLK_SIZE // in bytes
#define USB_TX_BUF_LEN    USB_CDC_MAX_TX_BLK_SIZE // in bytes

// usb cdc buffers
typedef struct _usb_buf_t{
	unsigned char rx[USB_RX_BUF_LEN];
	unsigned char tx[USB_TX_BUF_LEN];
}usb_buf_t;
extern usb_buf_t usb_buf;

#define usb_buf_rx ((unsigned char *)&usb_buf.rx)
#define usb_buf_tx ((unsigned char *)&usb_buf.tx)

void USBCDC_RxCb(unsigned char *data, unsigned int len);
void usb_init(void);

#include "usbCDC/drivers.h"

#endif /* USB_H_ */
