//----------------------------------------------------------------------------------------
//   Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
// 
//   This is unpublished proprietary source code of ASIX Electronic Corporation
// 
//   The copyright notice above does not evidence any actual or intended
//   publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name: stoe.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: stoe.c,v $
//----------------------------------------------------------------------------------------
// Revision 1.1.1.1  2006/06/20 05:50:28  borbin
// no message
//----------------------------------------------------------------------------------------

// INCLUDE FILE DECLARATIONS
#include "reg80390.h"
#include "stoe.h"
#include "dma.h"
#include "delay.h"
#include "interrupt.h"

#include "adapter.h"

#include <intrins.h>

#define XSTACKSIZE 64

// GLOBAL VARIABLES DECLARATIONS
LOCAL_STATION XDATA* PNetStation = {0};
BUF_DESC_PAGE XDATA* PBDP = 0;
void (*STOE_RcvCallback)(byte XDATA*, word, byte); // call eth layer rcv process function.

// LOCAL VARIABLES DECLARATIONS
static byte XDATA stoe_BufferSize[STOE_SOCKET_BUFFER_SIZE] _at_ STOE_BDP_START_ADDR;
static byte XDATA stoe_InterruptStatus = 0;

// LOCAL SUBPROGRAM DECLARATIONS
static void stoe_BdpInit(void);
static void stoe_StartOperate(void);
static void stoe_RcvHandle(void);
//sr 3/5/12
void stoe_StopOperate(void);

// Reserved xstack for the functions with reenterence in bootloader and runtime code will use those functions
//byte XDATA xStackBuf[XSTACKSIZE] _at_ (STOE_BDP_START_ADDR-(4*PAGE_SIZE)- XSTACKSIZE);
byte XDATA xStackBuf[XSTACKSIZE] _at_ (0x5700-(4*PAGE_SIZE)- XSTACKSIZE);

// ----------------------------------------------------------------------------
// Function Name: stoe_ReadReg
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void stoe_ReadReg(byte regaddr, byte XDATA* pbuf, byte length)
{
	byte	isr;

	isr = EA;
	EA = 0;
	_nop_();
	TCIR = regaddr;
	while (length--)
	  pbuf[length] = TDR;
	EA = isr;

} // End of stoe_ReadReg //

// ----------------------------------------------------------------------------
// Function Name: stoe_WriteReg
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void stoe_WriteReg(byte regaddr, byte XDATA* pbuf, byte length)
{
	byte	isr;

	isr = EA;
	EA = 0;
	while (length--)
		TDR = pbuf[length];
	TCIR = regaddr;
	EA = isr;

} // End of stoe_WriteReg

// ----------------------------------------------------------------------------
// Function Name: STOE_Init
// Purpose: to initial all registers and variables of STOE. 
// Params: network_type -0: auto- negotiation
//						-1: fixed 100 full speed.
//						-2: fixed 100 half speed.
//						-3: fixed 10 full speed.
//						-4: fixed 10 half speed.
// Returns: none
// Note:
// ----------------------------------------------------------------------------
void STOE_Init(byte network_type)
{
	byte XDATA	temp[2];
	byte XDATA	value[6] = {0,0,0,0,0,0};

	// set xmit & rcv memory.
	stoe_BdpInit();

	PNetStation = &PBDP->NetStation;

	// source IP
	stoe_ReadReg(STOE_IP_ADDR_REG, (byte XDATA*)&PNetStation->DefaultIP, 4);
	PNetStation->StationIP = PNetStation->DefaultIP;

	// subnet mask
	stoe_ReadReg(STOE_SUBNET_MASK_REG, (byte XDATA*)&PNetStation->DefaultMask, 4);
	PNetStation->SubnetMask = PNetStation->DefaultMask;

	// gateway
	PNetStation->DefaultGateway = ((PNetStation->StationIP & PNetStation->SubnetMask) | 1);
	if (PNetStation->StationIP == PNetStation->DefaultGateway)
	  {
	  PNetStation->DefaultGateway |= (~PNetStation->SubnetMask);
	  PNetStation->DefaultGateway &= 0xfffffffe;
	  }

	// set L2 control register
	temp[0] = (RX_TRANSPARENT | TX_TRANSPARENT);

	stoe_WriteReg(STOE_L2_CTL_REG, temp, 1);

	// set ARP table timeout register
	temp[0] = STOE_ARP_TIMEOUT;
	stoe_WriteReg(STOE_ARP_TIMEOUT_REG, temp, 1);

	// set L4 control register
	temp[0] = (DROP_CHKSUMERR_PKT | ENABLE_XMIT_CHKSUM | ENABLE_XMIT_CROSS);

	stoe_WriteReg(STOE_L4_CTL_REG, temp, 1);

	// set BDP point
	temp[0] = (STOE_BDP_START_ADDR >> 16);
	temp[1] = (STOE_BDP_START_ADDR >> 8);
	stoe_WriteReg(STOE_L4_BDP_PNT_REG, temp, 2);

	// receive callback function initialize to null point.
	STOE_RcvCallback = 0;

	DMA_Init();
	DELAY_Init();
	MAC_Init(network_type);

	STOE_SetGateway(PNetStation->DefaultGateway);

} // End of STOE_Init

//-----------------------------------------------------------------------------
// Function Name: stoe_BdpInit
// Purpose: initial Buffer Descriptor Page.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void stoe_BdpInit(void)
{
	word	page = (STOE_BDP_START_ADDR >> 8);

	PBDP = STOE_BDP_START_ADDR;

	// set BDP number
	PBDP->BDP_ID = 0;

	// set rcv buffer.
	PBDP->RSPP = page + 1;
	PBDP->REPP = page + PAGES_OF_RCV;
	PBDP->RHPR = PBDP->RSPP;
	PBDP->RTPR = PBDP->RSPP;
	PBDP->RFP = PAGES_OF_RCV;

	// set xmit buffer.
	PBDP->TSPP = page + 1 + PAGES_OF_RCV;
	PBDP->TEPP = page + PAGES_OF_RCV + PAGES_OF_XMIT;
	PBDP->THPR = PBDP->TSPP;
	PBDP->TTPR = PBDP->TSPP;
	PBDP->TFP = PAGES_OF_XMIT;

	// set rcv and xmit start/end buffer address.
	PBDP->RcvStartAddr = STOE_BDP_START_ADDR + 256;
	PBDP->XmtStartAddr = PBDP->RcvStartAddr + STOE_RCV_BUF_SIZE;
	PBDP->RcvEndAddr = PBDP->XmtStartAddr - 1;
	PBDP->XmtEndAddr = PBDP->XmtStartAddr + STOE_XMIT_BUF_SIZE - 1;

} // End of stoe_BdpInit

//#if (STOE_GET_IPADDR_TYPE == STOE_INITIAL_ASSIGN_IP)

// ----------------------------------------------------------------------------
// Function Name: STOE_SetIPAddr
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_SetIPAddr(ulong ip)
{
	if (ip != PNetStation->StationIP)
	{
		PNetStation->StationIP = ip;
		stoe_WriteReg(STOE_IP_ADDR_REG, (byte XDATA*)&PNetStation->StationIP, 4);
	}

} // End of STOE_SetIPAddr

// ----------------------------------------------------------------------------
// Function Name: STOE_SetSubnetMask
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_SetSubnetMask(ulong subnet)
{
	if (subnet != PNetStation->SubnetMask)
	{
		PNetStation->SubnetMask = subnet;
		stoe_WriteReg(STOE_SUBNET_MASK_REG, (byte XDATA*)&PNetStation->SubnetMask, 4);
	}

} // End of STOE_SetSubnetMask

// ----------------------------------------------------------------------------
// Function Name: STOE_Gateway
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_SetGateway(ulong XDATA gateway)
{
	if (gateway != PNetStation->Gateway)
	{
		PNetStation->Gateway = gateway;

		stoe_WriteReg(STOE_GATEWAY_IP_REG, (byte XDATA*)&PNetStation->Gateway, 4);
		ETH_RestartSendArpToGateway();
	}

} // End of STOE_SetGateway
//#endif

// ----------------------------------------------------------------------------
// Function Name: STOE_Start
// Purpose: enable all used interrupts and set some register to start
//			timer, software-dma and processing received packets.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_Start(void)
{
	byte XDATA	temp;

	// clear interrupt status
	stoe_ReadReg(STOE_INT_STATUS_REG, &temp, 1);
	// start STOE L2/L3/L4 engines
	stoe_StartOperate();
	// start DMA module
	DMA_Start();
	// start MAC module
	MAC_Start();
	EXTINT4(1);	// Enable INT4 interrupt for stoe & mac modules.
} // End of STOE_Start

// ----------------------------------------------------------------------------
// Function Name: STOE_ProcessInterrupt
// Purpose: programmer can call this function by polling type to find that
//			interrupt event had happened.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_ProcessInterrupt(void)
{
	static byte XDATA	int_status;
	byte XDATA			temp;

	while (1)
	{
		// read stoe interrupt status
		stoe_ReadReg(STOE_INT_STATUS_REG, &int_status, 1);
		int_status &= STOE_DEFAULT_INT_MASK;

		if (!int_status)
			break;

		if (int_status & RCV_PACKET)
		{
			while (PBDP->RFP < PAGES_OF_RCV)
				stoe_RcvHandle();
		}
		if (int_status & RCV_BUF_RING_FULL)
		{
//			printd ("\rRX Packet Buffer Ring is Full.\n\r");
			temp = RESUME_PKT_RCV;
			stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);
		}
	}
}

// ----------------------------------------------------------------------------
// Function Name: STOE_WaitHwReleaseRcvBuf
// Purpose:
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_WaitHwReleaseRcvBuf(void)
{
	byte XDATA	temp;
	byte		isr;

	isr = EA;
	EA = 0;
	if (PBDP->RFP < PAGES_OF_RCV)
	{
		PBDP->RHPR = PBDP->RTPR;
		PBDP->RFP = PAGES_OF_RCV;
	}
	EA = isr;

	temp = RESUME_PKT_RCV;
	stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);

	DELAY_Us(1000);

	isr = EA;
	EA = 0;
	while (PBDP->RFP < PAGES_OF_RCV)
	{
		PBDP->RHPR = PBDP->RTPR;
		PBDP->RFP = PAGES_OF_RCV;
		EA = isr;

		DELAY_Us(1000);

		isr = EA;
		EA = 0;
	}
	EA = isr;

} // End of STOE_WaitHwReleaseRcvBuf()

// ----------------------------------------------------------------------------
// Function Name: stoe_StartOperate
// Purpose: start L2/L3/L4 engine
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void stoe_StartOperate(void)
{
	byte XDATA	temp;

	stoe_ReadReg(STOE_L2_CTL_REG, &temp, 1);

	temp |= (RX_START_OPERA | TX_START_OPERA);
	stoe_WriteReg(STOE_L2_CTL_REG, &temp, 1);

} // End of stoe_StartOperate

//#if 0
#if 1
// ----------------------------------------------------------------------------
// Function Name: stoe_StopOperate
// Purpose: stop L2/L3/L4 engine
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
void stoe_StopOperate(void)
{
	byte XDATA	temp;

	stoe_ReadReg(STOE_L2_CTL_REG, &temp, 1);

	temp &= ~(RX_START_OPERA | TX_START_OPERA);
	stoe_WriteReg(STOE_L2_CTL_REG, &temp, 1);

} // End of stoe_StopOperate
#endif

//-----------------------------------------------------------------------------
// Function Name: stoe_RcvHandle
// Purpose: 
// Params: 
// Returns:
// Note:
// ----------------------------------------------------------------------------
void stoe_RcvHandle(void)
{
	static STOE_FRAME_HEADER XDATA*	pframe;
	word	length;
	byte	pages, isr;

	pframe = ((PBDP->RHPR) << 8);
	length = (pframe->Length & 0xfff);

	PBDP->STOE_RxInform.Protocol = pframe->Protocol;
	PBDP->STOE_RxInform.TotalLen = length;
	PBDP->STOE_RxInform.PBuf = &pframe->Packet; 

	if (length && STOE_RcvCallback)
		STOE_RcvCallback(&pframe->Packet, length, pframe->Protocol);

	if (PBDP->RHPR > pframe->NPR)
		pages = (PBDP->TSPP - PBDP->RHPR) + (pframe->NPR - PBDP->RSPP);
	else
		pages = pframe->NPR - PBDP->RHPR;

	isr = EA;
	EA = 0;
	PBDP->RHPR = pframe->NPR;
	PBDP->RFP += pages;
	EA = isr;

} // End of stoe_RcvHandle

//-----------------------------------------------------------------------------
// Function Name: stoe_XmitHandle
// Purpose: Set send packet bit to tell hardware transmit one packet to ethernet.
// Params: 
// Returns:	
// Note:
// ----------------------------------------------------------------------------
void stoe_XmitHandle(void)
{
	byte XDATA temp;

	stoe_ReadReg(STOE_L4_CMD_REG, &temp, 1);
	while (temp & XMIT_PACKET)
		stoe_ReadReg(STOE_L4_CMD_REG, &temp, 1);

	temp = XMIT_PACKET;
	stoe_WriteReg(STOE_L4_CMD_REG, &temp, 1);

} // stoe_XmitHandle

//-----------------------------------------------------------------------------
// Function Name: STOE_Send
// Purpose:	Call this function to set add STOE header.
// Params:	pbuf : buffer address, must between TSPP and TEPP.
//			length : packet length.
//			protocol : if the packet is ip packet, set the ip layer protocol,
//						if not a ip packet, set value to 0xff.
// Returns:
// Note:
// ----------------------------------------------------------------------------
void STOE_Send(byte XDATA* pbuf, word length, byte protocol)
{
	STOE_FRAME_HEADER XDATA*	pframe =	(STOE_FRAME_HEADER XDATA*)pbuf;
	byte	occupypages = PBDP->STOE_TxInform.Pages;
	byte	isr;

	pframe->Length = length;
	pframe->Protocol = protocol;

	isr = EA;
	EA = 0;
	PBDP->TFP -= occupypages;
	EA = isr;

	PBDP->TTPR += occupypages;
	if (PBDP->TTPR > PBDP->TEPP)
		PBDP->TTPR = PBDP->TSPP + PBDP->TTPR - PBDP->TEPP - 1;

	pframe->NPR = PBDP->TTPR;

	stoe_XmitHandle();

} // End of STOE_Send()


//-----------------------------------------------------------------------------
// Function Name: STOE_AssignSendBuf
// Purpose: Assign a buffer that adress is between TSPP and TEPP, so STOE engine
//				cans transmit the packet to MAC SRAM.
// Params: length : Buffer size that the packet occupied.
// Returns: Assigned transmit buffer address
// Note:
// ----------------------------------------------------------------------------
byte XDATA* STOE_AssignSendBuf(word length)
{
	byte XDATA*	pframe;
	byte		occupypages;

	occupypages = ((length + PAGE_SIZE - 1)/PAGE_SIZE);

	if (PBDP->TFP <= occupypages)
		return (byte XDATA*)0;

	pframe = ((PBDP->TTPR) << 8);
	PBDP->STOE_TxInform.Pages = occupypages;

	return pframe;

} // End of STOE_AssignSendBuf

/*//commented out by cjm
#if (!BOOTLDR_ISR)
//commented out by cjm*/
//-----------------------------------------------------------------------------
// Function Name: STOE_CopyCode2TPBR
// Purpose: 
// Params: 
// Returns:
// Note:
// ----------------------------------------------------------------------------
byte XDATA* STOE_CopyCode2TPBR(byte XDATA* pdest, byte* psour, word length)
{
	word	space, leftlen;

	// destination address is not in range of TPBR.
	if ((pdest > PBDP->XmtEndAddr) || (pdest < PBDP->XmtStartAddr))
		return 0;

	space = (word)PBDP->XmtEndAddr - (word)pdest + 1;
	if (space > length)
	{
		while (length--)
			*pdest++ = *psour++;
	}
	else
	{
		leftlen = length - space;

		while (space--)
			*pdest++ = *psour++;

		pdest = PBDP->XmtStartAddr;

		while (leftlen--)
			*pdest++ = *psour++;
	}

	return pdest;

} // End of STOE_CopyCode2TPBR
/*//commented out by cjm
#endif
//commented out by cjm*/

//-----------------------------------------------------------------------------
// Function Name: STOE_CHECK_MAC
// Purpose: 
// Params: 
// Returns:
// Note:
// ----------------------------------------------------------------------------
byte STOE_CHECK_MAC(ulong *ip_addr)
{
	byte entry_addr, result[6] = {0};
	entry_addr = (byte)(*ip_addr & 0x7F) * 2;
// 	PRINTD(DEBUG_MSG, ("STOE_CHECK_MAC :: entry_addr = %bx\n\r", entry_addr));
	stoe_WriteReg(STOE_ARP_ADDR_REG, &entry_addr, 1);

	entry_addr = ARP_CACHE_CMD_READ | ARP_CACHE_CMD_GO;
	stoe_WriteReg(STOE_ARP_CMD_REG, &entry_addr, 1);

	stoe_ReadReg(STOE_ARP_DATA_REG, result, 6);

	if (result[0]& 0x80)
		return 1;
	
	return 0;

} // End of STOE_CHECK_MAC()

// End of stoe.c