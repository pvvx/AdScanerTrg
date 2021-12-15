/*
 * scaning.h
 *
 *  Created on: 20.11.2021
 *      Author: pvvx
 */

#ifndef SCANING_H_
#define SCANING_H_

extern uint8_t bindkey1[16]; // for MAC1
extern uint8_t bindkey2[16]; // for MAC2

int scanning_event_callback(u32 h, u8 *p, int n);
void scan_task(void);

#endif /* SCANING_H_ */
