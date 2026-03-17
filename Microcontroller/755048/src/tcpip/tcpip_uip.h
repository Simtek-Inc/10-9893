//=============================================================================
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//=============================================================================
// Module Name:tcpip_uip.h
// Purpose:
// Author:
// Date:
// Notes:
// $Log: tcpip_uip.h,v $
//=============================================================================

#ifndef __TCPIP_UIP_H__
#define __TCPIP_UIP_H__

// INCLUDE FILE DECLARATIONS //
#include "types.h"

// NAMING CONSTANT DECLARATIONS //
#define TCPIP_STATE_FREE			0
#define TCPIP_STATE_WAIT_CONNECT	1
#define TCPIP_STATE_CONNECT			2
#define TCPIP_STATE_WAIT_FIN		3
#define TCPIP_STATE_LAST_ACK		4

#define TCPIP_KEEPALIVE_PERIOD		(240 * 2) // 2 mimute //
#define TCPIP_CONN_TIMEOUT			(240 * 8) // 8 minute //

// UIP_APPCALL: the name of the application function. This function
//   must return void and take no arguments (i.e., C type "void
//   appfunc(void)").
#ifndef UIP_APPCALL
#define UIP_APPCALL()	TCPIP_CallTcpApp()
#endif

// TYPE DECLARATIONS //
// ------------------------------------------------------------- //
typedef struct _TCPIP_APPLICATION
{
	byte	Valid;
	byte	(*NewConnHandle)(ulong XDATA*, word, byte);
	void	(*EventHandle)(byte, byte);
	void	(*ReceiveHandle)(byte XDATA*, word, byte);

} TCPIP_APPLICATION;

// ------------------------------------------------------------- //
typedef struct _TCPIP_LISTEN
{
	word	LocalPort;
	byte	InterfaceId;// TCPIP_APPLICATION Connect ID //

} TCPIP_LISTEN;

// ------------------------------------------------------------- //
typedef struct _TCPIP_UDP_CONNECT
{
	byte	State;
	byte	InterfaceId;// TCPIP_APPLICATION Connect ID //
	byte	AppId;		// Uplayer Connect ID //
	ulong	Ip;
	word	LocalPort;
	word	RemotePort;
	byte	HeadLen;
	word	PayloadLen;
	byte*	PHead;		// point to up-layer protocol header //
	byte*	PPayload;	// point to up_layer protocol payload //
	void XDATA*	PSocket;// UDP socket //

} TCPIP_UDP_CONNECT;

// ------------------------------------------------------------- //
typedef struct _TCPIP_TCP_CONNECT
{
	byte	State;
	byte	InterfaceId;// TCPIP_APPLICATION Connect ID //
	byte	AppId;		// Uplayer Connect ID //
	byte	FinalFlag;
	ulong	Ip;
	byte	Seq[4];
	void XDATA*	PSocket;
	word	XmitLen;
	word	SecXmitLen;
	word	LeftLen;
	byte*	PData;
	byte*	PSecData;
	byte*	PNextData;
	byte	Count;

} TCPIP_TCP_CONNECT;

// GLOBAL VARIABLES //
extern byte TCPIP_PppoeHeaderLen;
extern byte TCPIP_IsPppoe;
extern byte TCPIP_DontFrag;
extern byte TCPIP_EthHeadLen;
extern word TCPIP_MssLen;
extern void (*TCPIP_PingRespCallBack)(void);


// EXPORTED SUBPROGRAM SPECIFICATIONS //
void TCPIP_CallTcpApp(void);
void TCPIP_CallUdpApp(void);


#endif // End of __TCPIP_UIP_H__ //


// End of tcpip_uip.h //