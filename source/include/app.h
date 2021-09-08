/*
 * app.h
 *
 *      Author: pvvx
 */

#ifndef APP_H_
#define APP_H_

#include "ble.h"
#if (USE_USB_CDC)
#include "usb.h"
#endif
#if (USE_UART_DEV)
#include "uart_dev.h"
#endif

void user_init(void);
void main_loop(void);

#endif /* APP_H_ */
