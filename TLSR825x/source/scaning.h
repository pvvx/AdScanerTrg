/*
 * scaning.h
 *
 *  Created on: 20.11.2021
 *      Author: pvvx
 */

#ifndef SCANING_H_
#define SCANING_H_

int scanning_event_callback(u32 h, u8 *p, int n);
void scan_task(void);

#endif /* SCANING_H_ */
