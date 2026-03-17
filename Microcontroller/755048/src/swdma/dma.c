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
// Module Name: dma.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: dma.c,v $
// Revision 1.1  2006/05/12 14:24:19  robin6633
// no message
//
// Revision 1.1.1.1  2006/02/23 00:55:10  borbin
// no message
//
//=============================================================================
//

// INCLUDE FILE DECLARATIONS //
#include "reg80390.h"
#include "dma.h"
#if (DMA_INCLUDE_ETHERNET_MODULE ==	DMA_YES)
#include "stoe.h"
#endif
#include <intrins.h>

// ----------------------------------------------------------------------------
// Function Name : DMA_ReadReg
// Purpose       : Read software-dma register.
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
void DMA_ReadReg(byte regaddr, byte XDATA* pbuf, byte length)
{
	byte	isr;

	isr   = EA;
	EA    = 0;
	_nop_();
	DCIR  = regaddr;
	while (length--)
	  pbuf[length] = DDR;
	EA    = isr;

} // End of DMA_ReadReg //

// ----------------------------------------------------------------------------
// Function Name : DMA_WriteReg
// Purpose       : Set software-dma register.
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
void DMA_WriteReg(byte regaddr, byte XDATA* pbuf, byte length)
{
	byte	isr;

	isr   = EA;
	EA    = 0;
	while (length--)
	  DDR = pbuf[length];
	DCIR  = regaddr;
	EA    = isr;

} // End of STOE_DMA_WriteReg //

// ----------------------------------------------------------------------------
// Function Name : DMA_Init
// Purpose       : Initial software-dma module.
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
void DMA_Init(void)
{
	// set DMA gap //
	TDR  = DMA_GAP;
	TCIR = 0x24;

} // End of DMA_Init //

// ----------------------------------------------------------------------------
// Function Name : DMA_Start
// Purpose       : Start software-dma module.
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
void DMA_Start(void)
{
	EINT2 = 1;	// Enable INT2 interrupt for DMA request. //

} // End of DMA_Start //

// ----------------------------------------------------------------------------
// Function Name: dma_CheckStatus
// Purpose      : Wait software-dma to complete, and check if dma error happened or not?
// Params       : 
// Returns      : 0 : dma error had happened, so set DMA_FORCE_STOP bit for
//				      dma can work next time.
//			      1 : dma had completed.
// Note         :
// ----------------------------------------------------------------------------
byte dma_CheckStatus(void)
{
	byte XDATA	temp;

	while (1)
	  {
	  DMA_ReadReg(SW_DMA_CMD_REG, &temp, 1);
	  if (temp & 0x37)
	    {
	  	if (temp & DMA_ERROR_BIT)
	  	  {
	  	  temp &= ~0x7f;
	  	  temp |= DMA_FORCE_STOP;
	  	  DMA_WriteReg(SW_DMA_CMD_REG, &temp, 1);
	  	  return 0;
	  	  }
	  	}
	  	else
	  	  return 1;
	  }
} // End of dma_CheckStatus //

#if (DMA_INCLUDE_ETHERNET_MODULE == DMA_YES)
// ----------------------------------------------------------------------------
// Function Name : DMA_GrantXdata
// Purpose       : Use sw-dma doing memory copy. The scope of source and destination
//			    	 target must be smaller than 64k sram memory.
//			       If the scope of either source or destination target is large than
//				     64k memory, the programmers can call DMA_Grant() instead of
//				     this function.
//			       DMA_GrantXdata() can run faster than DMA_Grant() function.
// Params        : 
// Returns       : A address of XDATA memory that point to the next address of
//				     the end address of the scope of destination target.
// Note          :
// ----------------------------------------------------------------------------
byte XDATA* DMA_GrantXdata(byte XDATA* pdest, byte XDATA* psour, word length)
{
	byte XDATA		   temp[3];
	word XDATA* 	   ptempshort = (word XDATA*)&temp[1];
	byte 			   dmatype    = 0;

	temp[0]          = 0;
	*ptempshort      = psour;
	DMA_WriteReg(SW_DMA_SOUR_ADDR_REG, temp, 3);
	*ptempshort      = pdest;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, temp, 3);
	*ptempshort      = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, temp + 1, 2);

	if ((psour >= PBDP->RcvStartAddr) && (psour <= PBDP->RcvEndAddr))
	  dmatype        = DMA_FROM_RPBR;
	if ((pdest >= PBDP->XmtStartAddr) && (pdest <= PBDP->XmtEndAddr))
	  dmatype       |= DMA_TO_TPBR;

	temp[0]          = (DMA_CMD_GO | dmatype);
	DMA_WriteReg(SW_DMA_CMD_REG, temp, 1);

	if (dmatype | DMA_TO_TPBR)
	  {
	  pdest         += length;
	  if (pdest > PBDP->XmtEndAddr)
	    pdest       -= STOE_XMIT_BUF_SIZE;
	  }

	// check software dma had completed. //
	if (dma_CheckStatus())
	  return pdest;
	else
	  return 0;

} // DMA_GrantXdata //

#if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)
// ----------------------------------------------------------------------------
// Function Name : DMA_Grant
// Purpose       : Use sw-dma doing memory copy. 
// Params        : 
// Returns       : A address of sram memory that point to the next address of
//				     the end address of the scope of destination target.
// Note          :
// ----------------------------------------------------------------------------
byte* DMA_Grant(byte* pdest, byte* psour, word length)
{
	byte  XDATA		   sourtemp[4];
	byte  XDATA		   desttemp[4];
	ulong XDATA*	   psourtemplong = (ulong XDATA*)&sourtemp[0];
	ulong XDATA*	   pdesttemplong = (ulong XDATA*)&desttemp[0];
	word XDATA*	   	   ptempshort;
	byte			   dmatype       = 0;
	byte*			   ptemp;

	*psourtemplong   = (ulong)psour;
	*pdesttemplong   = (ulong)pdest;

	sourtemp[1]--;	// for Keil C memory type //
	DMA_WriteReg(SW_DMA_SOUR_ADDR_REG, &sourtemp[1], 3);

	if (sourtemp[1] == 0)
	  {
	  ptempshort     = (word XDATA*)&sourtemp[2];
	  if ((*ptempshort >= (word)PBDP->RcvStartAddr) && (*ptempshort <= (word)PBDP->RcvEndAddr))
	  	dmatype      = DMA_FROM_RPBR;
	  }

	desttemp[1]--;	// for Keil C memory type //
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, &desttemp[1], 3);

	if (desttemp[1] == 0)
	  {
	  ptempshort     = (word XDATA*)&desttemp[2];
	  if ((*ptempshort >= (word)PBDP->XmtStartAddr) && (*ptempshort <= (word)PBDP->XmtEndAddr))
	    dmatype     |= DMA_TO_TPBR;
	  }

	ptempshort       = (word XDATA*)&sourtemp[0];
	*ptempshort      = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, sourtemp, 2);

	sourtemp[0]      = (DMA_CMD_GO | dmatype);
	DMA_WriteReg(SW_DMA_CMD_REG, sourtemp, 1);

	if (dmatype | DMA_TO_TPBR)
	  {
	  ptemp          = pdest;
	  pdest         += length;
// for KEIL //
	  if (ptemp > pdest)
	    pdest       += 0x10000;
// end //
	  ptempshort     = (word XDATA*)&desttemp[2];
	  (*ptempshort) += length;
	  if (*ptempshort > (word)PBDP->XmtEndAddr)
	    pdest       -= STOE_XMIT_BUF_SIZE;
	  }

	// check software dma had completed. //
	if (dma_CheckStatus())
	  return pdest;
	else
	return 0;

} // DMA_Grant //
#endif // End of #if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K) //
#else
// ----------------------------------------------------------------------------
// Function Name : DMA_GrantXdata
// Purpose       : Use sw-dma doing memory copy. The scope of source and destination
//	      			target must	be smaller than 64k sram memory.
//		           If the scope of either source or destination target is large than
//			      	64k memory,	the programmers can call DMA_Grant() instead of
//      			this function.
//		      	   DMA_GrantXdata() can run faster than DMA_Grant() function.
// Params        : 
// Returns       :  A address of XDATA memory that point to the next address of
//			         the end address of the scope of destination target.
// Note          :
// ----------------------------------------------------------------------------
byte XDATA* DMA_GrantXdata(byte XDATA* pdest, byte XDATA* psour, word length)
{
	byte XDATA	   temp[3];
	word XDATA*   ptempshort = (word XDATA*)&temp[1];
		
	temp[0]      = 0;
	*ptempshort  = psour;
	DMA_WriteReg(SW_DMA_SOUR_ADDR_REG, temp, 3);
	*ptempshort  = pdest;
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, temp, 3);
	*ptempshort  = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, temp + 1, 2);

	temp[0] = DMA_CMD_GO;
	DMA_WriteReg(SW_DMA_CMD_REG, temp, 1);

	pdest       += length;

	// check software dma had completed. //
	if (dma_CheckStatus())
	  return pdest;
	else
	  return 0;

} // DMA_GrantXdata //

#if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K)
// ----------------------------------------------------------------------------
// Function Name : DMA_Grant
// Purpose       : Use dma doing memory copy. 
// Params        :
// Returns       : A address of sram memory that point to the next address of
//				   the end address of the scope of destination target.
// Note          :
// ----------------------------------------------------------------------------
byte* DMA_Grant(byte* pdest, byte* psour, word length)
{
	byte XDATA		temp[4];
	ulong XDATA*	ptemplong = (ulong XDATA*)&temp[0];
	word XDATA*	ptempshort;
	byte*			ptemp;

	*ptemplong  = (ulong)psour;
	temp[1]--;	// for Keil C memory type //
	DMA_WriteReg(SW_DMA_SOUR_ADDR_REG, &temp[1], 3);

	*ptemplong  = (ulong)pdest;
	temp[1]--;	// for Keil C memory type //
	DMA_WriteReg(SW_DMA_TARGET_ADDR_REG, &temp[1], 3);

	ptempshort  = (word XDATA*)&temp[0];
	*ptempshort = length;
	DMA_WriteReg(SW_DMA_BYTE_COUNT_REG, temp, 2);

	temp[0]     = DMA_CMD_GO;
	DMA_WriteReg(SW_DMA_CMD_REG, temp, 1);

	ptemp       = pdest;
	pdest      += length;
// for KEIL //
	if (ptemp > pdest)
	  pdest    += 0x10000;
// end //
	// check software dma had completed. //
	if (dma_CheckStatus())
	  return pdest;
	else
	  return 0;

} // DMA_Grant //
#endif // End of #if (DMA_SRAM_RANGE == DMA_COPY_LARGE_THAN_64K) //


#endif // End of #if (DMA_INCLUDE_ETHERNET_MODULE == DMA_YES) //

// End of dma.c //