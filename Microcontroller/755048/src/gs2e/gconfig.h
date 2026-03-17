//========================================================================================
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
// 
//     This is unpublished proprietary source code of ASIX Electronic Corporation
// 
//     The copyright notice above does not evidence any actual or intended 
//     publication of such source code.
//========================================================================================
// 
//========================================================================================
// Module Name    : gconfig.h
// Purpose        :
// Author         :
// Date           :
// Notes          :
// $Log           : gconfig.h,v $
//========================================================================================

#ifndef __GCONFIG_H__
#define __GCONFIG_H__

// INCLUDE FILE DECLARATIONS //
#include "types.h"

// NAMING CONSTANT DECLARATIONS //
#define GCONFIG_DEFAULT_DATA_PORT		      51020                                     // Default data port for both server and client modes
#define GCONFIG_DEFAULT_Host_PORT		      51021                                     // Default data port for both server and client modes

// Following UDP broadcast, multicast and unicast listening ports are used for communication
// with Device Server Manager program and are not configurable
#define GCONFIG_UDP_BCAST_SERVER_PORT	25122                                           // UDP broadcast listening ports
#define GCONFIG_UDP_UCAST_SERVER_PORT	25124                                           // UDP unicast listening ports

#define GCONFIG_CFG_DATA_FLASH_ADDR		0x4000                                          //
#define GCONFIG_CFG_PKT_LEN			72                                              //

#define GCONFIG_OPCODE_OFFSET			8                                               //
#define GCONFIG_OPCODE_SEARCH_REQ		0x00                                            //
#define GCONFIG_OPCODE_SEARCH_ACK		0x01                                            //

#define GCONFIG_OPCODE_SET_REQ		0x02                                            //
#define GCONFIG_OPCODE_SET_ACK		0x03                                            //
#define GCONFIG_OPCODE_SET_DENY		0x04                                            //

#define GCONFIG_OPCODE_UPGRADE_REQ		0x05                                            //
#define GCONFIG_OPCODE_UPGRADE_ACK		0x06                                            //
#define GCONFIG_OPCODE_UPGRADE_DENY		0x07                                            //

#define GCONFIG_OPCODE_RESET_REQ		0x08                                            //
#define GCONFIG_OPCODE_RESET_ACK		0x09                                            //
#define GCONFIG_OPCODE_RESET_DENY		0x0A                                            //

#define GCONFIG_OPCODE_REBOOT_REQ		0x0B                                            //
#define GCONFIG_OPCODE_REBOOT_ACK		0x0C                                            //
#define GCONFIG_OPCODE_REBOOT_DENY		0x0D                                            //

#define GCONFIG_OPCODE_MONITOR_REQ		0x10                                            //
#define GCONFIG_OPCODE_MONITOR_ACK		0x11                                            //

#define GCONFIG_OPCODE_CONN_CLOSE		0x12                                            //
#define GCONFIG_OPCODE_NONE			0xFF                                            //

#define GCONFIG_OPTION_ENABLE_REBOOT		0x01                                      //
#define GCONFIG_OPTION_ENABLE_UDP_AUTO_CONNECT	0x02                                      //
#define GCONFIG_OPTION_ENABLE_SIMTEK_DEBUG    	0x04                                      //

#define GCONFIG_NETWORK_SERVER		0x0000                                          //
#define GCONFIG_NETWORK_CLIENT		0x8000                                          //
#define GCONFIG_NETWORK_DHCP_ENABLE		0x4000                                          //
#define GCONFIG_NETWORK_DHCP_DISABLE	0x0000                                          //
#define GCONFIG_NETWORK_PROTO_UDP		0x0800                                          //
#define GCONFIG_NETWORK_PROTO_TCP		0x0400                                          //
#define GCONFIG_NETWORK_PROTO_UDP_BCAST	0x0100                                          //
#define GCONFIG_NETWORK_TIMEOUT_0_MIN	0x0000                                          //
#define GCONFIG_NETWORK_TIMEOUT_10_MIN	0x000A                                          //
#define GCONFIG_NETWORK_ACC_IP_ENABLE	0x1000                                          //
#define GCONFIG_NETWORK_ACC_IP_DISABLE	0x0000                                          //

#define GCONFIG_SPORT_BRATE_921600		0x0000                                          //
#define GCONFIG_SPORT_BRATE_115200		0x0800                                          //
#define GCONFIG_SPORT_BRATE_38400		0x1000                                          //
#define GCONFIG_SPORT_BRATE_19200		0x1800                                          //
#define GCONFIG_SPORT_BRATE_9600		0x2000                                          //
#define GCONFIG_SPORT_BRATE_4800		0x2800                                          //
#define GCONFIG_SPORT_BRATE_2400		0x3000                                          //
#define GCONFIG_SPORT_BRATE_1200		0x3800                                          //
#define GCONFIG_SPORT_DATABIT_5		0x0000                                          //
#define GCONFIG_SPORT_DATABIT_6		0x0100                                          //
#define GCONFIG_SPORT_DATABIT_7		0x0200                                          //
#define GCONFIG_SPORT_DATABIT_8		0x0300                                          //
#define GCONFIG_SPORT_PARITY_ODD		0x0000                                          //
#define GCONFIG_SPORT_PARITY_EVEN		0x0020                                          //
#define GCONFIG_SPORT_PARITY_NONE		0x0040                                          //
#define GCONFIG_SPORT_FLOWC_XON_XOFF	0x0000                                          //
#define GCONFIG_SPORT_FLOWC_HARDWARE	0x0004                                          //
#define GCONFIG_SPORT_FLOWC_NONE		0x0008                                          //
#define GCONFIG_SPORT_STOPBIT_1		0x0000                                          //
#define GCONFIG_SPORT_STOPBIT_1P5		0x0001                                          //
#define GCONFIG_SPORT_STOPBIT_2		0x0002                                          //

//#define GCONFIG_SMTP_EVENT_COLDSTART	0x0001                                          //
//#define GCONFIG_SMTP_EVENT_WARMSTART	0x0002                                          //
//#define GCONFIG_SMTP_EVENT_AUTH_FAIL	0x0004                                          //
//#define GCONFIG_SMTP_EVENT_IP_CHANGED	0x0008                                          //
//#define GCONFIG_SMTP_EVENT_PSW_CHANGED	0x0010                                          //

#define GCONFIG_ACCESSIBLE_IP_NUM		4                                               //
#define GCONFIG_VERSION_STR_LEN		12                                              //
#define GCONFIG_VERSION_STRING		"3.1.200"                                       //

// TYPE DECLARATIONS //
typedef struct _GCONFIG_GID                                                               //
{
      byte                    id[8];                                                      //
} GCONFIG_GID;                                                                            //

typedef struct _GCONFIG_DEV_NAME                                                          //
{
      byte				DevName[16];                                                //
} GCONFIG_DEV_NAME;                                                                       //

typedef struct _GCONFIG_MAC_ADDR                                                          //
{
      byte				MacAddr[6];                                                 //
} GCONFIG_MAC_ADDR;                                                                       //

typedef struct _GCONFIG_SMTP_CONFIG                                                       //
{
      byte                    DomainName[36];                                             //
      byte                    FromAddr[36];                                               //
      byte                    ToAddr1[36];                                                //
      byte                    ToAddr2[36];                                                //
      byte                    ToAddr3[36];                                                //
      word                    EventEnableBits;                                            //
} GCONFIG_SMTP_CONFIG;                                                                    //

typedef struct                                                                            //
{
      byte                    Username[16];                                               //
      byte                    Passwd[16];                                                 //
      byte                    Level;                                                      //
      byte                    Rsvd;                                                       //
} GCONFIG_ADMIN;                                                                          //

typedef struct _GCONFIG_CFG_PKT                                                           //
{
      GCONFIG_GID             Gid;                                                        // length 8 bytes

      byte                    Opcode;                                                     // length 1 bytes = 9
      byte                    Option;                                                     // length 1 bytes = 10
      GCONFIG_DEV_NAME        DevName;                                                    // length 16 bytes = 26
      GCONFIG_MAC_ADDR        MacAddr;                                                    // length 6 bytes = 32
      word                    Network;                                                    // length 2 bytes = 34
      ulong                   ServerDynamicIP;                                            // length 4 bytes = 38
      ulong                   ServerStaticIP;                                             // length 4 bytes = 42
      word                    ServerDataPktListenPort;                                    // length 2 bytes = 44
      byte                    MasterAddress;                                              // length 1 bytes = 46
      byte                    t5;                                                         // length 1 bytes = 47
      word                    ServerBroadcastListenPort;                                  // length 2 bytes = 48
      ulong                   ClientDestIP;                                               // length 4 bytes = 52
      word                    ClientDestPort;                                             // length 2 bytes = 54
      ulong                   Netmask;                                                    // length 4 bytes = 58
      ulong                   t1;
      ulong                   t2;
      word                    t3;
      byte                    t4;
      byte                    DeviceStatus;                                               // length 1 bytes = 70
      word                    EthernetTxTimer;                                            // length 2 bytes = 72
} GCONFIG_CFG_PKT;

typedef struct _GCONFIG_CFG_DATA
{
      word                    CheckSum;
      word                    CfgLength;
      /* the following 5 items will be used for boot loader */
      ulong                   DeviceIp;
      ulong                   DeviceNetmask;
      ulong                   t6;                                                         // length 1 bytes = 47
      ulong                   UdpAutoConnectClientIp;
      word                    UdpAutoConnectClientPort;
      GCONFIG_DEV_NAME        DevName;
      word                    Network;
      ulong                   ServerStaticIP;
      word                    ServerDataPktListenPort;
      byte                    MasterAddress;                                              // length 1 bytes = 46
      byte                    t5;                                                         // length 1 bytes = 47
      word                    ServerBroadcastListenPort;
      ulong                   ClientDestIP;
      word                    ClientDestPort;
      ulong                   Netmask;
      ulong                   t1;
      ulong                   t2;
      word                    t3;
      byte                    t4;
      byte                    Option;
      word                    EthernetTxTimer;
} GCONFIG_CFG_DATA;

typedef struct _GCONFIG_MONITOR_PKT
{
	GCONFIG_GID 		Gid;
	byte				Opcode;
	byte				ModemStatus;
	byte				VerStr[GCONFIG_VERSION_STR_LEN];
	ulong				TxBytes;
	ulong				RxBytes;
} GCONFIG_MONITOR_PKT;

typedef struct _GCONFIG_CONN_CLOSE_PKT
{
	GCONFIG_GID 		Gid;
	byte				Opcode;
	byte				Option;
	ulong				RemoteIp;
	word 				Port;
} GCONFIG_CONN_CLOSE_PKT;

// GLOBAL VARIABLES //
extern GCONFIG_GID XDATA GCONFIG_Gid;

// EXPORTED SUBPROGRAM SPECIFICATIONS //
void GCONFIG_Task(void);
void GCONFIG_Init(void);
void GCONFIG_SetConfigPacket(GCONFIG_CFG_PKT*);
void GCONFIG_GetConfigPacket(GCONFIG_CFG_PKT*);
void GCONFIG_ReadConfigData(void);
void GCONFIG_WriteConfigData(void);
void GCONFIG_ReadDefaultConfigData(void);
//void gconfig_DisplayConfigData(GCONFIG_CFG_DATA *pConfig);
void gconfig_ExDisplayConfigData(void);

ulong GCONFIG_IpAddr2Ulong(byte*, byte);

byte GCONFIG_EnableUdpAutoConnectAfterReboot(void);
void GCONFIG_EnableFirmwareUpgrade(void);
byte Simtek_Modes_Enable(byte);

void GCONFIG_SetDeviceName(GCONFIG_DEV_NAME*);
void GCONFIG_SetNetwork(word );
void GCONFIG_SetServerStaticIP(ulong);
void GCONFIG_SetHostMasterAddress(byte adrs);
void GCONFIG_SetServerDataPktListenPort(word );
void GCONFIG_SetServerBroadcastListenPort(word );
void GCONFIG_SetClientDestIP(ulong);
void GCONFIG_SetClientDestPort(word );
void GCONFIG_SetNetmask(ulong);

void GCONFIG_SetEthernetTxTimer(word );
void GCONFIG_SetUdpAutoConnectClientIp(ulong);
void GCONFIG_SetUdpAutoConnectClientPort(word );

GCONFIG_DEV_NAME* GCONFIG_GetDeviceName(void);
GCONFIG_MAC_ADDR GCONFIG_GetMacAddress(void);
word  GCONFIG_GetNetwork(void);
ulong GCONFIG_GetServerStaticIP(void);
byte  GCONFIG_GetHostMasterAddress(void);
word  GCONFIG_GetServerDataPktListenPort(void);
word  GCONFIG_GetServerBroadcastListenPort(void);
ulong GCONFIG_GetClientDestIP(void);
word  GCONFIG_GetClientDestPort(void);
ulong GCONFIG_GetNetmask(void);
word  GCONFIG_GetEthernetTxTimer(void);

#endif // End of __GCONFIG_H__ //

// End of gconfig.h //
