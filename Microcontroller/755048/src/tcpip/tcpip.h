//=============================================================================
//      Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
// 
//      This is unpublished proprietary source code of ASIX Electronic Corporation
// 
//      The copyright notice above does not evidence any actual or intended
//      publication of such source code.
//=============================================================================
//  Module Name:tcpip.h
//  Purpose:
//  Author:
//  Date:
//  Notes:
//  $Log: tcpip.h,v $
//=============================================================================

#ifndef __TCPIP_H__
#define __TCPIP_H__

// INCLUDE FILE DECLARATIONS //
#include "types.h"

// NAMING CONSTANT DECLARATIONS //
#define TCPIP_MAX_APP_CONN			16
#define TCPIP_MAX_TCP_LISTEN		      12
#define TCPIP_MAX_UDP_LISTEN		      12
#define TCPIP_MAX_TCP_CONN			16
#define TCPIP_MAX_UDP_CONN			16

#define TCPIP_NO_NEW_CONN			0xff

#define	TCPIP_CONNECT_CANCEL		0
#define TCPIP_CONNECT_WAIT			1
#define TCPIP_CONNECT_ACTIVE		      2
#define TCPIP_CONNECT_XMIT_COMPLETE	      3
#define TCPIP_CONNECT_BUSY			0xf1

#define TCPIP_SEND_NOT_FINAL		      0
#define TCPIP_SEND_FINAL			1
#define TCPIP_SEND_NOT_PUSH			2

#define TCPIP_KEEPALIVE_ON			1
#define TCPIP_KEEPALIVE_OFF			0

#define TCPIP_DONT_FRAGMENT			1
#define TCPIP_FRAGMENT				0

#define TCPIP_PROTOCOL_ICMP			1
#define TCPIP_PROTOCOL_TCP			6
#define TCPIP_PROTOCOL_UDP			17

#define TCPIP_NON_TRANSPARENT_MODE	      0
#define TCPIP_TRANSPARENT_MODE		1

#define TCPIP_TIME_TO_LIVE			255	// TTL item of ip layer //

// GLOBAL VARIABLES //

// for uip //
extern byte xdata uip_buf[];
extern volatile word idata uip_len;
extern volatile byte *uip_appdata;
extern byte xdata uip_getgatewaymacaddrflag;

#define TCPIP_GetRcvBuffer()		(uip_buf)
#define TCPIP_GetXmitBuffer()		(uip_buf)
#define TCPIP_SetRcvLength(len)	(uip_len = len)
#define TCPIP_SetXmitLength(len)	(uip_len = len)
#define TCPIP_GetXmitLength()		(uip_len)
#define TCPIP_GetPayloadBuffer()	(uip_appdata)
#define TCPIP_GetGwMacAddrFlag()	(uip_getgatewaymacaddrflag)
#define TCPIP_SetGwMacAddrFlag(a)	(uip_getgatewaymacaddrflag = a)

// EXPORTED SUBPROGRAM SPECIFICATIONS //
void TCPIP_Init(byte);
byte TCPIP_Bind(byte (* )(ulong XDATA*, word, byte), void (* )(byte, byte), void (* )(byte XDATA*, word, byte));
void TCPIP_Unbind(byte);

// for tcp //
byte TCPIP_TcpListen(word, byte);
void TCPIP_TcpUnlisten(word);
byte TCPIP_TcpNew(byte, byte, ulong, word, word);
void TCPIP_TcpConnect(byte);
void TCPIP_TcpKeepAlive(byte, byte);
void TCPIP_TcpClose(byte);
word TCPIP_QueryTcpLocalPort(byte);
void TCPIP_TcpSend(byte, byte*, word, byte);

// for udp //
byte TCPIP_UdpListen(word, byte);
void TCPIP_UdpUnlisten(word);
byte TCPIP_UdpNew(byte, byte, ulong, word, word);
void TCPIP_UdpClose(byte);
word TCPIP_QueryUdpLocalPort(byte);
void TCPIP_UdpSend(byte, byte*,byte, byte*, word);

// for other purpose //
void TCPIP_AssignLowlayerXmitFunc(void (* )(word));
void TCPIP_AssignPingRespFunc(void (* )(void));
void TCPIP_PeriodicCheck(void);
void TCPIP_SetPppoeMode(byte);
ulong TCPIP_GetIPAddr(void);
ulong TCPIP_GetSubnetMask(void);
ulong TCPIP_GetGateway(void);
void TCPIP_SetIPAddr(ulong);
void TCPIP_SetSubnetMask(ulong);
void TCPIP_SetGateway(ulong);
void TCPIP_SetMacAddr(byte*);
void TCPIP_Receive(void);
void TCPIP_DontFragment(byte);

// for uip //
word htons(word);

// for uip_arp //
void uip_arp_ipin(void);
void uip_arp_arpin(void);
void uip_arp_out(void);

// for debug //
//void TCPIP_Debug(void);

#endif // End of __TCPIP_H__ //


// End of tcpip.h //