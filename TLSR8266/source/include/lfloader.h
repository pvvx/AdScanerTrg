/*
 * lfloader.h
 *
 *  Created on: 07.03.2020
 *      Author: pvvx
 */

#ifndef LFLOADER_H_
#define LFLOADER_H_

#define FLOADER_FADDR1 0x71000	// usbfloader
#define FLOADER_FADDR2 0x72800	// uartfloader
#define FLOADER_SIZE  0x01000

void load_floader(void); // Проверяет уровень на SWS "0". Для работы необходимо записать usbfloader.bin в 0x71000 или uartfloader.bin в 0x72800

#endif /* LFLOADER_H_ */
