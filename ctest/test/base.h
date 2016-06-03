/**************************************************************
 * Copyright (C) 2016-2016 All rights reserved.
 * @Version: 1.0
 * @Created: 2016-04-25 21:03
 * @Author: menqi - 1083734876@qq.com
 * @Description: 
 *
 * @History: 
 **************************************************************/

#ifndef _BASE_H_
#define _BASE_H_

#include <stdio.h>
#include <stdlib.h>
#include "basetypes.h"

#define AP_CONF_PATH  "/tmp/config/ap.conf"
#define WPA_SUPPLICANT_CONF_PATH "/tmp/config/wpa_supplicant.conf"
#define KEEPALVIE_CONF_PATH  "/tmp/config/keepalive.conf"
#define PARAMS_IN_AMBOOT_CONF_PATH  "/tmp/params_in_amboot.conf"
#define PARMAS_IN_LINUX_CONF_PATH "/tmp/config/params_in_linux.conf"
#define SMARTAVC_CONF_PATH "/tmp/config/smartavc.conf"
#define COOEE_CONF_PATH  "/tmp/config/cooee.conf"

#define UART_BAUD_RATE  9600

#define CPLUSPLUS_HEAD \
#if defined(__cplusplus) \
extern "C" {     \
#endif

#define CPLUSPLUS_END \
#if defined(__cplusplus) \
} \
#endif

#define LOG_ERROR(format, args...)  do { \
	fprintf(stdout, "ERROR %s, %d, "format, __FILE__, __LINE__, ##args);   \
	fflush(stdout);   \
} while (0)

#define LOG_PRINT(format, args...)  do { \
	fprintf(stdout, "Info: "format, ##args);   \
	fflush(stdout);   \
} while (0)

//todo
#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
#define LOG_DEBUG(format, args...)  do { \
	fprintf(stdout, "Debug: "format, ##args);   \
	fflush(stdout);   \
} while (0)
#else
#define LOG_DEBUG(format, args...)
#endif

typedef enum error_code {
	ECode_Success = 0,
	ECode_Error,
	ECode_WlanUp_Fail,
	ECode_WlanConfig_Fail,
	ECode_RecordingStart_Fail,
} ERROR_CODE;

typedef enum module_state {
	STATE_INIT = 0,
	STATE_IDLE = 1,
	STATE_ERROR = 99,
} MODULE_STATE;

typedef enum fastboot_mode {
	FASTBOOT_MODE_WLAN_CONFIG = 0x00,//000
	FASTBOOT_MODE_WLAN_RECONNECT = 0x01,//001
	FASTBOOT_MODE_NOTIFY = 0x02, //010
	FASTBOOT_MODE_RECORDING = 0x04,//100
	FASTBOOT_MODE_STREAMING = 0x05,// 101
} FASTBOOT_MODE;

typedef enum {
	//from mcu
	msg_PirOn = 0x11,
	msg_PirOff,
	msg_LowBattery,
	msg_NormalBattery,
	msg_Discharging,
	msg_Charging,
	msg_ChrgFull,
	msg_NeedChrg,
	msg_DcPlugin,
	msg_DcUnplugged,

	//to/from app
	msg_BootUp,
	msg_ShutDown,

	//to mcu
	msg_WifiConnected = 0xEE,
	msg_WifiDisconnected = 0xFF,
} MSG_TYPE;

typedef enum {
	EChrg_ChrgFull = 0,
	EChrg_Charging,
	EChrg_Discharging,
} EChrg;

typedef enum {
	EDC_Plugin = 0,
	EDC_Unplugged,
} EDC;

typedef enum {
	EPIR_ON = 0,
	EPIR_OFF,
} EPIR;

typedef enum {
	EBattery_Low = 0,
	EBattery_NeedChrg,
	EBattery_Normal,
} EBattery;

typedef enum {
	EWlan_Connected = 0x11,
	EWlan_Disconnected = 0x22,
} EWlan;

typedef enum {
	ECloud_Connected = 0,
	ECloud_Disconnected,
} ECloud;

typedef struct {
	u8 type;
	u8 length;
} __attribute__((packed))STLVMsgHeader;

typedef struct {
	u8 chrg;
	u8 dc;
	u8 pir;
	u8 battery;
	u8 quantity;//(0-100)%
	u8 wlan;
	u8 reserved0;
	u8 reserved1;
	void *p_agent;
} SDeviceStatus;
#endif

