/*
 * cmd_parser.h
 *
 *  Created on: 13.11.2021
 *      Author: pvvx
 */

#ifndef CMD_PARSER_H_
#define CMD_PARSER_H_

enum {
	CMD_ID_INFO     = 0x00, // Get Info
	CMD_ID_MAC1     = 0x40, // Get/set MAC1
	CMD_ID_MAC2     = 0x41, // Get/set MAC2
	CMD_ID_CFG      = 0x55,	// Get/set device config
} CMD_ID_KEYS;

void cmd_parser(void * p);

#endif /* CMD_PARSER_H_ */
