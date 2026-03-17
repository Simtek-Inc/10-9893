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
// Module Name: mstimer.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: mstimer.c,v $
// Revision 1.2  2006/07/25 05:34:56  borbin
// no message
//
// Revision 1.1.1.1  2006/06/20 05:50:28  borbin
// no message
//
//=============================================================================
//

// INCLUDE FILE DECLARATIONS //
#include "reg80390.h"
#include "mstimer.h"
#include "interrupt.h"
#include "dma.h"
#include "ax11000_cfg.h"

// NAMING CONSTANT DECLARATIONS //

// GLOBAL VARIABLES DECLARATIONS //
byte IDATA ExecuteRuntimeFlag _at_ 0x30;
/*//commented out by cjm
#if (BOOTLDR_ISR)
  byte CODE RT_SWTIMER_INT _at_ 0x6073;
#endif
//commented out by cjm*/

// LOCAL VARIABLES DECLARATIONS //
static ulong swtimer_Counter = 0;


//
// ----------------------------------------------------------------------------
// Function Name: SWTIMER_Timer
// Purpose: 
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
static void SWTIMER_Timer(void) interrupt 10
{
		byte	temp;

		EINT5 = 0;
		EIF &= ~INT5F;
		temp = SDSTSR;
		if (temp & SWTIMER_EXPIRED)
			swtimer_Counter++;
		EINT5 = 1;

} // End of SWTIMER_Timer //

//
// ----------------------------------------------------------------------------
// Function Name: SWTIMER_Init
// Purpose: Initial software timer module.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void SWTIMER_Init(void)
{
#ifdef RuntimeCodeAt24KH
		ExecuteRuntimeFlag |= 0x01;
#endif

} // End of SWTIMER_Init //

//
// ----------------------------------------------------------------------------
// Function Name: SWTIMER_Start
// Purpose: Enable and start software timer.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void SWTIMER_Start(void)
{
	byte XDATA	temp[2];

	// set software timer //
	temp[1] = (byte)SWTIMER_INTERVAL;
	temp[0] = (byte)(SWTIMER_INTERVAL >> 8) + SWTIMER_START + SWTIMER_INT_ENABLE;
	DMA_WriteReg(SWTIMER_REG, temp, 2);
	EXTINT5(1);	// Enable INT5 interrupt for software timer. //
} // End of SWTIMER_Start //

#if (SWTIMER_STOP_FUNC == SWTIMER_SUPPORT_STOP_FUNC)
//
// ----------------------------------------------------------------------------
// Function Name: SWTIMER_Stop
// Purpose: Disable and Stop software timer.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
void SWTIMER_Stop(void)
{
	byte XDATA	temp[2];

	// set software timer //
	temp[1] = (byte)SWTIMER_INTERVAL;
	temp[0] = (byte)(SWTIMER_INTERVAL >> 8);
	DMA_WriteReg(SWTIMER_REG, temp, 2);
} // End of SWTIMER_Stop //
#endif

//
// ----------------------------------------------------------------------------
// Function Name: SWTIMER_Tick
// Purpose: Get software timer count.
// Params:
// Returns:
// Note:
// ----------------------------------------------------------------------------
 //
ulong SWTIMER_Tick(void)
{
	ulong	timer;
	byte	isr;

	isr = EA;
	EA = 0;
	timer = swtimer_Counter;
	EA = isr;
	return timer;
} // End of SWTIMER_Tick //


// End of mstmier.c //