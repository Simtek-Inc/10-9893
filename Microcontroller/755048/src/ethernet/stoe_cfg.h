//========================================================================================
//     Copyright (c) 2006     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//========================================================================================
// Module Name    : stoe_cfg.h
// Purpose        : 
// Author         :
// Date           :
// Notes          :
// $Log           : stoe_cfg.h,v $
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.2      2006/07/25 05:32:12  borbin
// no message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.1.1.1  2006/06/20 05:50:28  borbin
// no message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//========================================================================================
//
#ifndef __STOE_CFG_H__
#define __STOE_CFG_H__
//          INCLUDE FILE DECLARATIONS                                                     //
//----------------------------------------------------------------------------------------
#define PAGES_OF_XMIT                           8                                         // pages. 2k // //16     // pages, 4k //
#define PAGES_OF_RCV                            16                                        // pages. 4k // //24  // pages, 6k //
//----------------------------------------------------------------------------------------
#define STOE_ARP_TIMEOUT                        16                                        // hardware arp table timeout is 128 sec. //
//----------------------------------------------------------------------------------------
//          NAMING CONSTANT DECLARATIONS                                                  //
//----------------------------------------------------------------------------------------
#define STOE_HW_CHECKSUN_DISABLE                0                                         //    0: disable stoe checksum offload engine //
#define STOE_HW_CHECKSUN_ENABLE                 1                                         //    1: enable stoe checksum offload engine. //
#define STOE_CHECKSUM_OFFLOAD                   (STOE_HW_CHECKSUN_ENABLE)                 //
//----------------------------------------------------------------------------------------
#endif                                    //    End of __STOE_CFG_H__         //
// End of stoe_cfg.h //
