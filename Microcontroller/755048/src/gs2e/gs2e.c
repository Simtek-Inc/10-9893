//----------------------------------------------------------------------------------------
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name: gs2e.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: gs2e.c,v $
//----------------------------------------------------------------------------------------

// INCLUDE FILE DECLARATIONS //
#include "adapter.h"
#include "gs2e.h"
#include "gudpdat.h"
#include "gudpbc.h"
#include "gudpuc.h"
#include "gconfig.h"
#include "printd.h"
#include "mac.h"
#include "ax11000.h"                                                                      //
#include "eeprom.h"
#include "ping.h"
#include "instrument.h"
#include "Hardware.H"
// NAMING CONSTANT DECLARATIONS //

// MACRO DECLARATIONS //

// GLOBAL VARIABLES DECLARATIONS //

// LOCAL VARIABLES DECLARATIONS //
static byte XDATA gs2e_State;
static byte XDATA gs2e_ConnType;

void  GS2E_PortLeaseTimeoutUpdate(void);
uint  DEADMANcnt;
uint  PortLeaseTimeoutVal;                                                            //
// LOCAL SUBPROGRAM DECLARATIONS //

//----------------------------------------------------------------------------------------
// name    : GS2E_PortLeaseTimeoutUpdate
// purpose : 
// params  :
// returns :
// note    :
//----------------------------------------------------------------------------------------
void  GS2E_PortLeaseTimeoutUpdate(void)
{
    if (DEADMANcnt == 0)                                                                  // check to make sure the connection has been made
      {
      if(PortLeaseTimeoutVal != 0)                                                        // check to see the port is being used
        {
        if(PortLeaseTimeoutVal == 1)                                                      // see if the timeout on the port has been reached
          {
          PortLeaseTimeoutVal = 0;                                                        // turn the port lease timeout off
          if(acResetRequest < 0xffff)                                                     // make sure the maximum number of action requests is not pending
            { acResetRequest++; }                                                         // trigger a reset action
          printd("\rConnection inactive");                                                //

          }
        else                                                                              // the port lease timeout has not been reached
          {
          PortLeaseTimeoutVal--;                                                          // update the port lease timeout
          }
        }
      }
}

//----------------------------------------------------------------------------------------
// name    : GS2E_Task
// Purpose       : 
// Params        :
// Returns       :
// Note          :
//----------------------------------------------------------------------------------------
void GS2E_Task(void)
{
	GCONFIG_Task();
	if (gs2e_ConnType == GS2E_CONN_UDP)
	  GUDPDAT_Task();
    if (DEADMANcnt > 0)
      {
      if (gs2e_State == GS2E_STATE_UDP_DATA_PROCESS)
        DEADMANcnt = 0;
      else
        {
        DEADMANcnt--;
        printd("DEADMANcnt = %-5du\r",DEADMANcnt);
        }
      }
    if (DEADMANcnt == 1) {  AX11000_SoftReboot();  DEADMANcnt = 0;  }
    GS2E_PortLeaseTimeoutUpdate();
} // End of GS2E_Task() //

//----------------------------------------------------------------------------------------
// name    : GS2E_Init()
// Purpose       : Initialization
// Params        :
// Returns       :
// Note          :
//----------------------------------------------------------------------------------------
void GS2E_Init(void)
{	
	gs2e_State = GS2E_STATE_IDLE;                                                       // 
	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP_BCAST) == GCONFIG_NETWORK_PROTO_UDP_BCAST)
	  {
	  GUDPBC_Init(GCONFIG_UDP_BCAST_SERVER_PORT);                                       // 
	  }
	GUDPUC_Init(GCONFIG_UDP_UCAST_SERVER_PORT);                                         // 

	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_PROTO_UDP) == GCONFIG_NETWORK_PROTO_UDP)
	  {
	  gs2e_ConnType = GS2E_CONN_UDP;                                                    // 
	  GUDPDAT_Init(GCONFIG_GetServerDataPktListenPort());                               // 
	  }
    DEADMANcnt          = 3000;                                                           // 
    PortLeaseTimeoutVal = PortLeaseTimeout;                                               // set the port lease timeout for an approximately 5 second delay
	EEPROM_Init();
} // End of GS2E_Init() //

//----------------------------------------------------------------------------------------
// name    : GS2E_GetTaskState
// Purpose       : 
// Params        :
// Returns       :
// Note          :
//----------------------------------------------------------------------------------------
byte GS2E_GetTaskState(void)
{
	return gs2e_State;
} // End of GS2E_GetTaskState() //

//----------------------------------------------------------------------------------------
// name    : GS2E_SetTaskState
// Purpose       : 
// Params        :
// Returns       :
// Note          :
//----------------------------------------------------------------------------------------
void GS2E_SetTaskState(byte state)
{
	gs2e_State = state;
} // End of GS2E_SetTaskState() //

// ----------------------------------------------------------------------------
// name    : GS2E_GetConnType
// Purpose       : 
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
byte GS2E_GetConnType(void)
{
	return gs2e_ConnType;
} // End of GS2E_GetConnType() //

// End of gs2e.c //
