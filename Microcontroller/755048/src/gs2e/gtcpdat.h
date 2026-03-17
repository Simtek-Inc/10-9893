/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:gtcpdat.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gtcpdat.h,v $
 *
 *=============================================================================
 */

#ifndef __GGTCPDAT_H__
#define __GGTCPDAT_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define GTCPDAT_ENABLE_DEBUG_MSG	0 // 1: enable 0:disable
#define GTCPDAT_TX_BUF_SIZE       	1460 * 2 // Decrease the buffer size may cause unpredictable 
                                             // problems of data transmission from DS to VSP

/* TYPE DECLARATIONS */
typedef struct _GTCPDAT_CONN
{
	U32_T	Ip;
	U16_T	Port;
	U8_T	State;
	U8_T	TcpSocket;
	U16_T	Timer;
	U8_T	FileId;
} GTCPDAT_CONN;

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void GTCPDAT_Task(void);
void GTCPDAT_Init(U16_T);
U8_T GTCPDAT_NewConn(U32_T XDATA*, U16_T, U8_T);
void GTCPDAT_Event(U8_T, U8_T);
void GTCPDAT_Receive(U8_T XDATA*, U16_T, U8_T);

#endif /* End of __GGTCPDAT_H__ */

/* End of gtcpdat.h */
