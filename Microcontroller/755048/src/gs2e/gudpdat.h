//----------------------------------------------------------------------------------------
//     Copyright (c) 2006     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name:gudpdat.h
// Purpose:
// Author:
// Date:
// Notes:
// $Log: gudpdat.h,v $
//----------------------------------------------------------------------------------------

#ifndef __GGUDPDAT_H__
#define __GGUDPDAT_H__

// INCLUDE FILE DECLARATIONS //
#include "types.h"

// NAMING CONSTANT DECLARATIONS //
#define GUDPDAT_TX_BUF_SIZE                     1472
#define TrxSocket                                  4

// TYPE DECLARATIONS //
typedef struct _GUDPDAT_CONN
{
      ulong Ip;
      word  Port;
      byte  State;
      byte  UdpSocket;
      ulong Timer;
      ulong UrRxBytes;
      ulong UrTxBytes;
} GUDPDAT_CONN;

// GLOBAL VARIABLES //
//extern word  GTCPDAT_GetRandomPortNum(void);

// EXPORTED SUBPROGRAM SPECIFICATIONS //
void GUDPDAT_Task(void);
void GUDPDAT_Init(word );
byte GUDPDAT_NewConn(ulong XDATA*, word , byte);
void GUDPDAT_Event(byte, byte);
void GUDPDAT_Receive(byte XDATA*, word , byte);
//ulong GUDPDAT_GetTxBytes(byte id);
//ulong GUDPDAT_GetRxBytes(byte id);
void GUDPDAT_ConnClose(ulong remoteIp, word  port);
void gudpdat_ClientOpen(byte id);

#endif // End of __GGUDPDAT_H__ //

// End of gudpdat.h //
