//----------------------------------------------------------------------------------------
//     Copyright (c) 2005	Simtek, Incorporated      All rights reserved.
//
//     This is unpublished proprietary source code of Simtek, Inc
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name : types.h
// Purpose     : Definition of type.
// Author      : Robin Lee
// Date        : 2005-12-28
// Notes       : None.
// $Log: types.h,v $
//----------------------------------------------------------------------------------------
// Revision 1.2  2006/05/18 05:02:30  Louis
// no message
//----------------------------------------------------------------------------------------
// Revision 1.1  2006/04/07 11:36:17  robin6633
// no message
//----------------------------------------------------------------------------------------

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char               bool;                                                 // boolean
typedef unsigned long               ulong;                                                // 32-bit unsigned
typedef unsigned short int          word;                                                 // 16-bit unsigned
typedef unsigned char               byte;                                                 //  8-bit unsigned
typedef unsigned short int          uint;                                                 // 16-bit unsigned

#ifndef NULL
  #define NULL ((void *) 0L)
#endif

#define TRUE                  1
#define FALSE                 0
#define ON                    1
#define OFF                   0
#define Client                1
#define Server                0

// Bit Definitions for Bitwise Operation //
#define BIT0            0x01
#define BIT1            0x02
#define BIT2            0x04
#define BIT3            0x08
#define BIT4            0x10
#define BIT5            0x20
#define BIT6            0x40
#define BIT7            0x80
#define BIT8            0x0100
#define BIT9            0x0200
#define BIT10           0x0400
#define BIT11           0x0800
#define BIT12           0x1000
#define BIT13           0x2000
#define BIT14           0x4000
#define BIT15           0x8000
#define BIT16           0x00010000
#define BIT17           0x00020000
#define BIT18           0x00040000
#define BIT19           0x00080000
#define BIT20           0x00100000
#define BIT21           0x00200000
#define BIT22           0x00400000
#define BIT23           0x00800000
#define BIT24           0x01000000
#define BIT25           0x02000000
#define BIT26           0x04000000
#define BIT27           0x08000000
#define BIT28           0x10000000
#define BIT29           0x20000000
#define BIT30           0x40000000
#define BIT31           0x80000000

#define LOW_BYTE(dat)   (byte) (dat & 0x00FF)
#define HIGH_BYTE(dat)  (byte)((dat & 0xFF00) >> 8)

#define U32BYTE0(dat)   (byte) (dat  & 0x000000FF)
#define U32BYTE1(dat)   (byte)((dat  & 0x0000FF00) >> 8)
#define U32BYTE2(dat)   (byte)((dat  & 0x00FF0000) >> 16)
#define U32BYTE3(dat)   (byte)((dat  & 0xFF000000) >> 24)
 
// Keil compiler user define //
#define     KEIL_CPL

#ifdef KEIL_CPL
 #define XDATA    xdata
 #define IDATA    idata
 #define BDATA    bdata
 #define CODE     code
 #define FAR      far
#else
 #define XDATA
 #define IDATA
 #define BDATA
 #define CODE
 #define FAR
#endif

// Serial interface command direction //
#define     SI_WR                   BIT0
#define     SI_RD                   BIT1

#define     FLASH_WR_ENB            (PCON |= PWE_)
#define     FLASH_WR_DISB           (PCON &= ~PWE_)

#endif 	// end of TYPES_H //