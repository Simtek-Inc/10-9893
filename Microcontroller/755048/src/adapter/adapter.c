// ============================================================================
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
// ============================================================================
//
// ============================================================================
// Module Name: adapter.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: adapter.c,v $
// ============================================================================

// INCLUDE FILE DECLARATIONS //
#include "adapter.h"
#include "stoe.h"
#include "dma.h"
#include "tcpip.h"
#include "pppoe.h"
#include "mstimer.h"
#include "dhcpc.h"

// NAMING CONSTANT DECLARATIONS //
#define ETH_HEADER_LEN		14
#define ETH_NON_TRANSPARENT	0
#define ETH_TRANSPARENT		1
#define ETH_ARP_SEND_INTERVAL_TIME	180

// GLOBAL VARIABLES DECLARATIONS //
//byte ETH_IsPppoe = 0;
byte ETH_DoDhcp = 0;
//byte ETH_PppoeHeaderLen = 0;
byte ETH_DhcpArpReply = TRUE;

// LOCAL VARIABLES DECLARATIONS //
static byte  eth_EthHeadLen = 0;
static word  eth_PayLoadOffset = 0;
static ulong eth_ArpSendIntervalTime = 3;
static ulong eth_ArpSendTime = 0;

// LOCAL SUBPROGRAM DECLARATIONS //
static void eth_RcvHandle(byte XDATA*, word , byte);

//
// ----------------------------------------------------------------------------
// Function Name: ETH_Init
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_Init(void)
{
	byte*		point;

	// ethernet initiation //
	STOE_Init(ETH_PHY_SENSE_TYPE);
	STOE_RcvCallback = eth_RcvHandle;

	// Initialise TCP/UDP up-layer interfaces //
	TCPIP_Init(ETH_TRANSPARENT);
	eth_EthHeadLen = ETH_HEADER_LEN;
	// Initialise the ARP cache. //
	point =	MAC_GetMacAddr();
	TCPIP_SetMacAddr(point);

	TCPIP_AssignLowlayerXmitFunc(ETH_Send);
	// PPPoE initiation //
//	PPPOE_Init();
} // End of ETH_Init //

//
// ----------------------------------------------------------------------------
// Function Name: ETH_Start
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_Start(void)
{
	STOE_Start();

} // End of ETH_Start //

//
// ----------------------------------------------------------------------------
// Function Name: eth_RcvHandle
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void eth_RcvHandle(byte XDATA* pbuf, word  length, byte protocol)
{
	// copy packet into uIP buffer //
	DMA_GrantXdata(TCPIP_GetRcvBuffer(), pbuf, length);

	TCPIP_SetRcvLength(length);

	switch (protocol)
	{
	default:
		break;
	case 1:
	case 2:
	case 6:
	case 17:
		if (ETH_DoDhcp == 0)
			uip_arp_ipin();
		TCPIP_Receive();
		if(TCPIP_GetXmitLength() > 0)
		{
			//if (!ETH_IsPppoe)
				uip_arp_out();
			ETH_Send(0);
		}
		break;
	case 0xff:
			if(pbuf[12] == 0x08 && pbuf[13] == 0x06)
			{
				uip_arp_arpin();
				if ((pbuf[21] == 0x02) && ((*(ulong *)&pbuf[28]) == STOE_GetIPAddr()))
				{
					if (ETH_CheckDhcpArpReply() == FALSE)
						ETH_SetDhcpArpReplyFlag(TRUE);
				}
				else
				{
	        	    if(TCPIP_GetXmitLength() > 0)
						ETH_Send(0);

					if (TCPIP_GetGwMacAddrFlag())
						eth_ArpSendIntervalTime = ETH_ARP_SEND_INTERVAL_TIME;
				}
			}

		break;
	}  // End of switch //

} // End of eth_RcvHandle //

//
// ----------------------------------------------------------------------------
// Function Name: ETH_Send
// Purpose: Copy Data to Mac ram and send to Ethernet.
// Params: 
// Returns: none
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_Send(word payloadOffset)
{
	word 	length = TCPIP_GetXmitLength();

	eth_PayLoadOffset = payloadOffset;
	ETH_SendPkt(TCPIP_GetXmitBuffer(), length);
} // End of ETH_Send() //

//
// ----------------------------------------------------------------------------
// Function Name: ETH_SendPkt
// Purpose: Copy Data to Mac ram and send to Ethernet.
// Params: none
// Returns: none
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_SendPkt(byte XDATA* psour, word  length)
{
	byte XDATA* pBuf;
	byte XDATA* point;
	word 		len;
	word 		addr;
	byte		isPppoeIp = 0;
	word 		totalLen = length;
	ulong	sourIp = STOE_GetIPAddr();
	ulong	destIp = *(ulong*)&psour[16];
	byte	mode = 0;

	if (!PBDP->MacInfo.LinkSpeed)
		return;

	pBuf = STOE_AssignSendBuf(length + MAC_TX_RX_HEADER);
	if (!pBuf)
		return;
	point = pBuf + MAC_TX_RX_HEADER;

	{
		if ((*(psour + 12) == 8) && (*(psour + 13) == 0))
			PBDP->STOE_TxInform.Protocol = *(psour + 23);
		else
			PBDP->STOE_TxInform.Protocol = 0xff;
	}

	if (pBuf)
	{
		if (eth_PayLoadOffset)
			len = eth_PayLoadOffset;
		else
		{
				len = 40 + eth_EthHeadLen;
		}

		if (length > len)
		{
			// Do software DMA //
			DMA_GrantXdata(point, TCPIP_GetXmitBuffer(), len);

			point += len;
			length -= len;

			addr = (ulong)(TCPIP_GetPayloadBuffer()) >> 8;

			if(addr > 0x8000)	// if(flag_dataincode) //
				STOE_CopyCode2TPBR(point, TCPIP_GetPayloadBuffer(), length);
			else if (addr >= 0x200)
				DMA_Grant(point, TCPIP_GetPayloadBuffer(), length);
			else
				DMA_GrantXdata(point, TCPIP_GetPayloadBuffer(), length);
		}
		else
		{
			// Do software DMA //
			DMA_GrantXdata(point, TCPIP_GetXmitBuffer(), len);

		} // End of if (length > len) //

		if (isPppoeIp)
			pBuf[5] = 0x80;
		else
			pBuf[5] = 0;
		STOE_Send(pBuf, totalLen, PBDP->STOE_TxInform.Protocol);
	}

	eth_PayLoadOffset = 0; // clear offset flag //

} // End of ETH_SendPkt() //

//
// ----------------------------------------------------------------------------
// Function Name: eth_CheckGatewayIp
// Purpose: Copy Data to Mac ram and send to Ethernet.
// Params: none
// Returns: none
// Note:
// ----------------------------------------------------------------------------
 //
bool eth_CheckGatewayIp(byte XDATA* pBuf)
{
	ulong	gatewayIp, receiveIp;

	gatewayIp = STOE_GetGateway();
	if (!gatewayIp)
	{
		return FALSE;
	}

	receiveIp = *(ulong *)&pBuf[28];
	if (gatewayIp != receiveIp)
	{
		return FALSE;
	}

	return TRUE;
}

//
// ----------------------------------------------------------------------------
// Function Name: ETH_SendArpToGateway
// Purpose: send arp request to gateway
// Params: none
// Returns: none
// Note:
// ----------------------------------------------------------------------------
void ETH_SendArpToGateway(bool StopSendAfterRcvReply)
{
	ulong	gateway, ip, TempTime;
	byte	*buf = uip_buf;
	byte*	point;

	if (StopSendAfterRcvReply)
	{
		if (TCPIP_GetGwMacAddrFlag())
			return;
	}

	// first check stoe gateway setting
	gateway = STOE_GetGateway();
	if (!gateway)
		return;

	// check stoe source ip setting
	ip = STOE_GetIPAddr();
	if (!ip)
		return;

	// check is it time to send arp request
	TempTime = SWTIMER_Tick();
	if ((TempTime - eth_ArpSendTime) < (eth_ArpSendIntervalTime * SWTIMER_COUNT_SECOND))
		return;

	// update time
	eth_ArpSendTime = TempTime;

	if (TCPIP_GetGwMacAddrFlag())
	{
		eth_ArpSendIntervalTime = ETH_ARP_SEND_INTERVAL_TIME;
	}
	else
	{
		eth_ArpSendIntervalTime = 10;
	}

	// send arp request to gateway,
	// in order to get gateway MAC address.
	point = MAC_GetMacAddr();
	uip_len = 42;

	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = *(point);
	buf[7] = *(point + 1);
	buf[8] = *(point + 2);
	buf[9] = *(point + 3);
	buf[10] = *(point + 4);
	buf[11] = *(point + 5);
	buf[12] = 0x08; 
	buf[13] = 0x06;

	buf[14] = 0x00;
	buf[15] = 0x01;
	buf[16] = 0x08;
	buf[17] = 0x00;
	buf[18] = 0x06;
	buf[19] = 0x04;
	buf[20] = 0x00;
	buf[21] = 0x01;

	buf[22] = *(point);
	buf[23] = *(point + 1);
	buf[24] = *(point + 2);
	buf[25] = *(point + 3);
	buf[26] = *(point + 4);
	buf[27] = *(point + 5);
	*(ulong *)&buf[28] = STOE_GetIPAddr();
	
	buf[32] = 0x00;
	buf[33] = 0x00;
	buf[34] = 0x00;
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	*(ulong *)&buf[38] = gateway;

	ETH_Send(0);

	return;
}

//
// ----------------------------------------------------------------------------
// Function Name: ETH_SendArpRequest
// Purpose: send arp request to dstIp
// Params: none
// Returns: none
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_SendArpRequest(ulong dstIp)
{
	byte *buf = uip_buf;
	byte *point;

    // in order to get gateway MAC address.
	point = MAC_GetMacAddr();
	uip_len = 42;

	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = *(point);
	buf[7] = *(point + 1);
	buf[8] = *(point + 2);
	buf[9] = *(point + 3);
	buf[10] = *(point + 4);
	buf[11] = *(point + 5);
	buf[12] = 0x08; 
	buf[13] = 0x06;

	buf[14] = 0x00;
	buf[15] = 0x01;
	buf[16] = 0x08;
	buf[17] = 0x00;
	buf[18] = 0x06;
	buf[19] = 0x04;
	buf[20] = 0x00;
	buf[21] = 0x01;

	buf[22] = *(point);
	buf[23] = *(point + 1);
	buf[24] = *(point + 2);
	buf[25] = *(point + 3);
	buf[26] = *(point + 4);
	buf[27] = *(point + 5);
	*(ulong *)&buf[28] = STOE_GetIPAddr();
	
	buf[32] = 0x00;
	buf[33] = 0x00;
	buf[34] = 0x00;
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	*(ulong *)&buf[38] = dstIp;

	ETH_Send(0);

	return;
}

//
// ----------------------------------------------------------------------------
// Function Name: ETH_RestartSendArpToGateway
// Purpose: Restart sending arp request to gateway
// Params: none
// Returns: none
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_RestartSendArpToGateway(void)
{
	TCPIP_SetGwMacAddrFlag(FALSE);
	eth_ArpSendIntervalTime = 3;
}

//
// ----------------------------------------------------------------------------
// Function Name: ETH_SendDhcpArpRequest
// Purpose: send arp request to gateway
// Params: none
// Returns: none
// Note:
// ----------------------------------------------------------------------------
 //
void ETH_SendDhcpArpRequest(void)
{
	byte	*buf = uip_buf;
	byte*	point;

	eth_ArpSendIntervalTime = 5;

	point = MAC_GetMacAddr();
	uip_len = 42;

	buf[0] = 0xFF;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;
	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = *(point);
	buf[7] = *(point + 1);
	buf[8] = *(point + 2);
	buf[9] = *(point + 3);
	buf[10] = *(point + 4);
	buf[11] = *(point + 5);
	buf[12] = 0x08; 
	buf[13] = 0x06;

	buf[14] = 0x00;
	buf[15] = 0x01;
	buf[16] = 0x08;
	buf[17] = 0x00;
	buf[18] = 0x06;
	buf[19] = 0x04;
	buf[20] = 0x00;
	buf[21] = 0x01;

	buf[22] = *(point);
	buf[23] = *(point + 1);
	buf[24] = *(point + 2);
	buf[25] = *(point + 3);
	buf[26] = *(point + 4);
	buf[27] = *(point + 5);
	*(ulong *)&buf[28] = STOE_GetIPAddr(); // Source IP address
	
	buf[32] = 0x00;
	buf[33] = 0x00;
	buf[34] = 0x00;
	buf[35] = 0x00;
	buf[36] = 0x00;
	buf[37] = 0x00;
	*(ulong *)&buf[38] = STOE_GetIPAddr(); // Destination IP address

	ETH_DhcpArpReply = FALSE;

	ETH_Send(0);

	return;
}

// End of adapter.c //