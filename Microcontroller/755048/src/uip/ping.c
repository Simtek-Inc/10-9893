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
 /*============================================================================
 * Module Name: ping.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: ping.c,v $
 * Revision 1.5  2006/05/23 05:23:12  robin6633
 * 1.Add a define condition.
 *
 * Revision 1.4  2006/05/23 01:54:07  robin6633
 * 1.Removed debug information from UART2.
 *
 * Revision 1.3  2006/05/19 03:49:56  robin6633
 * 1.Fixed the ping timeout value.
 *
 * Revision 1.2  2006/05/17 08:27:49  robin6633
 * Changed the debug information display.
 *
 * Revision 1.1  2006/05/12 14:24:06  robin6633
 * no message
 *
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "ping.h"
#include "uip.h"

#if (INCLUDE_PING_REQUEST)
#include "uart.h"
#if (UART0_ENABLE)
  #include <stdio.h>
#endif

/* NAMING CONSTANT DECLARATIONS */

/* GLOBAL VARIABLES DECLARATIONS */
PING_CONNECT PingConnect;
U8_T XDATA PingFlag;

/* LOCAL VARIABLES DECLARATIONS */
static U8_T XDATA pingSequence[2] = {0, 0};
static U16_T ip_Id = 0;

/* LOCAL SUBPROGRAM DECLARATIONS */
static void ping_Send(void);

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_Init
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_Init(void)
{
	U8_T	i = sizeof (PingConnect);
	U8_T*	point = (U8_T*)&PingConnect;

	while (i--)
	{
		*point = 0;
		point++;
	}
	
} /* End of PING_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_EchoCheck
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_EchoCheck(void)
{
	U16_T	timer = (U16_T)SWTIMER_Tick();
	U8_T*	pbuf;

	if ((timer - PingConnect.Timer) >= PING_TIMEOUT)
	{
		if (PingConnect.Wait)
		{
			if ((timer - PingConnect.Timer) >= (2* PING_TIMEOUT))
			{
				PingConnect.Wait = 0;
				PingConnect.ReplyBad++;
#if (UART0_ENABLE)
				printd ("\rRequest timed out. %x\n\r", timer);
#endif
			}
		}

		if (PingConnect.Wait == 0)
		{
			if (PingConnect.Count == 0)
			{
				PingFlag = 0;
#if (UART0_ENABLE)
				pbuf = (U8_T*)&PingConnect.IP;
				printd ("\r\n\rPing statistics for %bu.%bu.%bu.%bu:\n\r",
					pbuf[0], pbuf[1], pbuf[2], pbuf[3]);
				/*printd ("\r    Packets : Sent = %lu; Received = %lu; Lose = %u\n\r\n\rAX11000>",
					PingConnect.Sent, PingConnect.ReplyGood, PingConnect.ReplyBad);*/
				printd ("\r    Packets : Sent = %lu; Received = %lu; Lose = %u\n\r\n\r",
					PingConnect.Sent, PingConnect.ReplyGood, PingConnect.ReplyBad);
#endif
			}
			else
			{
				if (PingConnect.Count != 0xffff)
					PingConnect.Count--;

				PingConnect.Wait = 1;
				PingConnect.Sent++;
				PingConnect.Timer = timer;

				ping_Send();
			}
		}
	}
} /* End of PING_EchoCheck() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_ReplyCheck
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_ReplyCheck(void)
{
	ICMP_HEADER*	pIcmpHeader;
	IP_HEADER*		pIpHeader;
	U8_T*			pbuf;

	pIcmpHeader = (ICMP_HEADER*)&uip_buf[ICMP_HEADER_OFFSET];
	pIpHeader = (IP_HEADER*)&uip_buf[IP_HEADER_OFFSET];

	if ((pIpHeader->SourIp == PingConnect.IP) &&
		(*(U16_T XDATA*)pIcmpHeader->Sequence == PingConnect.Sequence))
	{
		if (PingConnect.Length != (pIpHeader->Length - 28))
			return;

		PingConnect.Wait = 0;
		PingConnect.ReplyGood++;
		pbuf = (U8_T*)&PingConnect.IP;
#if (UART0_ENABLE)
		printd ("\rReply from %bu.%bu.%bu.%bu: bytes = %u\n\r",
			pbuf[0], pbuf[1], pbuf[2], pbuf[3], PingConnect.Length);
#endif
	}
} /* End of PING_ReplyCheck() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: ping_Send
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void ping_Send(void)
{
	U8_T*			pDest;
	ICMP_HEADER*	pIcmpHeader;
	IP_HEADER*		pIpHeader;
	U16_T			loop;
	U8_T			index;

	/* icmp protocol*/
	pIcmpHeader = (ICMP_HEADER*)&uip_buf[ICMP_HEADER_OFFSET];
	pIcmpHeader->Type = PING_ECHO_REQUEST;
	pIcmpHeader->Code = 0;
	pIcmpHeader->CheckSum = 0;
	pIcmpHeader->Id_Data = 0x0200;

	pingSequence[0]++;
	if (pingSequence[0] == 0)
		pingSequence[1]++;
	pIcmpHeader->Sequence[0] = pingSequence[0];
	pIcmpHeader->Sequence[1] = pingSequence[1];

	PingConnect.Sequence = *(U16_T*)pingSequence;

	/* ping padload*/
	pDest = &pIcmpHeader->Data;
	loop = PingConnect.Length;
	uip_len = loop + ICMP_HEADER_OFFSET + 8;
	index = 0;

	while (loop--)
	{
		*pDest++ = index + 0x61;
		if (++index == 23)
			index = 0;
	}

	/* ip protocol */
	pIpHeader = (IP_HEADER*)&uip_buf[IP_HEADER_OFFSET];
	pIpHeader->VerHlen = 0x45;
	pIpHeader->TOS = 0;
#if (STOE_TRANSPARENT)
	pIpHeader->Length = uip_len -14;
#else
	pIpHeader->Length = uip_len;
#endif
	pIpHeader->ID = ip_Id++;
	pIpHeader->Frags = 0;
	pIpHeader->TTL = UIP_TTL;
	pIpHeader->Protocol = UIP_PROTO_ICMP;
	pIpHeader->SourIp = PNetStation->StationIP;
	pIpHeader->DestIp = PingConnect.IP;
	pIpHeader->CheckSum = 0;

	PayLoadOffset = uip_len;	/* all frame in continuous Data Memory */

	/* Send packet */
#if STOE_TRANSPARENT
	uip_arp_out();
#endif
	ETH_Send();

} /* End of ping_Send() */

#endif /* INCLUDE_PING_REQUEST */

#if (!STOE_TRANSPARENT)
/*
 * ----------------------------------------------------------------------------
 * Function Name: PING_Test
 * Purpose:
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void PING_Test(U32_T ip)
{
	ICMP_HEADER*	pIcmpHeader;
	IP_HEADER*		pIpHeader;

	/* icmp protocol*/
	pIcmpHeader = (ICMP_HEADER*)&uip_buf[ICMP_HEADER_OFFSET];
	pIcmpHeader->Type = PING_ECHO_REQUEST;
	pIcmpHeader->Code = 0;
	pIcmpHeader->CheckSum = 0;
	pIcmpHeader->Id_Data = 0x0200;

	pIcmpHeader->Sequence[0] = 0xff;
	pIcmpHeader->Sequence[1] = 0xff;

	/* ping padload*/
	pIcmpHeader->Data = 0x61;

	uip_len = ICMP_HEADER_OFFSET + 9;

	/* ip protocol */
	pIpHeader = (IP_HEADER*)&uip_buf[IP_HEADER_OFFSET];
	pIpHeader->VerHlen = 0x45;
	pIpHeader->TOS = 0;
	pIpHeader->Length = uip_len;
	pIpHeader->ID = 0xffff;
	pIpHeader->Frags = 0;
	pIpHeader->TTL = UIP_TTL;
	pIpHeader->Protocol = UIP_PROTO_ICMP;
	pIpHeader->SourIp = PNetStation->StationIP;
	pIpHeader->DestIp = ip;
	pIpHeader->CheckSum = 0;

	PayLoadOffset = uip_len; /* all frame in continuous Data Memory */

	/* Send packet */
#if STOE_TRANSPARENT
	uip_arp_out();
#endif
	ETH_Send();

} /* End of PING_Test() */
#endif /* !STOE_TRANSPARENT */


/* End of ping.c */