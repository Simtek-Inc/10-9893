//
//================================================================================
//     Copyright (c) 2005	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//================================================================================
//
//================================================================================
// Module Name : interrupt.c
// Purpose     : This file include peripheral interrupt service routine and
//               the wake up interrupt service routine.
// Author      : Robin Lee
// Date        : 2006-01-09
// Notes       :
// $Log: interrupt.c,v $
// Revision 1.3  2006/07/20 03:50:59  robin6633
// 1.Change the header file name "reg80390.h" to "ax110xx.h".
//
// Revision 1.2  2006/04/20 01:22:56  robin6633
// 1.Added watchdog timeout ISR.
//
// Revision 1.1  2006/04/07 11:36:17  robin6633
// no message
//
//================================================================================
//

// INCLUDE FILE DECLARATIONS //
#include	"reg80390.h"
#include	"types.h"
#include	"interrupt.h"
#include	"mac.h"
#include	"stoe.h"

// NAMING CONSTANT DECLARATIONS //

// STATIC VARIABLE DECLARATIONS //
byte	ax11000WakeupEvent = 0;


// LOCAL SUBPROGRAM DECLARATIONS //
static void	ax11000_PeripherialISR(void);
static void	ax11000_PmmISR(void);

// LOCAL SUBPROGRAM BODIES //

//
//--------------------------------------------------------------------------------
// static void ax11000_PeripherialISR(void)
// Purpose : The interrupt service routine of Flipper peripheral devices.
// Params  : None.
// Returns : None.
// Note    : None.
//--------------------------------------------------------------------------------
//
static void ax11000_PeripherialISR(void) interrupt 9 //use external interrupt 4 (0x4B)
{
	byte	intrStt1 = 0;
	byte	intrStt2 = 0;

	// Interrupt type check //
	while (1)
	  {
	  EA = 0;
	  intrStt1 = PISSR1;
	  intrStt2 = PISSR2;
	  EA = 1;

	  if ((intrStt1 == 0) && (intrStt2 == 0))
	    break;

	  if (intrStt1 & ETH_INT_STU)
	    {
	    MAC_SetInterruptFlag();
	    }
	}
}

//
//--------------------------------------------------------------------------------
// static void ax11000_PmmISR(void)
// Purpose : The interrupt service routine of the
//           wake up event in power management mode.
// Params  : None.
// Returns : None.
// Note    : None.
//--------------------------------------------------------------------------------
 //
static void ax11000_PmmISR(void) interrupt 11 //use external interrupt 6 (0x5B)
{
	byte	wakeStatus;

	EA = 0;
	EIF &= ~INT6F;	// Clear the flag of interrupt 6.
	EA = 1;
	wakeStatus = PCON;
	if (!(wakeStatus & SWB_))
	  {
	  PCON &= ~ (PMM_ | STOP_);
	  }

	// Ethernet wake up function //
	wakeStatus = 0;
/*
#if MAC_REMOTE_WAKEUP
	wakeStatus = MAC_ReadWakeupStatus();
	ax11000WakeupEvent = wakeStatus;
	printd("w\n\r");
#endif
*/
}
// EXPORTED SUBPROGRAM BODIES //
// End of interrupt.c //
