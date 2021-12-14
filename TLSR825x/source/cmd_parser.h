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
#if USE_BINDKEY
	CMD_ID_BKEY1 	= 0x18, // Get/set beacon bindkey1 in EEP (for MAC1)
	CMD_ID_BKEY2	= 0x19, // Get/set beacon bindkey2 in EEP (for MAC2)
#endif
	CMD_ID_UTC_TIME = 0x23, // Get/set utc time
	CMD_ID_TADJUST  = 0x24, // Get/set adjust time clock delta (in 1/16 us for 1 sec)
	CMD_ID_MAC1     = 0x40, // Get/set MAC1
	CMD_ID_MAC2     = 0x41, // Get/set MAC2
	CMD_ID_CFG      = 0x55,	// Get/set device config
//	CMD_ID_PINCODE  = 0x70, // Set new PinCode 0..999999
	CMD_ID_MTU		= 0x71, // Request Mtu Size Exchange (23..255)
	CMD_ID_REBOOT	= 0x72, // Set Reboot on disconnect
} CMD_ID_KEYS;


#endif /* CMD_PARSER_H_ */
