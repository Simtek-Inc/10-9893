//----------------------------------------------------------------------------------------
//     Copyright (c) 2011	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended 
//     publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name:ping.h
// Purpose:
// Author:
// Date:
// Notes:
// $Log: ping.h,v $
//----------------------------------------------------------------------------------------

#ifndef __PING_H__
#define __PING_H__

// INCLUDE FILE DECLARATIONS //
#include "types.h"

// NAMING CONSTANT DECLARATIONS //

// TYPE DECLARATIONS //
typedef struct _PING_CONFIG_SET
{
	ulong TargetIP;
	word  PatternLen;
	word  EchoTime;		
	byte  TTL;
	byte  TxCount;
	byte  RxCount;
	
	word  PingID;
	word  Sequence;	
} tsPING_CONFIG_SET;

struct _PING_LINK
{
	struct _PING_LINK *pPre;
	struct _PING_LINK *pNext;
	word   WaitTime;
	word   ReplyLen;
	byte   Buf[1];
};
typedef struct _PING_LINK tsPING_LINK;

// GLOBAL VARIABLES //
extern byte  XDATA PingFlag;// for mwif compatable //

// EXPORTED SUBPROGRAM SPECIFICATIONS //
void PING_Init(void);
void PING_Task(void);
char PING_CmdEnQueue(tsPING_LINK *pEntity);
char PING_CmdDeQueue(tsPING_LINK *pEntity);

#endif // End of __PING_H__ //

// End of ping.h //
