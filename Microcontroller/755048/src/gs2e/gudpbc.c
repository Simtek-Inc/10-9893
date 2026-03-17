//=============================================================================
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//=============================================================================
//
//=============================================================================
// Module Name: gudpbc.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: gudpbc.c,v $
//
//=============================================================================

// INCLUDE FILE DECLARATIONS //
#include "adapter.h"
#include "gudpbc.h"
#include "gs2e.h"
#include "tcpip.h"
#include "uip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart.h"
#include "gconfig.h"
#include "ax11000.h"
#include "mac.h"
#include "printd.h"
#include <string.h>

// NAMING CONSTANT DECLARATIONS //
#define GUDPBC_ENABLE_DEBUG_MSG		0 // 1: enable 0:disable
#define GUDPBC_MAX_CONNS			1
#define GUDPBC_NO_NEW_CONN			0xFF

#define GUDPBC_STATE_FREE			0
#define GUDPBC_STATE_WAIT			1
#define GUDPBC_STATE_CONNECTED		2

// GLOBAL VARIABLES DECLARATIONS //
extern GCONFIG_CFG_PKT gudpuc_ConfigRxPkt;

// LOCAL VARIABLES DECLARATIONS //
static GUDPBC_CONN gudpbc_Conns[GUDPBC_MAX_CONNS];
static byte gudpbc_InterAppId;

// LOCAL SUBPROGRAM DECLARATIONS //
//  S2E Version 2.3.1 Tony 2009/4/17 Modify the reply address to broadcast address for the different IP subnet search

void gudpbc_HandleSearchReq(byte XDATA* pData, byte id);

// ----------------------------------------------------------------------------
// Function Name: GUDPBC_Task
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPBC_Task(void)
{

} // End of GUDPBC_Task() //

// ----------------------------------------------------------------------------
// Function Name: GUDPBC_Init()
// Purpose: Initialization
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPBC_Init(word localPort)
{
	byte	i;

	for (i = 0; i < GUDPBC_MAX_CONNS; i++)
	  gudpbc_Conns[i].State = GUDPBC_STATE_FREE;
	gudpbc_InterAppId = TCPIP_Bind(GUDPBC_NewConn, GUDPBC_Event, GUDPBC_Receive);

	// unicast packet //
      printd("UDP broadcast port      : ");                                             // let the user know the the port is not initailized, there was an error
	if (gudpbc_InterAppId == TCPIP_NO_NEW_CONN)                                         // check if the binding failed
        printd("**BINDING ERROR!**\n\r");                                                 // let the user know the the port is not initailized, there was an error
      else
        {
//      printd("ok. port : ");                                                            // ip binding completed!
        i = TCPIP_UdpListen(localPort, gudpbc_InterAppId);                                // attempt to set the listening port for the binding
        if(i == 1)
          printd("%du\n\r",localPort);                                                    // set listen socket completed!
        else
          printd("* BINDING ERROR! *\n\r");                                               // set listen socket FAILED! socket ERROR
        }
} // End of GUDPBC_Init() //

// ----------------------------------------------------------------------------
// Function Name: GUDPBC_NewConn
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
byte GUDPBC_NewConn(ulong XDATA* pip, word remotePort, byte socket)
{
	byte	i;

	pip = pip;
	remotePort = remotePort;

	for (i = 0; i < GUDPBC_MAX_CONNS; i++)
	  {
	  if (gudpbc_Conns[i].State != GUDPBC_STATE_FREE)
	    TCPIP_UdpClose(gudpbc_Conns[i].UdpSocket);
	  gudpbc_Conns[i].State = GUDPBC_STATE_CONNECTED;
	  gudpbc_Conns[i].UdpSocket = socket;
#if GUDPBC_ENABLE_DEBUG_MSG
	  printd("GUDPBC_NewConn(id=%bu, remotePort=%du, socket=%bu)\n\r",i, remotePort,socket);
#endif	
	  return i;
	  }
	return GUDPBC_NO_NEW_CONN;
} // End of GUDPBC_NewConn() //

// ----------------------------------------------------------------------------
// Function Name: GUDPBC_Event
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPBC_Event(byte id, byte event)
{
	gudpbc_Conns[id].State = event;

} // End of GUDPBC_Event() //

// ----------------------------------------------------------------------------
// Function Name: GUDPBC_Receive
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void GUDPBC_Receive(byte XDATA* pData, word length, byte id)
{
	bool bValidReq = FALSE;
	GCONFIG_MAC_ADDR macAddr;

	if (length == 9) // 9: search request packet
	  {
	  memcpy(&gudpuc_ConfigRxPkt, pData, 9);

	  if (memcmp(&GCONFIG_Gid, pData, sizeof(GCONFIG_GID)) == 0) // Valid data
	    {
	    macAddr = GCONFIG_GetMacAddress();	

	    switch (gudpuc_ConfigRxPkt.Opcode)
	      {
	      case GCONFIG_OPCODE_SEARCH_REQ:
	          {
	          gudpbc_HandleSearchReq(pData, id);
	          }
	          break;
	      default:
	          break;	
	      }
	    }
	  }
} // End of GUDPBC_Receive() //

// ----------------------------------------------------------------------------
// Function Name: gudpbc_HandleSearchReq
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void gudpbc_HandleSearchReq(byte XDATA* pData, byte id)
{	
	word		length, headerLen, tmpShort;
	byte XDATA*	pRcvBuf;
	byte XDATA*	pMacAddr;
	uip_udpip_hdr XDATA*	pUdpHeader;

	GCONFIG_GetConfigPacket(&gudpuc_ConfigRxPkt);
 	gudpuc_ConfigRxPkt.Opcode = GCONFIG_OPCODE_SEARCH_ACK;
	id = id;

	pRcvBuf = TCPIP_GetRcvBuffer();
	headerLen = (word)pData - (word)pRcvBuf;
	length = sizeof(gudpuc_ConfigRxPkt) + headerLen;

	// ethernet header //
	pMacAddr = MAC_GetMacAddr();
	pRcvBuf[0] = 0xFF;
	pRcvBuf[1] = 0xFF;
	pRcvBuf[2] = 0xFF;
	pRcvBuf[3] = 0xFF;
	pRcvBuf[4] = 0xFF;
	pRcvBuf[5] = 0xFF;
	pRcvBuf[MAC_ADDRESS_LEN] = pMacAddr[0];
	pRcvBuf[MAC_ADDRESS_LEN +1] = pMacAddr[1];
	pRcvBuf[MAC_ADDRESS_LEN +2] = pMacAddr[2];
	pRcvBuf[MAC_ADDRESS_LEN +3] = pMacAddr[3];
	pRcvBuf[MAC_ADDRESS_LEN +4] = pMacAddr[4];
	pRcvBuf[MAC_ADDRESS_LEN +5] = pMacAddr[5];

	// ip header //
	pUdpHeader = (uip_udpip_hdr XDATA*)(pRcvBuf + ETH_HEADER_LEN);
	tmpShort = length - ETH_HEADER_LEN;

	pUdpHeader->len[0] = (byte)(tmpShort >> 8);
	pUdpHeader->len[1] = (byte)tmpShort;
	pUdpHeader->ipchksum = 0;
	pUdpHeader->destipaddr[0] = 0xFFFF;
 	pUdpHeader->destipaddr[1] = 0xFFFF;

	uip_gethostaddr(pUdpHeader->srcipaddr);

	// udp header //
	pUdpHeader = (uip_udpip_hdr XDATA*)(pData - 28);
	tmpShort = pUdpHeader->srcport;
	pUdpHeader->srcport = pUdpHeader->destport;
	pUdpHeader->destport = tmpShort;
	pUdpHeader->udplen = sizeof(gudpuc_ConfigRxPkt) + 8;
	pUdpHeader->udpchksum = 0;

	TCPIP_SetXmitLength(length);

	uip_appdata = (byte*)&gudpuc_ConfigRxPkt;

	ETH_Send(headerLen);
	uip_slen = 0;

} // End of gudpbc_HandleSearchReq() //

// End of gudpbc.c //