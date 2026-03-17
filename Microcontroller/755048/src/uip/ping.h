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
 * Module Name:ping.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: ping.h,v $
 * Revision 1.1  2006/05/12 14:24:06  robin6633
 * no message
 *
 *
 *=============================================================================
 */

#ifndef __PING_H__
#define __PING_H__

/* INCLUDE FILE DECLARATIONS */
#include "stoe.h"
#include "mstimer.h"

#define INCLUDE_PING_REQUEST   0 // 1: enable ping request function

/* NAMING CONSTANT DECLARATIONS */
#define PING_TIMEOUT	(SWTIMER_COUNT_SECOND)
#define MAX_PING_LEN	(MAX_ETHERNET_LEN - 42)

#define PING_ECHO_REQUEST		8
#define PING_ECHO_REPLY			0
#if (STOE_TRANSPARENT)
  #define IP_HEADER_OFFSET		14
  #define ICMP_HEADER_OFFSET	34	/*(14 + 20)*/
#else
  #define IP_HEADER_OFFSET		0
  #define ICMP_HEADER_OFFSET	(IP_HEADER_OFFSET + 20)
#endif

/* TYPE DECLARATIONS */
//---------------------------------------------------------------
typedef struct _PING_CONNECT
{
	U32_T	IP;
	U32_T	Sent;
	U32_T	ReplyGood;
	U16_T	ReplyBad;
	U16_T	Sequence;
	U16_T	Length;
	U16_T	Count;
	U16_T	Timer;
	U8_T	Wait;

} PING_CONNECT;

//---------------------------------------------------------------
typedef struct _ICMP_HEADER
{
	U8_T	Type;
	U8_T	Code;
	U16_T	CheckSum;
	U16_T	Id_Data;
	U8_T	Sequence[2];
	U8_T	Data;

} ICMP_HEADER;

// --------------------------------------------------------------
typedef struct _IP_HEADER
{
	U8_T	VerHlen;
	U8_T	TOS;
	U16_T	Length;
	U16_T	ID;
	U16_T	Frags;
	U8_T	TTL;
	U8_T	Protocol;
	U16_T	CheckSum;
	U32_T	SourIp;
	U32_T	DestIp;
	U8_T	Option_Data;

} IP_HEADER;

/* GLOBAL VARIABLES */
extern PING_CONNECT PingConnect;
extern U8_T XDATA PingFlag;

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void PING_Init(void);
void PING_EchoCheck(void);
void PING_ReplyCheck(void);
void PING_Test(U32_T);

#endif /* End of __PING_H__ */

/* End of ping.h */