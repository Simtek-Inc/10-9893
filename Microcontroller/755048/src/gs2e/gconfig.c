//----------------------------------------------------------------------------------------
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name    : gconfig.c
// Purpose :
// Author         :
// Date           :
// Notes          :
// $Log           : gconfig.c,v $
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// INCLUDE FILE DECLARATIONS //
//----------------------------------------------------------------------------------------
#include "reg80390.h"
#include "adapter.h"
#include "gconfig.h"

#include "gs2e.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart.h"
#include "printd.h"
#include "ax11000.h"
#include <absacc.h>
#include <string.h>
#include "instrument.h"
#include "hardware.h"
#include "util_menus.h"                                                                   //
//----------------------------------------------------------------------------------------
// NAMING CONSTANT DECLARATIONS //
//----------------------------------------------------------------------------------------
#define DEVICE_STATUS_IDLE                0
#define DEVICE_STATUS_CONNECTED           1
#define ENABLE_FIRMWARE_UPGRADE           0x5A
//----------------------------------------------------------------------------------------
// MACRO DECLARATIONS //
//----------------------------------------------------------------------------------------

//----------------------------------------------------------------------------------------
// TYPE DECLARATIONS //
//----------------------------------------------------------------------------------------
typedef byte (*pFlashParaData)(byte * addr, word  len);

//----------------------------------------------------------------------------------------
// GLOBAL VARIABLES DECLARATIONS //
//----------------------------------------------------------------------------------------
GCONFIG_GID XDATA GCONFIG_Gid = {'A','S','I','X','X','I','S','A'};
//----------------------------------------------------------------------------------------
// LOCAL VARIABLES DECLARATIONS //
//----------------------------------------------------------------------------------------
byte XDATA gconfig_SysClock;
GCONFIG_CFG_DATA XDATA gconfig_ConfigData;
byte IDATA FirmwareUpgradeFlag _at_ 0x31;
static GCONFIG_MAC_ADDR macAddr;
//----------------------------------------------------------------------------------------
// LOCAL SUBPROGRAM DECLARATIONS //
//----------------------------------------------------------------------------------------
static bool gconfig_ReStoreParameter(ulong addr, GCONFIG_CFG_DATA *pConfig, word  len);
static bool gconfig_StoreParameter(GCONFIG_CFG_DATA *pSramBase, word  len);
static word  gconfig_Checksum(word  *pBuf, ulong length);
static void gconfig_InDisplayConfigData(GCONFIG_CFG_DATA *pConfig);
static void	gconfig_DisplayConfigData(GCONFIG_CFG_DATA *pConfig);

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_Task
// Purpose : Main function
// Params  : void
// Returns : void
// Note           :
//----------------------------------------------------------------------------------------
void GCONFIG_Task(void)
{

} // End of GCONFIG_Task() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_Init
// Purpose : Initialization
// Params  : void
// Returns : void
// Note           :
//----------------------------------------------------------------------------------------
void GCONFIG_Init(void)
{
    byte  dataStopParityBits = 0;
	// Restore last-saved configuration if applicable
    GS2E_SetTaskState(GS2E_STATE_IDLE);

    memcpy((char *)&macAddr, PNetStation->CurrStaAddr, MAC_ADDRESS_LEN);

    GCONFIG_ReadConfigData();
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initialize debug mode control bit                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    fDebugMode      = Simtek_Modes_Enable(DebugModeMask);                                 //
    if(fDebugMode  == OFF)                                                                //
      fTransmitMode = Simtek_Modes_Enable(TransmitModeMask);                              //
    else                                                                                  //
      fTransmitMode = Server;                                                             //
// only enable if default configuration is needed vvvvvvvvvvvv
//    GCONFIG_ReadDefaultConfigData();                    // ^
//    GCONFIG_WriteConfigData();                          // ^
//    gconfig_InDisplayConfigData(&gconfig_ConfigData);   // ^
// only enable if default configuration is needed ^^^^^^^^^^^^
    // UART 2 parameters setting
    gconfig_SysClock = AX11000_GetSysClk();
} // End of GCONFIG_Init() //

/////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetConfigPacket
// Purpose : 
// Params  : pointer to configuration packet
// Returns : void
// Note           :
//----------------------------------------------------------------------------------------
void GCONFIG_SetConfigPacket(GCONFIG_CFG_PKT* pCfgPkt)
{//   GCONFIG_CFG_DATA                                  GCONFIG_CFG_PKT
    gconfig_ConfigData.Option                       = pCfgPkt->Option;
    gconfig_ConfigData.DevName                      = pCfgPkt->DevName;
    gconfig_ConfigData.Network                      = pCfgPkt->Network;
    gconfig_ConfigData.ServerStaticIP               = pCfgPkt->ServerStaticIP;
    gconfig_ConfigData.ServerDataPktListenPort      = pCfgPkt->ServerDataPktListenPort;
    gconfig_ConfigData.MasterAddress                = pCfgPkt->MasterAddress;
    gconfig_ConfigData.ServerBroadcastListenPort    = pCfgPkt->ServerBroadcastListenPort;
    gconfig_ConfigData.ClientDestIP                 = pCfgPkt->ClientDestIP;
    gconfig_ConfigData.ClientDestPort               = pCfgPkt->ClientDestPort; 
    gconfig_ConfigData.Netmask                      = pCfgPkt->Netmask;
    gconfig_ConfigData.EthernetTxTimer              = pCfgPkt->EthernetTxTimer;
    GCONFIG_WriteConfigData();
} // End of GCONFIG_SetConfigPacket() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetConfigPacket
// Purpose : 
// Params  : pointer to configuration packet
// Returns : void
// Note           :
//----------------------------------------------------------------------------------------
void GCONFIG_GetConfigPacket(GCONFIG_CFG_PKT* pCfgPkt)
{
    memcpy((char *)&pCfgPkt->Gid, (char *)&GCONFIG_Gid, sizeof(GCONFIG_Gid));
    pCfgPkt->Option                    = gconfig_ConfigData.Option;
    pCfgPkt->DevName                   = gconfig_ConfigData.DevName;
    pCfgPkt->MacAddr                   = macAddr;
    pCfgPkt->Network                   = gconfig_ConfigData.Network;
    pCfgPkt->ServerDynamicIP           = STOE_GetIPAddr();
    pCfgPkt->ServerStaticIP            = gconfig_ConfigData.ServerStaticIP;
    pCfgPkt->ServerDataPktListenPort   = gconfig_ConfigData.ServerDataPktListenPort;
    pCfgPkt->MasterAddress             = gconfig_ConfigData.MasterAddress;
    pCfgPkt->ServerBroadcastListenPort = gconfig_ConfigData.ServerBroadcastListenPort;
    pCfgPkt->ClientDestIP              = gconfig_ConfigData.ClientDestIP;
    pCfgPkt->ClientDestPort            = gconfig_ConfigData.ClientDestPort; 
    pCfgPkt->Netmask                   = gconfig_ConfigData.Netmask;
    pCfgPkt->EthernetTxTimer           = gconfig_ConfigData.EthernetTxTimer;

    if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
      pCfgPkt->DeviceStatus            = DEVICE_STATUS_IDLE;
    else
      pCfgPkt->DeviceStatus            = DEVICE_STATUS_CONNECTED;
} //End of GCONFIG_GetConfigPacket() //

// ----------------------------------------------------------------------------
// Name    : GCONFIG_ReadConfigData
// Purpose :
// Params  :
// Returns :
// Note    :
// ----------------------------------------------------------------------------
void GCONFIG_ReadConfigData(void)
{
#ifdef RuntimeCodeAt24KH // = 0x6000 = approx 0x6020 ROM offset
    bool bRet;

    printd("Read configuration data : ");

    bRet = gconfig_ReStoreParameter(0, &gconfig_ConfigData, sizeof(gconfig_ConfigData));

    if (bRet == FALSE || (gconfig_ConfigData.ServerStaticIP & 0x000000FF) == 0)         //
      {
      GCONFIG_ReadDefaultConfigData();                                                  // read the device cinfiguration data
//    gconfig_InDisplayConfigData(&gconfig_ConfigData);                                 // display the data for the user
      }
#else
    GCONFIG_ReadDefaultConfigData();                                                    // read the device cinfiguration data
//  gconfig_InDisplayConfigData(&gconfig_ConfigData);                                   // display the data for the user
#endif
} //End of GCONFIG_ReadConfigData() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_WriteConfigData
// Purpose : 
// Params  : void
// Returns : void
// Note           :
//----------------------------------------------------------------------------------------
void GCONFIG_WriteConfigData(void)
{
#ifdef RuntimeCodeAt24KH
    gconfig_StoreParameter(&gconfig_ConfigData, (word )sizeof(gconfig_ConfigData));
#endif
} //End of GCONFIG_WriteConfigData() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_ReadDefaultConfigData
// Purpose : 
// Params  : void
// Returns : void
// Note           :
//----------------------------------------------------------------------------------------
void GCONFIG_ReadDefaultConfigData(void)
{
    printd("Read default configuration data. \n\r");                                    //

    gconfig_ConfigData.CheckSum                  = 0x00;                                //
    gconfig_ConfigData.CfgLength                 = sizeof(gconfig_ConfigData);          //
    gconfig_ConfigData.UdpAutoConnectClientIp    = 0x00000000;                          //
    gconfig_ConfigData.UdpAutoConnectClientPort  = 0x0000;                              //

    gconfig_ConfigData.Option                    = 0;                                   //
    memcpy(&gconfig_ConfigData.DevName, &DefaultDevName, sizeof(GCONFIG_DEV_NAME));     //
    gconfig_ConfigData.Network                   = (GCONFIG_NETWORK_SERVER |            //
                                                    GCONFIG_NETWORK_DHCP_DISABLE |      //
                                                    GCONFIG_NETWORK_PROTO_UDP |         //
                                                    GCONFIG_NETWORK_PROTO_UDP_BCAST);   //
    gconfig_ConfigData.ServerStaticIP            = defaultServerStaticIP;                          //
    gconfig_ConfigData.ServerDataPktListenPort   = defaultDataPort;                       //
    gconfig_ConfigData.MasterAddress             = 0;                                   // 0
    gconfig_ConfigData.ServerBroadcastListenPort = GCONFIG_UDP_BCAST_SERVER_PORT;       //
    gconfig_ConfigData.ClientDestIP              = defaultClientDestIP;                 //
    gconfig_ConfigData.ClientDestPort            = defaultHostPort;                     //
    gconfig_ConfigData.Netmask                   = defaultsubnetmask;                   //
    gconfig_ConfigData.EthernetTxTimer           = 1;                                   //
} // End of GCONFIG_ReadDefaultConfigData //

/////////////////////////////////////////////////////////////////////////////////

// ---------------------------------------------------------------------------------------
// Name    : Simtek_Modes_Enable
// Purpose : use data stored with EDCT tool to enable different functions
// Params  : mask of bit to check for enable/disable of desired function
// Returns : ON if enabled, OFF if not
// Note    :
// ---------------------------------------------------------------------------------------
//
byte Simtek_Modes_Enable(byte mask)
{
    if ((gconfig_ConfigData.Option & mask) == 0)                                        // mask off all other bits and check remaing bit/s for state
      return OFF;                                                                       // debug if enabled turn mode off
    return ON;                                                                          // debug if disabled turn mode on
} //End of Simtek_Modes_Enable(byte mask) //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_IpAddr2Ulong()
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
ulong GCONFIG_IpAddr2Ulong(byte* pBuf, byte len)
{
    ulong ip     = 0;
    byte* point  = (byte*)&ip;
    byte  count  = 0;
    byte  count2 = 0;

    while (1)
      {
      count2 = 0;
      while ((*pBuf != '.') && len)
        {
        count2++;
        if (count2 > 3) return 0xffffffff;

        *point *= 10;
  
        if ((*pBuf < 0x3a) && (*pBuf > 0x2f))
          *point += (*pBuf - 0x30);
        else
          return 0xffffffff;

        pBuf++;
        len--;
        }
  
      if (len == 0) break;

      pBuf++;
      len--;
      count++;
      point++;

      if (count > 3) return 0xffffffff;
      }

    if (count != 3) return 0xffffffff;

    return ip;
} //End of GCONFIG_IpAddr2Ulong() //
/////////////////////////////////////////////////////////////////////////////////
// Get Functions

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetDeviceName
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetDeviceName(GCONFIG_DEV_NAME* pDevName)
{
    memcpy(&gconfig_ConfigData.DevName, pDevName, sizeof(GCONFIG_DEV_NAME)); 
} // End of GCONFIG_GetDeviceName() //


//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetNetwork
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetNetwork(word  val)
{
      gconfig_ConfigData.Network = val;
} // End of GCONFIG_GetNetwork() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetServerStaticIP
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetServerStaticIP(ulong ip)
{
      gconfig_ConfigData.ServerStaticIP = ip;
} // End of GCONFIG_SetServerStaticIP() //

// ----------------------------------------------------------------------------
// Name    : GCONFIG_SetServerDataPktListenPort
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetServerDataPktListenPort(word  port)
{
      gconfig_ConfigData.ServerDataPktListenPort = port;
} // End of GCONFIG_SetServerDataPktListenPort() //

// ----------------------------------------------------------------------------
// Name    : GCONFIG_SetHostMasterAddress
// Purpose :
// Params  :
// Returns :
// Note    :
// ----------------------------------------------------------------------------
void GCONFIG_SetHostMasterAddress(byte adrs)
{
    gconfig_ConfigData.MasterAddress = adrs;
} // End of GCONFIG_SetHostMasterAddress() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetServerBroadcastListenPort
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetServerBroadcastListenPort(word  port)
{
      gconfig_ConfigData.ServerBroadcastListenPort = port;
} // End of GCONFIG_SetServerBroadcastListenPort() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetClientDestIP
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetClientDestIP(ulong ip)
{
      gconfig_ConfigData.ClientDestIP = ip;
} // End of GCONFIG_SetClientDestIP() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetClientDestPort
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetClientDestPort(word  port)
{
      gconfig_ConfigData.ClientDestPort = port;
} // End of GCONFIG_SetClientDestPort() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetNetmask
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetNetmask(ulong netmask)
{
      gconfig_ConfigData.Netmask = netmask;
} // End of GCONFIG_SetNetmask() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_SetEthernetTxTimer
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void GCONFIG_SetEthernetTxTimer(word  timer)
{
      gconfig_ConfigData.EthernetTxTimer = timer;
} // End of GCONFIG_SetEthernetTxTimer() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetDeviceName
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
GCONFIG_DEV_NAME* GCONFIG_GetDeviceName(void)
{
      return &gconfig_ConfigData.DevName;
} // End of GCONFIG_GetDeviceName() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetMacAddress
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
GCONFIG_MAC_ADDR GCONFIG_GetMacAddress(void)
{
      return macAddr;
} // End of GCONFIG_GetMacAddress() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetNetwork
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
word  GCONFIG_GetNetwork(void)
{
      return gconfig_ConfigData.Network;
} // End of GCONFIG_GetNetwork() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetServerStaticIP
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
ulong GCONFIG_GetServerStaticIP(void)
{
      return gconfig_ConfigData.ServerStaticIP;
} // End of GCONFIG_GetServerStaticIP() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetHostMasterAddress
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
byte  GCONFIG_GetHostMasterAddress(void)
{
    return gconfig_ConfigData.MasterAddress;
} // End of GCONFIG_GetHostMasterAddress() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetServerDataPktListenPort
// Purpose : 
// Params  : void
// Returns :
// Note           :
//----------------------------------------------------------------------------------------
word  GCONFIG_GetServerDataPktListenPort(void)
{
    return gconfig_ConfigData.ServerDataPktListenPort;
} // End of GCONFIG_GetServerDataPktListenPort() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetServerBroadcastListenPort
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
word  GCONFIG_GetServerBroadcastListenPort(void)
{
      return gconfig_ConfigData.ServerBroadcastListenPort;
} // End of GCONFIG_GetServerBroadcastListenPort() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetClientDestIP
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
ulong GCONFIG_GetClientDestIP(void)
{
      return gconfig_ConfigData.ClientDestIP;
} // End of GCONFIG_GetClientDestIP() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetClientDestPort
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
word  GCONFIG_GetClientDestPort(void)
{
      return gconfig_ConfigData.ClientDestPort;
} // End of GCONFIG_GetClientDestPort() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetNetmask
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
ulong GCONFIG_GetNetmask(void)
{
      return gconfig_ConfigData.Netmask;
} // End of GCONFIG_GetNetmask() //

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_GetEthernetTxTimer
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
word  GCONFIG_GetEthernetTxTimer(void)
{
      return gconfig_ConfigData.EthernetTxTimer;
} // End of GCONFIG_GetEthernetTxTimer() //

//----------------------------------------------------------------------------------------
// Name    : gconfig_ReStoreParameter
// Purpose : read back the ethernet configuration data at startup or reboot 
// Params  : addr - base address to read from
//                  pBase - pointer to the location to write data to(RAM), or read data from(FLASH)
//                  len - number of bytes to read back
// Returns : False if an error occurs during read (bad data, or faulty checksum)
//                  True if all reads and writes completed sucessfully and check sums match
// Note    : All the procedeing notes and following comments were added by cjm and are the assumed functions.
//                  This block was left blank by the ASIX software engineers.
//----------------------------------------------------------------------------------------
static bool gconfig_ReStoreParameter(ulong addr, GCONFIG_CFG_DATA *pConfig, word  len)
{
	word 	i;
	byte	*pParaBaseByte;
	byte	lastWtst = 0;

	pParaBaseByte = (byte *)pConfig;

	lastWtst = WTST;
	WTST = PROG_WTST_7;
	if (CSREPR & PMS) // SRAM shadow
	  {
	  CSREPR |= FAES;
	  for (i = 0 ; i < len ; i++)
	    {
	    *(pParaBaseByte + i) = FCVAR(byte, GCONFIG_CFG_DATA_FLASH_ADDR + addr + i);
	    }
	  CSREPR &= ~FAES;
	  }
	else
	  {
	  CSREPR &= ~FAES;
	  for (i = 0 ; i < len ; i++)
	    {
	    *(pParaBaseByte + i) = FCVAR(byte, GCONFIG_CFG_DATA_FLASH_ADDR + addr + i);
	    }
	  }
	WTST = lastWtst;

	if (0xffff != gconfig_Checksum((word  *)pConfig, len))
	  {
	  printd("failed! (wrong checksum)\n\r");
	  return FALSE;
	  }
	else if (pConfig->CfgLength != len)
	  {
	  printd("Invalid configuration data!\n\r");
	  return FALSE;
	  }
	printd("pass   len = %05du\n\r", len);
//    gconfig_InDisplayConfigData(pConfig);
	return TRUE;
} // End of gconfig_ReStoreParameter() //

//----------------------------------------------------------------------------------------
// Name    : gconfig_StoreParameter
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
static bool gconfig_StoreParameter(GCONFIG_CFG_DATA *pConfigData, word  len)
{
	byte			ret = FALSE;
	pFlashParaData  ptrFlashWrite = (void code*)0x003F00;

      pConfigData->CheckSum = 0;
      pConfigData->CfgLength = len;
      pConfigData->CheckSum = ~gconfig_Checksum((word  *)pConfigData, len);
    
	ret = (*ptrFlashWrite)((byte *)pConfigData, len);	

	if (ret == FALSE)
		printd("Failed to program Configration Data!!\n\r");
	return ret;
} // End of gconfig_StoreParameter() //

//----------------------------------------------------------------------------------------
// Name    : gconfig_Checksum
// Purpose :
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
static word  gconfig_Checksum(word  *pBuf, ulong length)
{
	ulong	cksum = 0;
	ulong	i, count;
	byte*	inChar = &pBuf;
	word *	point;

	count = length/2;

    for (i = 0; i < count; i++)
      {
      point = pBuf;
      cksum += *pBuf++;
      if (point > pBuf)
        (*inChar) = *inChar + 1;

      if (!(i % 0x8000))
        {
        cksum = (cksum >> 16) + (cksum & 0xffff);
        cksum += (cksum >> 16);
        cksum &= 0xffff;
        }
      }
            
    if (length % 2)
      cksum += (*pBuf & 0xff00);

    cksum  = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    cksum &= 0xffff;

    return ((word )cksum);
} // End of gconfig_Checksum() //

//----------------------------------------------------------------------------------------
// Name    : gconfig_DisplayConfigData
// Purpose : display configuration data in the debug window
// Params  : pointer to configuration packet
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
static void gconfig_DisplayConfigData(GCONFIG_CFG_DATA *pConfigData)
{
    byte  i, tmp, *pData;
    word  val;

    if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
      {
      printd("  Device Name           : ");
      for (i = 0; i < 16; i++)
        printd ("%c", pConfigData->DevName.DevName[i]);

      printd("\n\r");
      printd("  MAC Address           : ");
      for (i = 0; i < 6; i++)
        printd ("%02bx ", macAddr.MacAddr[i]);
      printd("\n\r");
      printd("  Network Settings      : ");
      printd ("%04dx\r\n", pConfigData->Network);
      val = pConfigData->Network;
      if (val & GCONFIG_NETWORK_CLIENT)
        printd("    Server/Client       : Client");
      else
        printd("    Server/Client       : Server");
      printd("\n\r");
      if (val & GCONFIG_NETWORK_DHCP_ENABLE)
        printd("    DHCP                : Enabled");
      else
        printd("    DHCP                : Disabled");
      printd("\n\r");
      if (val & GCONFIG_NETWORK_PROTO_UDP)
        printd("    UDP                 : On");
      else
        printd("    UDP                 : Off");
      if (val & GCONFIG_NETWORK_PROTO_TCP)
        printd("          TCP         : On");
      else
        printd("               TCP   : Off");
      printd("\n\r");
      if (val & GCONFIG_NETWORK_PROTO_UDP_BCAST)
        printd("    BroadCast           : On");
      else
        printd("    BroadCast           : Off");
      printd("\n\r");
      printd("  Device Server IP      : ");
      tmp    = 0;
      pData  = (byte *)&pConfigData->ServerStaticIP;
      for (i = 0; i < 3; i++)
        tmp += printd ("%bu.", *(pData + i));
      tmp   += printd ("%bu", *(pData + i));
      tmp    = 17 - tmp;                                                                     // find the number of characters of IP 255.255.255.255(15) plus white space(3) minus the number printed
      if (tmp < 0) tmp = 0;
      for(i  = 0; i < tmp; i++)
        printd(" ");
      printd("Port  : ");
      printd("%du", pConfigData->ServerDataPktListenPort);
      printd("\n\r");
// Client Port Information
      tmp    = 0;
      printd("  Host Server IP        : ");
      pData  = (byte *)&pConfigData->ClientDestIP;
      for (i = 0; i < 3; i++)
        tmp += printd ("%bu.", *(pData + i));
      tmp   += printd ("%bu", *(pData + i));
      tmp    = 17 - tmp;                                                                     // find the number of characters of IP 255.255.255.255(15) plus white space(3) minus the number printed
      if (tmp < 0) tmp = 0;
      for(i  = 0; i < tmp; i++)
        printd(" ");
      printd("Port  : ");
      printd("%du", pConfigData->ClientDestPort);
      printd("\n\r");
      }
} // End of gconfig_DisplayConfigData() //

//----------------------------------------------------------------------------------------
// Name    : gconfig_ExDisplayConfigData
// Purpose : display configuration data from external call
// Params  : void
// Returns : void
// Note    : 
//----------------------------------------------------------------------------------------
void gconfig_ExDisplayConfigData(void)//&gconfig_ConfigData
{
      gconfig_DisplayConfigData(&gconfig_ConfigData);                                     // output the configuration data to the debug port
}

//----------------------------------------------------------------------------------------
// Name    : gconfig_InDisplayConfigData
// Purpose : display configuration data from internal call
// Params  : pointer to configuration packet
// Returns : void
// Note    : 
//----------------------------------------------------------------------------------------
void gconfig_InDisplayConfigData(GCONFIG_CFG_DATA *pConfig)//&gconfig_ConfigData
{
      if (GS2E_GetTaskState() == GS2E_STATE_IDLE)
        gconfig_DisplayConfigData(pConfig);                                               // output the configuration data to the debug port
}

//----------------------------------------------------------------------------------------
// Name    : GCONFIG_EnableFirmwareUpgrade
// Purpose : 
// Params  : void
// Returns : void
// Note    : 
//----------------------------------------------------------------------------------------
void GCONFIG_EnableFirmwareUpgrade(void)
{
      FirmwareUpgradeFlag = ENABLE_FIRMWARE_UPGRADE;
} // End of GCONFIG_EnableFirmwareUpgrade() //


// End of gconfig.c //
