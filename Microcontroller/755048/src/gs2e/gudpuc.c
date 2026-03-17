//----------------------------------------------------------------------------------------
//     Copyright (c) 2006     ASIX Electronic Corporation      All rights reserved.       //
//                                                                                        //
//     This is unpublished proprietary source code of ASIX Electronic Corporation         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//----------------------------------------------------------------------------------------
// Module Name : gudpuc.c                                                                 //
// Purpose     :                                                                          //
// Author      :                                                                          //
// Date        :                                                                          //
// Notes       :                                                                          //
// $Log        : gudpuc.c,v $                                                             //
//----------------------------------------------------------------------------------------
// Purpose     : revised file. adding command to allow dimming curve changes              //
// Author      : c.mason                                                                  //
// Date        : November 2, 2015                                                         //
// Notes       :                                                                          //
//----------------------------------------------------------------------------------------

// INCLUDE FILE DECLARATIONS //
#include <string.h>
#include "adapter.h"
#include "gudpuc.h"
#include "gs2e.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart.h"
#include "gconfig.h"
#include "ax11000.h"
#include "mac.h"
#include "printd.h"
#include "gudpdat.h"
//#include "hsuart.h"
#include "HARDWARE.H"

// NAMING CONSTANT DECLARATIONS //

// signature(12) macAddr(6) commmand(1) table(1) 24//
#define SIMTEK_HEADER_LEN 24
// signature(8) commmand(1) option(1) devname(16) macAddr(6) //
#define GUDPUC_AUTHENTIC_HEADER_LEN 32
#define GUDPUC_MAX_CONNS                  1
#define GUDPUC_NO_NEW_CONN                0xFF

#define GUDPUC_STATE_FREE                 0
#define GUDPUC_STATE_WAIT                 1
#define GUDPUC_STATE_CONNECTED            2

#define SCfg_Cmd_DEFAULT_REQ              0x00
#define SCfg_Cmd_DEFAULT_ACK              0x01
#define SCfg_Cmd_UPDATE_REQ               0x02
#define SCfg_Cmd_UPDATE_ACK               0x03
#define SCfg_Cmd_BURN_REQ                 0x04
#define SCfg_Cmd_BURN_ACK                 0x05
#define SCfg_Cmd_READ_REQ                 0x06
#define SCfg_Cmd_READ_ACK                 0x07
// GLOBAL VARIABLES DECLARATIONS //
// TYPE DECLARATIONS //
typedef struct _SID
{
      byte                                id[12]; 
} SID;

typedef struct _SCONFIG_DEV_NAME
{
      byte                                DevName[16]; 
} SCONFIG_DEV_NAME;

typedef struct _SCONFIG_MAC_ADDR
{
      byte                                MacAddr[6]; 
} SCONFIG_MAC_ADDR;

typedef struct _SCONFIG_CFG_PKT
{
      SID                                 Sid;                        // length 16  bytes = 16
      byte                                Command;                    // length 1   bytes = 17
      GCONFIG_MAC_ADDR                    MacAddr;                    // length 6   bytes = 23
      byte                                Table;                      // length 1   bytes = 24
      word                                dim_pnt[256];               // length 256 bytes = 280
} SCONFIG_CFG_PKT;

SID XDATA Sid = {'S','i','m','t','e','k','k','e','t','m','i','S'};

// LOCAL VARIABLES DECLARATIONS //
static GUDPUC_CONN gudpuc_Conns[GUDPUC_MAX_CONNS];
static byte gudpuc_InterAppId;
GCONFIG_CFG_PKT gudpuc_ConfigRxPkt;
GCONFIG_MONITOR_PKT *gudpuc_MonitorPkt;

SCONFIG_CFG_PKT simtek_ConfigPkt;

// LOCAL SUBPROGRAM DECLARATIONS //
void gudpuc_HandleSearchReq(byte id);
void gudpuc_HandleSetReq(byte XDATA* pData, word  length, byte id);
void gudpuc_HandleRebootReq(byte XDATA* pData, word  length, byte id);
void gudpuc_RebootDevice(void);

// ----------------------------------------------------------------------------
// Function Name: GUDPUC_Task
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPUC_Task(void)
{

} // End of GUDPUC_Task() //

// ----------------------------------------------------------------------------
// Function Name: GUDPUC_Init()
// Purpose: Initialization
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPUC_Init(word  localPort)
{
      byte  i;

      for (i = 0; i < GUDPUC_MAX_CONNS; i++)
        gudpuc_Conns[i].State = GUDPUC_STATE_FREE;
      gudpuc_InterAppId = TCPIP_Bind(GUDPUC_NewConn, GUDPUC_Event, GUDPUC_Receive);
      // unicast packet //
        printd("UDP unicast port        : ");                                           // let the user know the the port is initailization attempt
      if (gudpuc_InterAppId == TCPIP_NO_NEW_CONN)                                         // check if the binding failed
        printd(" **BINDING ERROR!**\n\r");                                                // FAILED
      else
        {
//        printd("ok. port : ");                                                           // ip binding completed!
        i = TCPIP_UdpListen(localPort, gudpuc_InterAppId);                                // attempt to set the listening port for the binding
        if(i == 1)                                                                        // if 1 the sucessful, else failed
          printd("%du\n\r",localPort);                                                    // set listen socket completed!
        else
          printd("* BINDING ERROR! *\n\r");                                               // set listen socket FAILED! socket ERROR
        }
} // End of GUDPUC_Init() //

// ----------------------------------------------------------------------------
// Function Name: GUDPUC_NewConn
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
byte GUDPUC_NewConn(ulong XDATA* pip, word  remotePort, byte socket)
{
      byte  i;

      pip = pip;
      remotePort = remotePort;      
      for (i = 0; i < GUDPUC_MAX_CONNS; i++)
        {
        if (gudpuc_Conns[i].State != GUDPUC_STATE_FREE)
          TCPIP_UdpClose(gudpuc_Conns[i].UdpSocket); //tony 2009-4-20
        gudpuc_Conns[i].State = GUDPUC_STATE_CONNECTED;
        gudpuc_Conns[i].UdpSocket = socket;
        return i;
        }
      return GUDPUC_NO_NEW_CONN;
} // End of GUDPUC_NewConn() //

// ----------------------------------------------------------------------------
// Function Name: GUDPUC_Event
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPUC_Event(byte id, byte event)
{
      gudpuc_Conns[id].State = event;
} // End of GUDPUC_Event() //

// ----------------------------------------------------------------------------
// Function Name: GUDPUC_Receive
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPUC_Receive(byte XDATA* pData, word  length, byte id)
{
      bool             bValidReq = FALSE;
      GCONFIG_MAC_ADDR macAddr;

//  printd("rucd\n\r");
      if (length == 9 || length == sizeof(gudpuc_ConfigRxPkt))
        {
        memcpy(&gudpuc_ConfigRxPkt, pData, length);
//  printd("passed len check\n\r");

        if (memcmp(&GCONFIG_Gid, pData, sizeof(GCONFIG_GID)) == 0) // Valid data
          {
//  printd("passed GID check\n\r");
          macAddr = GCONFIG_GetMacAddress();    
//  printd("MAC = %02bx%02bx%02bx%02bx%02bx%02bx\n\r",macAddr.MacAddr[0], macAddr.MacAddr[1], macAddr.MacAddr[2], macAddr.MacAddr[3], macAddr.MacAddr[4], macAddr.MacAddr[5]);
//  printd("len = %du\n\r",length);
          if (length > GUDPUC_AUTHENTIC_HEADER_LEN)
            {
            if (memcmp(&gudpuc_ConfigRxPkt.MacAddr, &macAddr, sizeof(macAddr)) == 0)
              {
              bValidReq = TRUE;
//  printd("passed MAC check\n\r");
              }
            //else
//  printd("failed MAC check\n\r");
            }
          switch (gudpuc_ConfigRxPkt.Opcode)
            {
            case GCONFIG_OPCODE_SEARCH_REQ:
                {
                gudpuc_HandleSearchReq(id);
//  printd("passed SEARCH check\n\r");
                }
                break;
            case GCONFIG_OPCODE_SET_REQ:
                {                         
                if (bValidReq && length == sizeof(gudpuc_ConfigRxPkt))
                  gudpuc_HandleSetReq(pData, length, id);
//  printd("passed SET check\n\r");
                }
                break;
            case GCONFIG_OPCODE_REBOOT_REQ:
                {                         
                if(bValidReq  == TRUE) printd("bValidReq = TRUE && ");
                if(fDebugMode == ON)   printd("fDebugMode = ON && ");
                if(length     == sizeof(gudpuc_ConfigRxPkt)) printd("Length = Good\n\r"); else  printd("Length = Bad\n\r");
                if (bValidReq && (length == sizeof(gudpuc_ConfigRxPkt)) && (fDebugMode == ON))
                  gudpuc_HandleRebootReq(pData, length, id);
//  printd("passed REBOOT check\n\r");
                }
                break;
            default:
//  printd("*****  failed UC data packet *****\n\r");
                break;  
            }// switch (gudpuc_ConfigRxPkt.Command)
          }// if (memcmp(&GCONFIG_Gid, pData, sizeof(GCONFIG_GID)) == 0)
  }// if (length == 9 || length == sizeof(gudpuc_ConfigRxPkt))
//    else if (length == sizeof(simtek_ConfigPkt))
//      {
//      memcpy(&simtek_ConfigPkt, pData, length);
//      if (memcmp(&Sid, pData, sizeof(SID)) == 0) // Valid data
//        {
//        macAddr = GCONFIG_GetMacAddress();
//        if (length > SIMTEK_HEADER_LEN)
//          {
//          if (memcmp(&simtek_ConfigPkt.MacAddr, &macAddr, sizeof(macAddr)) == 0)
//            bValidReq = TRUE;
//          }
//        switch (simtek_ConfigPkt.Command)
//          {
//          case SCfg_Cmd_DEFAULT_REQ:
//              {
//              }
//              break;
//          case SCfg_Cmd_UPDATE_REQ:
//              {
//              }
//              break;
//          case SCfg_Cmd_BURN_REQ:
//              {                         
//              }
//              break;
//          case SCfg_Cmd_READ_REQ:
//              {                         
//              }
//              break;
//          default:
//              break;  
//          }      // switch (simtek_ConfigPkt.Command)
//        }  // if (memcmp(&Sid, pData, sizeof(SID)) == 0)
//      }  // else if (length == sizeof(simtek_ConfigPkt))
} // End of GUDPUC_Receive() //

// ----------------------------------------------------------------------------
// Function Name: gudpuc_HandleSearchReq
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void gudpuc_HandleSearchReq(byte id) 
{     
      GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
      gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_SEARCH_ACK;
      TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, (byte*) &gudpuc_ConfigRxPkt, sizeof(gudpuc_ConfigRxPkt));
} // End of gudpuc_HandleSearchReq() //

// ----------------------------------------------------------------------------
// Function Name: gudpuc_HandleSetReq
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void gudpuc_HandleSetReq(byte XDATA* pData, word  length, byte id)
{
      byte reboot = (gudpuc_ConfigRxPkt.Option & GCONFIG_OPTION_ENABLE_REBOOT);

      GCONFIG_SetConfigPacket(&gudpuc_ConfigRxPkt);
      *(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_SET_ACK;
      TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, pData, length);

      if (reboot == GCONFIG_OPTION_ENABLE_REBOOT)
        {
        gudpuc_RebootDevice();
        }   
} // End of gudpuc_HandleSetReq() //

// ----------------------------------------------------------------------------
// Function Name: gudpuc_HandleRebootReq
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void gudpuc_HandleRebootReq(byte XDATA* pData, word  length, byte id)
{
      *(pData + GCONFIG_OPCODE_OFFSET) = GCONFIG_OPCODE_REBOOT_ACK;
      TCPIP_UdpSend(gudpuc_Conns[id].UdpSocket, 0, 0, pData, length);
      gudpuc_RebootDevice(); 
}
// End of gudpuc_HandleRebootReq() //

// ----------------------------------------------------------------------------
// Function Name: gudpuc_RebootDevice
// Purpose: Delay 10 ms before reboot
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void gudpuc_RebootDevice(void)
{
      ulong timeStart = SWTIMER_Tick();
      ulong timeEnd = timeStart;    

      while (timeEnd == timeStart)
        {
        timeEnd = SWTIMER_Tick();
        }
      AX11000_SoftReboot();
} // End of gudpuc_RebootDevice() //

// End of gudpuc.c //
