//
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
// Module Name: mac.c
// Purpose:  
// Author:
// Date:
// Notes:
// $Log: mac.c,v $
// Revision 1.1.1.1  2006/06/20 05:50:28  borbin
// no message
//
//=============================================================================
//

// INCLUDE FILE DECLARATIONS //
#include "reg80390.h"
#include "mac.h"
#include "stoe.h"
#include "delay.h"
#include <intrins.h>
#include "printd.h"

// GLOBAL VARIABLES DECLARATIONS //
static byte XDATA mac_InterruptStatus = 0;

// LOCAL SUBPROGRAM DECLARATIONS //
static void mac_InterruptEnable(void);
static void mac_InterruptDisable(void);
static void mac_StartOperate(void);
static void mac_StopOperate(void);

//
// ----------------------------------------------------------------------------
// Function Name: mac_ReadReg
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
//
void mac_ReadReg(byte regaddr, byte XDATA* pbuf, byte length)
{
	byte	isr;

	isr = EA;
	EA = 0;
	_nop_();
	MCIR = regaddr;
	while (length--)
		pbuf[length] = MDR;
	EA = isr;

} // End of mac_ReadReg //

//
// ----------------------------------------------------------------------------
// Function Name: mac_WriteReg
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void mac_WriteReg(byte regaddr, byte XDATA* pbuf, byte length)
{
	byte	isr;

	isr = EA;
	EA = 0;
	while (length--)
		MDR = pbuf[length];
	MCIR = regaddr;
	EA = isr;
} // End of mac_WriteReg //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_Init
// Purpose: initial all registers and variables of MAC.
// Params: network_type -0: auto- negotiation
// 				-1: fixed 100 full speed.
// 				-2: fixed 100 half speed.
// 				-3: fixed 10 full speed.
// 				-4: fixed 10 half speed.
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_Init(byte network_type)
{
	byte XDATA	temp[3];

	// read MAC address//
	mac_ReadReg(MAC_ADDR_REG, PNetStation->CurrStaAddr, MAC_ADDRESS_LEN);
	// use embedded phy //
	temp[0] = (PHY_SELECT_EMBEDDED | PHY_INTERNAL_PHY_OPERA_STATE | BIT1);
	mac_WriteReg(MAC_PHY_CTL_REG, temp, 1);
	// phy power up //
	PHY_PowerUp();
	// reset embedded phy //
	temp[0] = (PHY_SELECT_EMBEDDED | BIT1);
	mac_WriteReg(MAC_PHY_CTL_REG, temp, 1);
	DELAY_Ms(2);
	// set embedded phy to operating mode //
	temp[0] = (PHY_SELECT_EMBEDDED | PHY_INTERNAL_PHY_OPERA_STATE | BIT1);
	mac_WriteReg(MAC_PHY_CTL_REG, temp, 1);

	// stop & reset MAC operation //
	mac_StopOperate();

	if (network_type != FORCE_EMBEDDED_PHY_10_HALF)
	{
		// phy initialize. //
		PHY_Init(network_type);
	}
	PBDP->MacInfo.MediumLinkType = MEDIUM_ENABLE_TX_FLOWCTRL;
	PBDP->MacInfo.FullDuplex = 1;
	switch (network_type)
	{
	default:
	case AUTO_NEGOTIATION:
		PBDP->MacInfo.NetworkType = MAC_AUTO_NEGOTIATION;
		PBDP->MacInfo.MediumLinkType|=(MEDIUM_FULL_DUPLEX_MODE | MEDIUM_MII_100M_MODE |
			MEDIUM_ENABLE_RX_FLOWCTRL);
		break;
	case FIXED_100_FULL:
		PBDP->MacInfo.NetworkType = (MAC_LINK_100M_SPEED | MAC_LINK_FULL_DUPLEX);
		PBDP->MacInfo.MediumLinkType|=(MEDIUM_FULL_DUPLEX_MODE | MEDIUM_MII_100M_MODE |
			MEDIUM_ENABLE_RX_FLOWCTRL);
		break;
	case FIXED_100_HALF:
		PBDP->MacInfo.NetworkType = MAC_LINK_100M_SPEED;
		PBDP->MacInfo.MediumLinkType |= MEDIUM_MII_100M_MODE;
		break;
	case FIXED_10_FULL:
		PBDP->MacInfo.NetworkType = (MAC_LINK_10M_SPEED | MAC_LINK_FULL_DUPLEX);
		PBDP->MacInfo.MediumLinkType |= (MEDIUM_FULL_DUPLEX_MODE |
			MEDIUM_ENABLE_RX_FLOWCTRL);
		break;
	case FIXED_10_HALF:
		PBDP->MacInfo.NetworkType = MAC_LINK_10M_SPEED;
		break;
	case FORCE_EMBEDDED_PHY_10_HALF:
		PBDP->MacInfo.NetworkType = MAC_FORCE_ENBEDDED_PHY_10_HALF;
		PBDP->MacInfo.MediumLinkType = 0;
		break;
	}
	// set medium status //
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);
	// set IPG //
	temp[0] = 0x2b;
	temp[1] = 0x16;
	temp[2] = 0x95;
	mac_WriteReg(MAC_IPG_CTL_REG, temp, 3);
	// set RX filter. //
	temp[0] = BIT6;
	mac_WriteReg(MAC_RX_CTL_REG, temp, 1);
	MAC_SetRxFilter(MAC_RCV_BROADCAST);
	PBDP->MacInfo.LinkSpeed = 0; // Ethernet not linkup.
	// decide interrupt mask //
	PBDP->MacInfo.InterruptMask = PRIMARY_LINK_CHANGE_ENABLE;
} // End of MAC_Init() //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_GetMacAddr
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
byte* MAC_GetMacAddr(void)
{
	return PNetStation->CurrStaAddr;

} // End of MAC_GetMacAddr() //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_GetMacAddr
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_SetMacAddr(byte* addr)
{
	byte i;

	for (i = 0; i < MAC_ADDRESS_LEN; i++)
		PNetStation->CurrStaAddr[i] = addr[i];
	mac_WriteReg(MAC_ADDR_REG, PNetStation->CurrStaAddr, MAC_ADDRESS_LEN);
}

//
// ----------------------------------------------------------------------------
// Function Name: MAC_Start
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_Start(void)
{
	// clear mac interrupt status //
	mac_ReadReg(MAC_WAKEUP_LINK_INT_STATUS_REG, &mac_InterruptStatus, 1);
	// enable mac interrupt //
	mac_InterruptEnable();
} // End of MAC_Start() //

//
// ----------------------------------------------------------------------------
// Function Name: mac_StartOperate
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void mac_StartOperate(void)
{
	byte XDATA	temp;

	mac_ReadReg(MAC_RX_CTL_REG, &temp, 1);

	// set mac register to start receive/transmit packets. //
	temp |= START_OPERATION;
	mac_WriteReg(MAC_RX_CTL_REG, &temp, 1);

} // End of mac_StartOperate //

//
// ----------------------------------------------------------------------------
// Function Name: mac_StopOperate
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void mac_StopOperate(void)
{
	byte XDATA	temp;

	mac_ReadReg(MAC_RX_CTL_REG, &temp, 1);

	temp &= ~START_OPERATION;
	mac_WriteReg(MAC_RX_CTL_REG, &temp, 1);

} // End of mac_StopOperate //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_SetRxFilter
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_SetRxFilter(byte filter)
{
	byte XDATA	temp;

	mac_ReadReg(MAC_RX_CTL_REG, &temp, 1);

	// set rcv filter. //
	temp &= 0xc0;
	if (filter & MAC_RCV_PROMISCUOUS)
		temp |= PACKET_TYPE_PROMISCOUS;
	if (filter & MAC_RCV_ALL_MULTICAST)
		temp |= PACKET_TYPE_ALL_MULTI;
	if (filter & MAC_RCV_BROADCAST)
		temp |= PACKET_TYPE_BROADCAST;
	if (filter & MAC_RCV_MULTICAST)
		temp |= PACKET_TYPE_MULTICAST;

	mac_WriteReg(MAC_RX_CTL_REG, &temp, 1);

} // End of MAC_SetRxFilter() //

//
// ----------------------------------------------------------------------------
// Function Name: mac_InterruptEnable
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void mac_InterruptEnable(void)
{
	byte XDATA	temp;

	// set link change interrupt enable //
	temp = PBDP->MacInfo.InterruptMask;
	mac_WriteReg(MAC_LINK_CHANGE_INT_ENABLE_REG, &temp, 1);
} // End of mac_InterruptEnable //

//
// ----------------------------------------------------------------------------
// Function Name: mac_InterruptDisable
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void mac_InterruptDisable(void)
{
	byte XDATA	temp;

	temp = 0;
	mac_WriteReg(MAC_LINK_CHANGE_INT_ENABLE_REG, &temp, 1);

} // End of mac_InterruptDisable //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_SetInterruptFlag
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_SetInterruptFlag(void)
{
	mac_ReadReg(MAC_WAKEUP_LINK_INT_STATUS_REG, &mac_InterruptStatus, 1);
	if (!(mac_InterruptStatus & STATUS_PRIMARY_LINK_CHANGE))
		mac_InterruptStatus = 0;

} // End of MAC_SetInterruptFlag() //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_GetInterruptFlag
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
byte MAC_GetInterruptFlag(void)
{
	byte flag, isr;

	isr = EA;
	EA = 0;
	flag = mac_InterruptStatus;
	EA = isr;

	return flag;

} // End of MAC_GetInterruptFlag() //

//
// ----------------------------------------------------------------------------
// Function Name: mac_GetMediaType
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
byte mac_GetMediaType(void)
{
	word	phylinkstatus;

	phylinkstatus = PHY_CheckMediaType();

	PBDP->MacInfo.FullDuplex = TRUE;

	PBDP->MacInfo.MediumLinkType = (MEDIUM_ENABLE_TX_FLOWCTRL | MEDIUM_ENABLE_RECEIVE);

	// Determine if we're linked to 100 full duplex. //
	if (phylinkstatus & ANLPAR_100TXFD)
	{
		PBDP->MacInfo.LinkSpeed = 100;
		PBDP->MacInfo.MediumLinkType |=(MEDIUM_FULL_DUPLEX_MODE |MEDIUM_MII_100M_MODE);
//		printd ("Link to 100 FULL.\n\r");
	}
	// Determine if we're linked to 100 half duplex. //
	else if (phylinkstatus & ANLPAR_100TX)
	{
		PBDP->MacInfo.LinkSpeed = 100;
		PBDP->MacInfo.MediumLinkType |= MEDIUM_MII_100M_MODE;
		PBDP->MacInfo.FullDuplex = FALSE;
//		printd ("Link to 100 HALF.\n\r");
	} 
	// Determine if we're linked to 10 full duplex. //
	else if (phylinkstatus & ANLPAR_10TFD)
	{
		PBDP->MacInfo.LinkSpeed = 10;
		PBDP->MacInfo.MediumLinkType |= MEDIUM_FULL_DUPLEX_MODE;
//		printd ("Link to 10 FULL.\n\r");
	}
	// we're linked to 10 half duplex. //
	else
	{
		PBDP->MacInfo.LinkSpeed = 10;
		PBDP->MacInfo.FullDuplex = FALSE;
//		printd ("Link to 10 HALF.\n\r");
	}

	if (PBDP->MacInfo.FullDuplex)
		PBDP->MacInfo.MediumLinkType |= MEDIUM_ENABLE_RX_FLOWCTRL;
	mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);

	return PBDP->MacInfo.LinkSpeed;

} // End of mac_GetMediaType //

//
// ----------------------------------------------------------------------------
// Function Name : mac_LinkSpeedChk
// Purpose       :
// Params        : none
// Returns       : 100 : Ethernet is link to 100M
//                  10 : Ethernet is link to 10M
//                   0 : not link
// Note:
// ----------------------------------------------------------------------------
 //
byte mac_LinkSpeedChk(void)
{
	if (mac_InterruptStatus & STATUS_PRIMARY_IS_LINK_UP)
	{
		mac_InterruptStatus = 0;
		if (PBDP->MacInfo.NetworkType != MAC_FORCE_ENBEDDED_PHY_10_HALF)
		{
			mac_GetMediaType();
		}
		else
		{
			PBDP->MacInfo.FullDuplex = 0;
			PBDP->MacInfo.LinkSpeed = 10;

			PBDP->MacInfo.MediumLinkType = MEDIUM_ENABLE_RECEIVE;
			mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);
		}

		// start operation //
		mac_StartOperate();
		DELAY_Ms(0x30);
	}
	else
	{
		mac_InterruptStatus = 0;		
		if (PBDP->MacInfo.LinkSpeed)
		{
			PBDP->MacInfo.LinkSpeed = 0;

			// disable receive //
			PBDP->MacInfo.MediumLinkType &= ~MEDIUM_ENABLE_RECEIVE;
			mac_WriteReg(MAC_MEDIUM_STATUS_MODE_REG, &PBDP->MacInfo.MediumLinkType, 1);

			STOE_WaitHwReleaseRcvBuf();

			// stop operation and reset Ethernet MAC packet buffer //
			mac_StopOperate();

			if (PBDP->MacInfo.NetworkType != MAC_FORCE_ENBEDDED_PHY_10_HALF)
			{
				PHY_WatchDog();
			}
		}
	}

	return PBDP->MacInfo.LinkSpeed;

} // End of mac_LinkSpeedChk //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_ProcessInterrupt
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_ProcessInterrupt(void)
{
	mac_InterruptDisable();

	if (mac_InterruptStatus & STATUS_PRIMARY_LINK_CHANGE)
		mac_LinkSpeedChk();

	mac_InterruptEnable();

} // End of MAC_ProcessInterrupt //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_ProcessInterruptExt
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
byte MAC_ProcessInterruptExt(void)
{
	byte speed = 0;

	if (mac_InterruptStatus & STATUS_PRIMARY_LINK_CHANGE)
		speed = mac_LinkSpeedChk();

	mac_InterruptStatus = 0;

	return speed;
} // End of MAC_ProcessInterruptExt //

//
//-----------------------------------------------------------------------------
// Function Name: mac_ComputeCrc32
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
ulong mac_ComputeCrc32(word length, byte* pbuf)
{
	ulong	crc32 = 0xffffffff;
	byte	curByte, carry, j;
	
	for (; length; length--)
	{
		curByte = *pbuf++;
		for (j=0; j<8; j++)
		{
			carry = curByte & 1;
			if ( crc32 & 0x80000000 )
				carry ^= 1;
			crc32 <<= 1;
			curByte >>= 1;
			if (carry)
				crc32 ^= 0x04c11db7;
		}
	}
	return crc32;

} // End of mac_ComputeCrc32() //

//
// ----------------------------------------------------------------------------
// Function Name: MAC_MultiFilter
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void MAC_MultiFilter(byte* pbuf, byte len)
{
	byte	count = len/MAC_ADDRESS_LEN;
	byte	index, bitNum;
	byte 	filter[8] = {0};

	for (index = 0; index < count; index++)
	{
		if (!(pbuf[0] & 1))
			break;

		bitNum = (byte)((mac_ComputeCrc32(MAC_ADDRESS_LEN, pbuf)>>26)&0x3f);
		pbuf += MAC_ADDRESS_LEN;
		filter[7 - (bitNum/8)] |= (1 << (bitNum % 8));
	}

	mac_WriteReg(MAC_MULTICASE_REG, filter, 8);

} // End of MAC_MultiFilter() //

// End of mac.c //