/*
 * ds3231.h
 *
 *  Created on: 11.03.2021
 *      Author: pvvx
 */

#ifndef DS3231_H_
#define DS3231_H_

#include "i2cbus.h"

#define DEV_I2C_ADDR (0x68<<1)

typedef struct { // clock
	u8 Seconds;
	u8 Minutes;
	u8 Hour;
	u8 Day;
	u8 Date;
	u8 Month;
	u8 Year;
} t_ds3231_clock;

inline int ds3231_write(u8 * buf, u8 reg_addr, u8 len) {
	return I2C_Write_A8(DEV_I2C_ADDR, reg_addr, buf, len);
}

inline int ds3231_init() {
	I2CBusInit(400);
	return I2CBusWriteWord(DEV_I2C_ADDR, 0x0e, 0x0088);
}

inline int ds3231_read(void * buf, u8 reg_addr, u8 len) {
	return I2C_Read_A8(DEV_I2C_ADDR, reg_addr, buf, len, 1);
}

#endif /* DS3231_H_ */
