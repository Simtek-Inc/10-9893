//----------------------------------------------------------------------------------------
//     Copyright (c) 2005     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//----------------------------------------------------------------------------------------
//
//========================================================================================
// Module Name : uart.h
// Purpose     : A header file of uart.c includes UART0 and UART1 module.
// Author      : Robin Lee
// Date        : 2006-01-10
// Notes       :
// $Log: uart.h,v $
// Revision 1.1  2006/05/12 14:24:47  robin6633
// no message
//
// Revision 1.3  2006/05/03 02:46:00  robin6633
// Changed the the function name UART_GetBufCount()
// to UART_GetRxBufCount() .
//
// Revision 1.2  2006/05/02 01:43:45  robin6633
// Add an expanding function to get the counter value of UART software buffer.
//
// Revision 1.1  2006/04/07 11:39:14  robin6633
// no message
//
//========================================================================================

#ifndef __UART_H__
#define __UART_H__

// INCLUDE FILE DECLARATIONS //
#include	"uart_cfg.h"


// NAMING CONSTANT DECLARATIONS //
#if UART0_ENABLE
#define MAX_TX_UART0_BUF_SIZE	128 //512
#define MAX_TX_UART0_MASK		(MAX_TX_UART0_BUF_SIZE - 1)
#define MAX_RX_UART0_BUF_SIZE	64 //512
#define MAX_RX_UART0_MASK		(MAX_RX_UART0_BUF_SIZE - 1)
#endif

#if UART1_ENABLE
#define MAX_TX_UART1_BUF_SIZE	128
#define MAX_TX_UART1_MASK		(MAX_TX_UART1_BUF_SIZE - 1)
#define MAX_RX_UART1_BUF_SIZE	128
#define MAX_RX_UART1_MASK		(MAX_RX_UART1_BUF_SIZE - 1)
#endif

/* GLOBAL VARIABLES */
#if UART0_ENABLE
extern word  uart0_RxCount;
#endif

#if UART1_ENABLE
extern word  uart1_RxCount;
extern word  uart1_RxTail;
extern byte  uart1_RxBuf[MAX_RX_UART1_BUF_SIZE];
extern word  uart1_RxHead;
#endif

#define SCS100M                                    0xC0
#define SCSx75M                                    0x80
#define SCSx50M                                    0x40
#define SCSx25M                                    0x00

#define Baud___1200                                0x00
#define Baud___2400                                0x01
#define Baud___4800                                0x02
#define Baud___9600                                0x03
#define Baud__19200                                0x04
#define Baud__28800                                0x05
#define Baud__38400                                0x06
#define Baud__57600                                0x07
#define Baud__76800                                0x08
#define Baud__94750                                0x09
#define Baud_115200                                0x0A

#define F_25_B___1200                              Baud___1200 | SCSx25M
#define F_25_B___2400                              Baud___2400 | SCSx25M
#define F_25_B___4800                              Baud___4800 | SCSx25M
#define F_25_B___9600                              Baud___9600 | SCSx25M
#define F_25_B__19200                              Baud__19200 | SCSx25M
#define F_25_B__28800                              Baud__28800 | SCSx25M
#define F_25_B__38400                              Baud__38400 | SCSx25M
#define F_25_B__57600                              Baud__57600 | SCSx25M
#define F_25_B__76800                              Baud__76800 | SCSx25M
#define F_25_B__94750                              Baud__94750 | SCSx25M
#define F_25_B_115200                              Baud_115200 | SCSx25M
#define F_50_B___1200                              Baud___1200 | SCSx50M
#define F_50_B___2400                              Baud___2400 | SCSx50M
#define F_50_B___4800                              Baud___4800 | SCSx50M
#define F_50_B___9600                              Baud___9600 | SCSx50M
#define F_50_B__19200                              Baud__19200 | SCSx50M
#define F_50_B__28800                              Baud__28800 | SCSx50M
#define F_50_B__38400                              Baud__38400 | SCSx50M
#define F_50_B__57600                              Baud__57600 | SCSx50M
#define F_50_B__76800                              Baud__76800 | SCSx50M
#define F_50_B__94750                              Baud__94750 | SCSx50M
#define F_50_B_115200                              Baud_115200 | SCSx50M
#define F100_B___1200                              Baud___1200 | SCS100M
#define F100_B___2400                              Baud___2400 | SCS100M
#define F100_B___4800                              Baud___4800 | SCS100M
#define F100_B___9600                              Baud___9600 | SCS100M
#define F100_B__19200                              Baud__19200 | SCS100M
#define F100_B__28800                              Baud__28800 | SCS100M
#define F100_B__38400                              Baud__38400 | SCS100M
#define F100_B__57600                              Baud__57600 | SCS100M
#define F100_B__76800                              Baud__76800 | SCS100M
#define F100_B__94750                              Baud__94750 | SCS100M
#define F100_B_115200                              Baud_115200 | SCS100M



// EXPORTED SUBPROGRAM SPECIFICATIONS //
       void  uart_Init(byte port, baudrate);
extern void  UART_ShowBuffer(byte* pd, byte eol);
       bool  uart_ParityChk(char checkbyte);
       bool  uart_SetPort(byte portNum);
// UART 0 routines
extern word  uart0_PutChar(char c);
extern word  uart0_GetKey(void);

extern byte  UART0_setBaudRate(byte baudrate);
extern void  UART0_PutStr(byte* pData, byte cnt);
extern void  UART0_GetBuf(byte* pData, byte cnt);
extern void  UART0_RxBufUpdate(byte cnt);
extern byte  UART0_RxPtrSet(byte c);
extern void  UART0_RxBufSet(byte c, word  len);
extern byte  UART0_RxBufCmp(byte *pData,byte len);
extern word  UART0_GetByteCount(void);
extern void  UART0_DisplayRxPointers(void);
extern void  UART0_ShowRxBuffer(void);
       byte  UART0_TxChk(void);
       void  UART0_SetMode(byte mode);
word  UA0_GetCount(void);
extern char  uart0_NoBlockGetkey(void);
// UART 1 routines
extern word  uart1_PutChar(char c);
extern word  uart1_GetKey(void);
extern void  UART1_PutStr(byte* pData, byte cnt);
extern void  UART1_GetBuf(byte* pData, byte cnt);
extern void  UART1_RxBufUpdate(byte cnt);
extern byte  UART1_RxPtrSet(byte c);
extern void  UART1_RxBufSet(byte c, word  len);
extern byte  UART1_RxBufCmp(byte *pData,byte len);
extern word  UART1_GetByteCount(void);
extern void  UART1_DisplayRxPointers(byte crlf);
       byte  UART1_TxChk(void);
       void  UART1_SetMode(byte mode);
word  UA1_GetCount(void);
extern char  uart1_NoBlockGetkey(void);
//
void  PMM_Uart0Init(void);

#endif // End of __UART_H__ //
// End of uart.h //