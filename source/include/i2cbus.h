/*
 * ina2xx.h
 *
 *  Created on: 14.01.2020
 *      Author: pvvx
 */

#ifndef _I2CBUS_H_
#define _I2CBUS_H_

/* Universal I2C/SMBUS read-write transaction struct */
typedef struct _i2c_utr_t {
	unsigned char mode;  // bit0..6: number wr_byte for new START (bit7: =1 - generate STOP/START)
	unsigned char rdlen; // bit0..6: number rd_byte (bit7: =1 - old read byte generate NACK, =0 - ACK)
	unsigned char wrdata[1]; // i2c_addr_wr, wr_byte1, wr_byte2, wr_byte3, ... wr_byte126
} i2c_utr_t;

void I2CBusInit(unsigned int clk);
void I2CBusDeInit(void);
int I2CBusReadWord(unsigned char i2c_addr, unsigned char reg_addr, void *preg_data);
int I2CBusWriteWord(unsigned char i2c_addr, unsigned char reg_addr, unsigned short reg_data);
int I2CBusUtr(void * outdata, i2c_utr_t *tr, unsigned int wrlen);

int I2CBusWrite(unsigned char i2c_addr, void *data, int wrlen , unsigned char stop);
int I2CBusRead(unsigned char i2c_addr, void *data, int rdlen, int wrlen);

void I2C_start_stop(unsigned char flg); // flg: FLD_I2C_CMD_STOP, FLD_I2C_CMD_START
int I2C_dev_addr(unsigned char i2c_addr); // return NAK
int I2C_wr(unsigned char reg_data, unsigned char flg); // return NAK, flg: 0, FLD_I2C_CMD_STOP
unsigned char I2C_rd(unsigned char flg); // flg: 0, FLD_I2C_CMD_STOP, FLD_I2C_CMD_NAK, FLD_I2C_CMD_START, FLD_I2C_CMD_ADR

int I2C_Write_A8(unsigned char i2c_addr, unsigned char reg_addr, void *data, int wrlen);
int I2C_Read_A8(unsigned char i2c_addr, unsigned char reg_addr, void *data, int rdlen, unsigned char nak);

#endif /* _I2CBUS_H_ */
