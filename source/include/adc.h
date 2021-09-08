/*
 * adc.h
 *
 * Created on: 19.11.2019
 *      Author: pvvx
 */

#ifndef _ADC_H_
#define _ADC_H_

// Структура конфигурации опроса и инициализации устройства ADC
// Выходной пакет непрерывного опроса формируется по данному описанию
// CMD_DEV_CAD Get/Set CFG/ini ADC & Start measure
typedef struct __attribute__((packed)) _dev_adc_cfg_t {
	uint8_t pktcnt;	// минимальное кол-во передаваемых значений ADC в одном пакете передачи (автоподстройка до мах SMPS_BLK_CNT)
	uint8_t chnl; 	// Channel
	uint16_t sps; 	// период adc chl0
	uint8_t pga20db;
	uint8_t pga2db5;
} dev_adc_cfg_t; // [6]
extern dev_adc_cfg_t cfg_adc; // store in eep

extern u16 dfifo_rd_ptr; // dfifo_rd_ptr = reg_audio_wr_ptr - reinit ?
void init_adc(u16 per0, u8 per1, u8 vol, u8 scale, u8 chnl);
void deinit_adc(void);
unsigned int get_adc_dfifo_len(void);
int get_adc_dfifo(u16 * pbuf, unsigned int mincnt, unsigned int maxcnt);
//int init_adc_dfifo(u32 sps, u8 channel);
int init_adc_dfifo(dev_adc_cfg_t * p);

#endif /* _ADC_H_ */
