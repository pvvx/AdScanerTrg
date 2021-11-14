/*
 * ble.h
 *
 *  Created on: 22.02.2020
 *      Author: pvvx
 */

#ifndef BLE_H_
#define BLE_H_

#include "proj/tl_common.h"
#include "proj_lib/rf_drv.h"
#include "proj_lib/pm.h"
#include "proj_lib/ble/ll/ll.h"
#include "proj_lib/ble/hci/hci.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/trace.h"
#include "proj_lib/ble/service/ble_ll_ota.h"
#include "proj_lib/ble/blt_config.h"
#include "proj_lib/ble/ble_smp.h"
#include "proj_lib/ble/ble_common.h"

#ifndef MTU_DATA_SIZE
#define	MTU_DATA_SIZE	64 // ATT_MTU_SIZE // = 23
#endif

#ifndef DEF_CONN_PARMS
#define DEF_CONN_PARMS CONN_INTERVAL_10MS, CONN_INTERVAL_100MS, 0, CONN_TIMEOUT_1S
#endif

#ifndef DEF_ADV_INTERVAL
#define DEF_ADV_INTERVAL ADV_INTERVAL_1_28_S, ADV_INTERVAL_1_28_S + 16
#endif

#if (SPP_SERVICE_ENABLE)
#define SERVICE_UUID_SPP 		0xffe0
#define CHARACTERISTIC_UUID_SPP (SERVICE_UUID_SPP+1)
#define SPP_SERVICE_BUF_LEN		20 // MTU_DATA_SIZE
extern	u16 SppDataCCC;
extern	u8 SppDataBuffer[SPP_SERVICE_BUF_LEN];
#endif

#if (ADV_SERVICE_ENABLE)
#define SERVICE_UUID_ADV 		0x1f10
#define CHARACTERISTIC_UUID_ADV (SERVICE_UUID_ADV+1)
extern u16 advNotifyCCC;
extern u32 advDataValue;
#endif

typedef struct
{
  /** Minimum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  unsigned short intervalMin;
  /** Maximum value for the connection event (interval. 0x0006 - 0x0C80 * 1.25 ms) */
  unsigned short intervalMax;
  /** Number of LL latency connection events (0x0000 - 0x03e8) */
  unsigned short latency;
  /** Connection Timeout (0x000A - 0x0C80 * 10 ms) */
  unsigned short timeout;
} gap_periConnectParams_t;

///////////////////////////////////// ATT  HANDLER define ///////////////////////////////////////
typedef enum
{
	ATT_H_START = 0,

	//// Gap ////
	/**********************************************************************************************/
	GenericAccess_PS_H, 					//UUID: 2800, 	VALUE: uuid 1800
	GenericAccess_DeviceName_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	GenericAccess_DeviceName_DP_H,			//UUID: 2A00,   VALUE: device name
	GenericAccess_Appearance_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	GenericAccess_Appearance_DP_H,			//UUID: 2A01,	VALUE: appearance
	CONN_PARAM_CD_H,						//UUID: 2803, 	VALUE:  			Prop: Read
	CONN_PARAM_DP_H,						//UUID: 2A04,   VALUE: connParameter

	//// gatt ////
	/**********************************************************************************************/
	GenericAttribute_PS_H,					//UUID: 2800, 	VALUE: uuid 1801
	GenericAttribute_ServiceChanged_CD_H,	//UUID: 2803, 	VALUE:  			Prop: Indicate
	GenericAttribute_ServiceChanged_DP_H,   //UUID:	2A05,	VALUE: service change
	GenericAttribute_ServiceChanged_CCB_H,	//UUID: 2902,	VALUE: serviceChangeCCC

	//// device information ////
	/**********************************************************************************************/
	DeviceInformation_PS_H,					//UUID: 2800, 	VALUE: uuid 180A
	DeviceInformation_pnpID_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read
	DeviceInformation_pnpID_DP_H,			//UUID: 2A50,	VALUE: PnPtrs
#if	(BATT_SERVICE_ENABLE)
	//// battery service ////
	/**********************************************************************************************/
	BATT_PS_H, 								//UUID: 2800, 	VALUE: uuid 180f
	BATT_LEVEL_INPUT_CD_H,					//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	BATT_LEVEL_INPUT_DP_H,					//UUID: 2A19 	VALUE: batVal
	BATT_LEVEL_INPUT_CCB_H,					//UUID: 2902, 	VALUE: batValCCC
#endif
#if (ADV_SERVICE_ENABLE)
	//// ADV ////
	/**********************************************************************************************/
	ADV_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	ADV_CD_H,								//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	ADV_DP_H,								//UUID: adv uuid,  VALUE: Data
	ADV_CCB_H,								//UUID: 2902, 	VALUE: advValCCC
#endif
#if (SPP_SERVICE_ENABLE)
	//// TELIK_SPP ////
	/**********************************************************************************************/
	SPP_PS_H, 								//UUID: 2800, 	VALUE: telink
	//Server2Client
	SPP_Server2Client_INPUT_CD_H,			//UUID: 2803, 	VALUE:  			Prop: Read | Notify
	SPP_Server2Client_INPUT_DP_H,			//UUID: SPP_Server2Client uuid,  VALUE: SppDataServer2ClientData
	SPP_Server2Client_INPUT_CCB_H,			//UUID: 2902 	VALUE: SppDataServer2ClientDataCCC
	SPP_Server2Client_INPUT_DESC_H,			//UUID: 2901, 	VALUE: SPPS2CDescriptor
#endif
#if (OTA_SERVICE_ENABLE)
	//// Ota ////
	/**********************************************************************************************/
	OTA_PS_H, 								//UUID: 2800, 	VALUE: telink ota service uuid
	OTA_CMD_OUT_CD_H,						//UUID: 2803, 	VALUE:  			Prop: read | write_without_rsp
	OTA_CMD_OUT_DP_H,						//UUID: telink ota uuid,  VALUE: otaData
	OTA_CMD_OUT_DESC_H,						//UUID: 2901, 	VALUE: otaName
#endif

	ATT_END_H,

}ATT_HANDLE;

static inline u32 clock_tik_exceed(u32 ref, u32 span_us){
	return ((u32)(clock_time() - ref) > span_us);
}
extern u8 tbl_mac[6];
extern const attribute_t my_Attributes[];
void ble_init(void);

#endif /* BLE_H_ */
