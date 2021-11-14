/*
 * ble.c
 *
 *  Created on: 22.02.2020
 *      Author: pvvx
 */
#include "ble.h"

#if !BLE_MASTER

//////////////////////////////////////////////////////////////////////////////
//	Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8 tbl_advData[] = { // https://www.bluetooth.com/specifications/assigned-numbers/generic-access-profile/
	 // 0x01: Основные характеристики Bluetooth
	 0x02, GAP_ADTYPE_FLAGS,
	 0x06 							// BLE General Discoverable Mode and BR/EDR not supported
#if 0
	 // 0x19: Базовая спецификация Bluetooth
	 , 0x03, GAP_ADTYPE_APPEARANCE, 0x80, 0x01 				// 384, Generic Remote Control, Generic category
	 // 0x02: incomplete list of service class UUID (0xffe0)
#if SPP_SERVICE_ENABLE || BATT_SERVICE_ENABLE
	 , 0x01 + ((SPP_SERVICE_ENABLE)? 2 : 0) + ((SERVICE_UUID_BATTERY)? 2 : 0) , GAP_ADTYPE_16BIT_INCOMPLETE
#if SPP_SERVICE_ENABLE
	 , SERVICE_UUID_SPP & 0xff, (SERVICE_UUID_SPP>>8)& 0xff
#endif
#if BATT_SERVICE_ENABLE
	 , SERVICE_UUID_BATTERY & 0xff, (SERVICE_UUID_BATTERY>>8)& 0xff
#endif
#endif
#endif
};

const struct {
	u8 size;
	u8 id;
	u8 name[DEV_NAME_SIZE];
} tbl_scanRsp = {
	.size = DEV_NAME_SIZE + 1,
	.id = GAP_ADTYPE_LOCAL_NAME_COMPLETE,
	.name = {DEV_NAME}
};

gap_periConnectParams_t my_periConnParameters = {DEF_CONN_PARMS};
const u16 userdesc_UUID		= GATT_UUID_CHAR_USER_DESC;

/////////////////////////////////////////ADV/////////////////////////////////////
#if (ADV_SERVICE_ENABLE)
const u8 advDescriptor[] = "ADV";
//// ADV attribute values
static const u8 my_advCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(ADV_DP_H), U16_HI(ADV_DP_H),
	U16_LO(CHARACTERISTIC_UUID_ADV), U16_HI(CHARACTERISTIC_UUID_ADV)
};
const u16 my_advServiceUUID		= SERVICE_UUID_ADV;
const u16 my_advCharUUID       	= CHARACTERISTIC_UUID_ADV;

u16 advNotifyCCC;
u32 advDataValue;

#endif
/////////////////////////////////////////SPP/////////////////////////////////////
#if (SPP_SERVICE_ENABLE)
//SPP data descriptor
#define SPP_UUID_LEN	2 // 2 or 16
const u8 SppS2cDescriptor[] = "SPP";
#if SPP_UUID_LEN == 16
#define SPP_UUID_SERVICE			0xfb,0x34,0x9b,0x5f,0x80,0x00, 0x00,0x80, 0x00,0x10, 0x00,0x00, SERVICE_UUID_SPP&0xFF,SERVICE_UUID_SPP>>8,0x00,0x00
#define SPP_DATA_SERVER2CLIENT		0xfb,0x34,0x9b,0x5f,0x80,0x00, 0x00,0x80, 0x00,0x10, 0x00,0x00, CHARACTERISTIC_UUID_SPP&0xFF,CHARACTERISTIC_UUID_SPP>>8,0x00,0x00

const u8 SppServiceUUID[16]			= {SPP_UUID_SERVICE};
const u8 SppDataUUID[16] 			= {SPP_DATA_SERVER2CLIENT}; // "SPP: Module<->Phone"
// SPP attribute values
static const u8 SppS2cCharVal[19] = {
//	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
	U16_LO(SPP_Server2Client_INPUT_DP_H), U16_HI(SPP_Server2Client_INPUT_DP_H),
	SPP_DATA_SERVER2CLIENT
};
#else
const u16 SppServiceUUID		= SERVICE_UUID_SPP;
const u16 SppDataUUID 			= SERVICE_UUID_SPP+1; // "SPP: Module<->Phone"
// SPP attribute values
static const u8 SppS2cCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP | CHAR_PROP_NOTIFY,
	U16_LO(SPP_Server2Client_INPUT_DP_H), U16_HI(SPP_Server2Client_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_SPP), U16_HI(CHARACTERISTIC_UUID_SPP)
};
#endif

u16 SppDataCCC;
u8 SppDataBuffer[SPP_SERVICE_BUF_LEN];

int __attribute__((weak)) onSppReceiveData(void *par) {
	(void)par; // rf_packet_att_write_t * p
	//rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;
	//int len = pw->l2cap - 3;
	//add your code
	//valid data is pw->dat[0] ~ pw->dat[len-1]
	return 0;
}

int __attribute__((weak)) onSppSendData(void *par) {
	(void)par; // rf_packet_att_write_t * p
	return 0;
}

#endif // #if (SPP_SERVICE_ENABLE)

#if (OTA_SERVICE_ENABLE)
#undef TELINK_OTA_UUID_SERVICE
#define TELINK_OTA_UUID_SERVICE   			0x12,0x19,0x0d,0x0c,0x0b,0x0a,0x09,0x08,0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00
// OTA attribute values
static const u8 my_OtaCharVal[19] = {
	CHAR_PROP_READ | CHAR_PROP_WRITE_WITHOUT_RSP,
	U16_LO(OTA_CMD_OUT_DP_H), U16_HI(OTA_CMD_OUT_DP_H),
	TELINK_SPP_DATA_OTA
};
const u8 my_OtaServiceUUID[16]	= {TELINK_OTA_UUID_SERVICE};
const u8 my_OtaUUID[16]			= {TELINK_SPP_DATA_OTA};
const u8 my_OtaName[] = {'O', 'T', 'A'};
u8	my_OtaData;

extern u32 blt_ota_start_tick;
int pre_otaWrite(void * p) {
	blt_ota_start_tick = clock_time() | 1;
	return otaWrite(p);
}
void  __attribute__((weak)) show_ota_result(int result) {
}
void  __attribute__((weak)) entry_ota_mode(void) {
	bls_ota_setTimeout(100 * 1000000); // set OTA timeout  100 S
}
#endif // #if (OTA_SERVICE_ENABLE)

#if	(BATT_SERVICE_ENABLE)
//////////////////////// Battery /////////////////////////////////////////////////
//// Battery attribute values
static const u8 my_batCharVal[5] = {
	CHAR_PROP_READ | CHAR_PROP_NOTIFY,
	U16_LO(BATT_LEVEL_INPUT_DP_H), U16_HI(BATT_LEVEL_INPUT_DP_H),
	U16_LO(CHARACTERISTIC_UUID_BATTERY_LEVEL), U16_HI(CHARACTERISTIC_UUID_BATTERY_LEVEL)
};
const u16 my_batServiceUUID		= SERVICE_UUID_BATTERY;
const u16 my_batCharUUID       	= CHARACTERISTIC_UUID_BATTERY_LEVEL;
//static
u16 batteryValueInCCC;
u8 my_batVal	= 99;

#endif // #if	(BATT_SERVICE_ENABLE)

static const u16 clientCharacterCfgUUID = GATT_UUID_CLIENT_CHAR_CFG;

static const u16 extReportRefUUID = GATT_UUID_EXT_REPORT_REF;

static const u16 reportRefUUID = GATT_UUID_REPORT_REF;

static const u16 characterPresentFormatUUID = GATT_UUID_CHAR_PRESENT_FORMAT;

static const u16 my_primaryServiceUUID = GATT_UUID_PRIMARY_SERVICE;

static const u16 my_characterUUID = GATT_UUID_CHARACTER;

static const u16 my_devServiceUUID = SERVICE_UUID_DEVICE_INFORMATION;

static const u16 my_PnPUUID = CHARACTERISTIC_UUID_PNP_ID;

static const u16 my_devNameUUID = GATT_UUID_DEVICE_NAME;

//device information
static const u16 my_gapServiceUUID = SERVICE_UUID_GENERIC_ACCESS;
// Device Name Characteristic Properties
static u8 my_devNameCharacter = CHAR_PROP_READ | CHAR_PROP_NOTIFY;
// Appearance Characteristic Properties
static const u16 my_appearanceUIID = 0x2a01;
static const u16 my_periConnParamUUID = 0x2a04;
static u8 my_appearanceCharacter = CHAR_PROP_READ;
// Peripheral Preferred Connection Parameters Characteristic Properties
static u8 my_periConnParamChar = CHAR_PROP_READ;
u16 my_appearance = GAP_APPEARE_UNKNOWN;


static const u16 my_gattServiceUUID = SERVICE_UUID_GENERIC_ATTRIBUTE;
static const u8  serviceChangedProp = CHAR_PROP_INDICATE;
static const u16 serviceChangeUIID = GATT_UUID_SERVICE_CHANGE;
u16 serviceChangeVal[2] = {0};
static u8 serviceChangeCCC[2]={0,0};
static u8 my_PnPCharacter = CHAR_PROP_READ;
const u8 my_PnPtrs[] = {0x02, 0x8a, 0x24, 0x66, 0x82, 0x01, 0x00};

/////////////////////////////////////////////////////////

// TM : to modify
const attribute_t my_Attributes[] = {
	{ATT_END_H - 1, 0,0,0,0,0},	// total num of attribute
	// 0001 - 0007  gap
	{7,ATT_PERMISSIONS_READ,2,sizeof(my_gapServiceUUID),(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gapServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_devNameCharacter),(u8*)(&my_characterUUID), 		(u8*)(&my_devNameCharacter), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(tbl_scanRsp.name), (u8*)(&my_devNameUUID), (u8*)(tbl_scanRsp.name), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_appearanceCharacter),(u8*)(&my_characterUUID), 		(u8*)(&my_appearanceCharacter), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_appearance), (u8*)(&my_appearanceUIID), 	(u8*)(&my_appearance), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParamChar),(u8*)(&my_characterUUID), 		(u8*)(&my_periConnParamChar), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_periConnParameters),(u8*)(&my_periConnParamUUID), 	(u8*)(&my_periConnParameters), 0},

	// 0008 - 000b gatt
	{4,ATT_PERMISSIONS_READ,2,sizeof(my_gattServiceUUID),(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_gattServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(serviceChangedProp),(u8*)(&my_characterUUID), 		(u8*)(&serviceChangedProp), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(serviceChangeVal), (u8*)(&serviceChangeUIID), 	(u8*)(&serviceChangeVal), 0},
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(serviceChangeCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(serviceChangeCCC), 0},

	// 000c - 000e  device Information Service
	{3,ATT_PERMISSIONS_READ,2,sizeof(my_devServiceUUID),(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_devServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PnPCharacter),(u8*)(&my_characterUUID), 		(u8*)(&my_PnPCharacter), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_PnPtrs),(u8*)(&my_PnPUUID), (u8*)(my_PnPtrs), 0},

#if	(BATT_SERVICE_ENABLE)
	////////////////////////////////////// Battery Service /////////////////////////////////////////////////////
	{4,ATT_PERMISSIONS_READ,2,sizeof(my_batServiceUUID),(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_batServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_batCharVal),(u8*)(&my_characterUUID), (u8*)(my_batCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_batVal),(u8*)(&my_batCharUUID), (u8*)(&my_batVal), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(batteryValueInCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&batteryValueInCCC), 0},	//value
#endif
#if (ADV_SERVICE_ENABLE)
	////////////////////////////////////// Adv Service /////////////////////////////////////////////////////
	{4,ATT_PERMISSIONS_READ,2,sizeof(my_advServiceUUID),(u8*)(&my_primaryServiceUUID), 	(u8*)(&my_advServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ,2,sizeof(my_advCharVal),(u8*)(&my_characterUUID), (u8*)(my_advCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_READ,2,sizeof(advDataValue),(u8*)(&my_advCharUUID), (u8*)(&advDataValue), 0},	//value
	{0,ATT_PERMISSIONS_RDWR,2,sizeof(advNotifyCCC),(u8*)(&clientCharacterCfgUUID), (u8*)(&advNotifyCCC), 0},	//value
#endif
#if (SPP_SERVICE_ENABLE)
	////////////////////////////////////// SPP Service /////////////////////////////////////////////////////
	{5,ATT_PERMISSIONS_READ,2,sizeof(SppServiceUUID),(u8*)(&my_primaryServiceUUID), 	(u8*)(&SppServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ, 2,sizeof(SppS2cCharVal),(u8*)(&my_characterUUID),	(u8*)(SppS2cCharVal), 0},
	{0,ATT_PERMISSIONS_RDWR,SPP_UUID_LEN,sizeof(SppDataBuffer),(u8*)(&SppDataUUID), (u8*)&SppDataBuffer, (att_readwrite_callback_t)&onSppReceiveData, (att_readwrite_callback_t)&onSppSendData},
	{0,ATT_PERMISSIONS_RDWR, 2,sizeof(SppDataCCC),(u8*)(&clientCharacterCfgUUID), 	(u8*)&SppDataCCC, 0},
	{0,ATT_PERMISSIONS_READ, 2,sizeof(SppS2cDescriptor),(u8*)&userdesc_UUID,(u8*)(&SppS2cDescriptor)},
#endif
#if (OTA_SERVICE_ENABLE)
	////////////////////////////////////// OTA Service /////////////////////////////////////////////////////
	{4,ATT_PERMISSIONS_READ, 2,16,(u8*)(&my_primaryServiceUUID), (u8*)(&my_OtaServiceUUID), 0},
	{0,ATT_PERMISSIONS_READ, 2,sizeof(my_OtaCharVal),(u8*)(&my_characterUUID), (u8*)(my_OtaCharVal), 0},				//prop
	{0,ATT_PERMISSIONS_RDWR,16,sizeof(my_OtaData),(u8*)(&my_OtaUUID),	(&my_OtaData), &pre_otaWrite, &otaRead},			//value
	{0,ATT_PERMISSIONS_READ, 2,sizeof(my_OtaName),(u8*)(&userdesc_UUID), (u8*)(my_OtaName), 0},
#endif

};

#endif // #if !BLE_MASTER

//----------------------------- BLE stack
my_fifo_t			blt_rxfifo;
u8					blt_rxfifo_b[];

my_fifo_t			blt_txfifo;
u8					blt_txfifo_b[];

MYFIFO_INIT(blt_rxfifo, 64, 8); 	// 64*8 512 bytes + headers
MYFIFO_INIT(blt_txfifo, 40, 16);	// 40*16 640 bytes + headers

u8  tbl_mac[6] = {0xe1, 0xe1, 0x0b, 0x38, 0xc1, 0xa4};

/////////////////////////////////////////////////////////////////////
void ble_init(void) {
	rf_drv_init(CRYSTAL_TYPE);
	///////////// BLE stack Initialization ////////////////
	u32 *pmac = (u32 *) CFG_ADR_MAC;
	if (*pmac != 0xffffffff)
		memcpy (tbl_mac, pmac, 6);
	else {
		//TODO : should write mac to flash after pair OK
		tbl_mac[0] = (u8)rand();
		tbl_mac[1] = (u8)rand();
		flash_write_page(CFG_ADR_MAC, 6, tbl_mac);
	}

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU(tbl_mac);   // mandatory

#if BLE_MASTER
	blc_ll_initScanning_module(tbl_mac); 	//scan module: 		 mandatory for BLE master,
	////// Controller Initialization  //////////
	blc_ll_initBasicMCU(tbl_mac);   //mandatory

	blc_ll_initScanning_module(tbl_mac); 	//scan module: 		 mandatory for BLE master,
	blc_ll_initInitiating_module();			//initiate module: 	 mandatory for BLE master,
	//blc_ll_initMasterRoleSingleConn_module();			//master module: 	 mandatory for BLE master,

	//ll_whiteList_reset();  //clear whitelist
#else
	blc_ll_initAdvertising_module(tbl_mac); 	// adv module: 		 mandatory for BLE slave,
	blc_ll_initSlaveRole_module();				// slave module: 	 mandatory for BLE slave,

	////// Host Initialization  //////////

	bls_att_setAttributeTable ((u8 *)my_Attributes);

	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)&tbl_scanRsp, sizeof(tbl_scanRsp));

	blc_l2cap_register_handler(blc_l2cap_packet_receive);  	// l2cap initialization
	bls_smp_enableParing(SMP_PARING_DISABLE_TRRIGER);
#if (OTA_SERVICE_ENABLE)
	// OTA init
	bls_ota_clearNewFwDataArea(); //must
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(show_ota_result);
#endif
	///////////////////// USER application initialization ///////////////////
	if(bls_ll_setAdvParam(
			DEF_ADV_INTERVAL,
			ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
			0,  NULL,
			BLT_ENABLE_ADV_ALL,
			ADV_FP_NONE) != BLE_SUCCESS) {
			cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, clock_time() + 5000 * CLOCK_SYS_CLOCK_1MS); // <3.5 uA
			while(1);
	}
	bls_ll_setAdvEnable(1);  //adv enable
#endif
#if 0
	blc_hci_registerControllerEventHandler(event_handler);		//register event callback
	bls_hci_mod_setEventMask_cmd(0xfffff);			//enable all 18 events,event list see ble_ll.h
#endif
#if 0
	blc_att_registerMtuSizeExchangeCb(&MtuSizeExchanged_callback);
#endif

#if MTU_DATA_SIZE > 23
	blc_att_setRxMtuSize(MTU_DATA_SIZE); 	// If not set RX MTU size, default is: 23 bytes, max 241
#endif

	rf_set_power_level_index(RF_POWER_8dBm); // = RF_POWER_8dBm
	blc_ll_initPowerManagement_module();        // pm module:      	 optional
#if(__TL_LIB_8266__ || MCU_CORE_TYPE == MCU_CORE_8266)
	blc_pm_disableFlashShutdown_when_suspend();
#endif
	bls_pm_setSuspendMask(SUSPEND_DISABLE);	//(SUSPEND_ADV | SUSPEND_CONN)
}
