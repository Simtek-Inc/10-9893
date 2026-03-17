//
//================================================================================
//     Copyright (c) 2005     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//================================================================================
//
//================================================================================
// Module Name : uart.c
// Purpose     : The UART module driver. It manages the character
//               buffer and handles the ISR. This driver includes UARTs 0 & 1
// Author      : Robin Lee
// Date        : 2006-01-10
// Notes       : None.
//================================================================================
//

// INCLUDE FILE DECLARATIONS //
#include    "reg80390.h"
#include    "types.h"
#include    "uart.h"
#include    "ax11000_cfg.h"
#include    "printd.h"
#include    "adapter_cfg.h"
#include    <string.h>


// STATIC VARIABLE DECLARATIONS //
#if UART0_ENABLE
static byte       uart0_TxBuf[MAX_TX_UART0_BUF_SIZE];
static word       uart0_TxHead  = 0;
static word       uart0_TxTail  = 0;
static word       uart0_TxCount = 0;
static byte       uart0_TxFlag  = 0;
static byte       uart0_RxBuf[MAX_RX_UART0_BUF_SIZE];
static word       uart0_RxHead  = 0;
static word       uart0_RxTail  = 0;
//static byte       uart0_Mode = 0;
word              uart0_RxCount = 0;
#endif

#if UART1_ENABLE
static byte       uart1_TxBuf[MAX_TX_UART1_BUF_SIZE];
static word       uart1_TxHead  = 0;
static word       uart1_TxTail  = 0;
static word       uart1_TxCount = 0;
static byte       uart1_TxFlag  = 0;
byte              uart1_RxBuf[MAX_RX_UART1_BUF_SIZE];
word              uart1_RxHead  = 0;
word              uart1_RxTail  = 0;
word              uart1_RxCount = 0;
#endif
static byte       uartPort = 0;

// LOCAL SUBPROGRAM DECLARATIONS //
void              UART_ShowBuffer(byte* pd, byte eol);
#if UART0_ENABLE
static void       uart0_ISR(void);
static void       uart0_Init(byte baudrate);
static word       uart0_PutChar(char c);
static word       uart0_GetKey(void);
       char       uart0_NoBlockGetkey(void);

       byte       UART0_setBaudRate(byte baudrate);
void              UART0_PutStr(byte* pData, byte cnt);
void              UART0_GetBuf(byte* pData, byte cnt);
void              UART0_RxBufUpdate(byte cnt);
byte              UART0_RxPtrSet(byte c);
void              UART0_RxBufSet(byte c, word len);
byte              UART0_RxBufCmp(byte *pData,byte len);
word              UART0_GetByteCount(void);
void              UART0_DisplayRxPointers(void);
void              UART0_ShowRxBuffer(void);
byte              UART0_TxChk(void);

#endif

#if UART1_ENABLE
static void       uart1_ISR(void);
static void       uart1_Init(byte baudrate);
static word       uart1_PutChar(char c);
static word       uart1_GetKey(void);
       char       uart1_NoBlockGetkey(void);

void              UART1_PutStr(byte* pData, byte cnt);
void              UART1_GetBuf(byte* pData, byte cnt);
void              UART1_RxBufUpdate(byte cnt);
byte              UART1_RxPtrSet(byte c);
void              UART1_RxBufSet(byte c, word len);
byte              UART1_RxBufCmp(byte *pData,byte len);
word              UART1_GetByteCount(void);
void              UART1_DisplayRxPointers(byte crlf);
byte              UART1_TxChk(void);

#endif

// LOCAL SUBPROGRAM BODIES //
#if UART0_ENABLE

// ---------------------------------------------------------------------------------------
// Name    : static void UART0_ISR(void)
// Purpose : UART0 interrupt service routine. For sending out, it puts data
//           from software buffer into hardware serial buffer register (SBUF0).
//           For receiving, it gets data from hardware serial buffer register
//           (SBUF0) and stores into software buffer.
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
static void uart0_ISR(void) interrupt 4
{
      byte  parity = 0;

      if (RI0)
        {
        EA = 0;
        if (uart0_RxCount != MAX_RX_UART0_BUF_SIZE) 
          {
          uart0_RxBuf[uart0_RxHead] = SBUF0;
          uart0_RxCount++;
          uart0_RxHead++;
          uart0_RxHead &= MAX_RX_UART0_MASK;
          }
        RI0 = 0;
        EA = 1;
        } // End of if(RI0) //
      if (TI0)
        {
        EA = 0;
        uart0_TxTail++;
        uart0_TxTail &= MAX_TX_UART0_MASK;
        uart0_TxCount--;
        if (uart0_TxCount > 0)
          {
          SBUF0 = uart0_TxBuf[uart0_TxTail];
          }
        else
          {
          uart0_TxFlag = 0;
          }
        TI0 = 0;
        EA = 1;
        } // End of if(TI0) //
} // End of UART_Int //

//----------------------------------------------------------------------------------------
// name    : UART0_setBaudRate                                                            //
// purpose : read the baud switch and update the COM port settings                        //
// params  : desired baud rate                                                            //
// returns : baudrate setting and system clock setting                                    //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
byte  UART0_setBaudRate(byte baudrate)                                                    //
{
      byte     t;                                                                         //
      byte     sysClk;                                                                    //
      word     xcap;                                                                      //
      
//      printd(" br=%2bx ",baudrate);
//      switch(baudrate)                                                                    // 
//        {
//        case Baud___9600 : printd("  9600"); break;                                    // set the baud rate to   9600 bps
//        case Baud__19200 : printd(" 19200"); break;                                    // set the baud rate to  19200 bps
//        case Baud__28800 : printd(" 28800"); break;                                    // set the baud rate to  28800 bps
//        case Baud__38400 : printd(" 38400"); break;                                    // set the baud rate to  38400 bps
//        case Baud__57600 : printd(" 57600"); break;                                    // set the baud rate to  57600 bps
//        case Baud__76800 : printd(" 76800"); break;                                    // set the baud rate to  76800 bps
//        case Baud__94750 : printd(" 94750"); break;                                    // set the baud rate to  94750 bps
//        default          : printd("115200"); break;                                    // set the baud rate to 115200 bps
//        }

      sysClk = CSREPR & 0xC0;                                                             // get the clock frequency
//      switch(sysClk)                                                                      // find the frequncy to show the user
//        {
//        case SCS_100M : printd("@100MHz"); break;                                       // set the baud rate to  19200 bps
//        case SCS_50M  : printd("@50MHz "); break;                                       // set the baud rate to  94750 bps
//        default       : printd("@25MHz "); break;                                       // set the baud rate to 115200 bps
//        }

//      switch (sysClk)
//        {
//        case SCS_100M :  RLDH = 0xFE;  RLDL = 0xBA;   break;                              // Baud rate = 9600 @ 100MHz.
//        case SCS_50M  :  RLDH = 0xFF;  RLDL = 0x5D;   break;                              // Baud rate = 9600 @ 50MHz.
//        case SCS_25M  :  RLDH = 0xFF;  RLDL = 0xAF;   break;                              // Baud rate = 9600 @ 25MHz.
//        default       :  RLDH = 0xFF;  RLDL = 0xAF;   break;                              // Baud rate = 9600 @ 25MHz.
//        }
      
      t = sysClk | baudrate;                                                            // add the system clock setting to the baudrate
      switch (t)                                                                        // use the baudrate and clock frequency 
        {
        case F_25_B___1200 : xcap = 0xFD75;   break;                                    // Baud rate =   1200  @  25MHz.
        case F_25_B___2400 : xcap = 0xFEBA;   break;                                    // Baud rate =   2400  @  25MHz.
        case F_25_B___4800 : xcap = 0xFF5D;   break;                                    // Baud rate =   4800  @  25MHz.
        case F_25_B___9600 : xcap = 0xFFAF;   break;                                    // Baud rate =   9600  @  25MHz.
        case F_25_B__19200 : xcap = 0xFFD7;   break;                                    // Baud rate =  19200  @  25MHz.
        case F_25_B__28800 : xcap = 0xFFE5;   break;                                    // Baud rate =  28800  @  25MHz.
        case F_25_B__38400 : xcap = 0xFFEC;   break;                                    // Baud rate =  38400  @  25MHz.
        case F_25_B__57600 : xcap = 0xFFF2;   break;                                    // Baud rate =  57600  @  25MHz.
        case F_25_B__76800 : xcap = 0xFFF6;   break;                                    // Baud rate =  76800  @  25MHz.
        case F_25_B__94750 : xcap = 0xFFF8;   break;                                    // Baud rate =  94750  @  25MHz.
        case F_25_B_115200 : xcap = 0xFFF9;   break;                                    // Baud rate = 115200  @  25MHz.

        case F_50_B___1200 : xcap = 0xFAEA;   break;                                    // Baud rate =   1200  @  50MHz.
        case F_50_B___2400 : xcap = 0xFD75;   break;                                    // Baud rate =   2400  @  50MHz.
        case F_50_B___4800 : xcap = 0xFEBA;   break;                                    // Baud rate =   4800  @  50MHz.
        case F_50_B___9600 : xcap = 0xFF5D;   break;                                    // Baud rate =   9600  @  50MHz.
        case F_50_B__19200 : xcap = 0xFFAF;   break;                                    // Baud rate =  19200  @  50MHz.
        case F_50_B__28800 : xcap = 0xFFCA;   break;                                    // Baud rate =  28800  @  50MHz.
        case F_50_B__38400 : xcap = 0xFFD7;   break;                                    // Baud rate =  38400  @  50MHz.
        case F_50_B__57600 : xcap = 0xFFE5;   break;                                    // Baud rate =  57600  @  50MHz.
        case F_50_B__76800 : xcap = 0xFFEC;   break;                                    // Baud rate =  76800  @  50MHz.
        case F_50_B__94750 : xcap = 0xFFF0;   break;                                    // Baud rate =  94750  @  50MHz.
        case F_50_B_115200 : xcap = 0xFFF2;   break;                                    // Baud rate = 115200  @  50MHz.

        case F100_B___1200 : xcap = 0xF5D4;   break;                                    // Baud rate =   1200  @ 100MHz.
        case F100_B___2400 : xcap = 0xFAEA;   break;                                    // Baud rate =   2400  @ 100MHz.
        case F100_B___4800 : xcap = 0xFD75;   break;                                    // Baud rate =   4800  @ 100MHz.
        case F100_B___9600 : xcap = 0xFEBA;   break;                                    // Baud rate =   9600  @ 100MHz.
        case F100_B__19200 : xcap = 0xFF5D;   break;                                    // Baud rate =  19200  @ 100MHz.
        case F100_B__28800 : xcap = 0xFF93;   break;                                    // Baud rate =  28800  @ 100MHz.
        case F100_B__38400 : xcap = 0xFFAF;   break;                                    // Baud rate =  38400  @ 100MHz.
        case F100_B__57600 : xcap = 0xFFCA;   break;                                    // Baud rate =  57600  @ 100MHz.
        case F100_B__76800 : xcap = 0xFFD7;   break;                                    // Baud rate =  76800  @ 100MHz.
        case F100_B__94750 : xcap = 0xFFDF;   break;                                    // Baud rate =  94750  @ 100MHz.
        case F100_B_115200 : xcap = 0xFFE5;   break;                                    // Baud rate = 115200  @ 100MHz.

        default            : xcap = 0xFD75;   break;                                    // Baud rate = DEFAULT 2400 @ 25MHz.
        }

//; SET UP TIMER 2 FOR CLOCKING SERIAL PORT, AT 38400 BPS WITH 11.0592Mhz CRYSTAL
//    Freq(MHz)       100   50    25
//    RLD for 38400 = FFAF  FFD7  FFEB
//            19200 = FF5D  FFAF  FFD7
//             9600 = FEBA  FF5D  FFAF
//             4800 = FD74  FEBA  FF5D
//      printd(" t=%2bx xcap=%4dx",t,xcap);
//      while(UART0_TxChk());                                                               //
//
      sysClk = HIGH_BYTE(xcap);
//      printd(" h=%2bx",sysClk);
      RLDH = sysClk;
      sysClk = LOW_BYTE(xcap);
//      printd(" l=%2bx",sysClk);
      RLDL = sysClk;

      return t;                                                                           // return the software timer count
}

// ---------------------------------------------------------------------------------------
// Name    : static void uart0_Init
// Purpose : Setting operation mode of UART0 and initiating the global values.
// Params  : baud rate
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
static void uart0_Init(byte baudrate)
{
      byte  sysClk  = 0;

      uart0_TxHead  = 0;                                                                  // set the head index to the beginning of the ring buffer
      uart0_TxTail  = 0;                                                                  // set the tail index to the beginning of the ring buffer
      uart0_TxCount = 0;                                                                  // set the buffer count to zero
      uart0_TxFlag  = 0;                                                                  // set the transmit engine started flag
      uart0_RxHead  = 0;                                                                  // set the head index to the beginning of the ring buffer
      uart0_RxTail  = 0;                                                                  // set the tail index to the beginning of the ring buffer
      uart0_RxCount = 0;                                                                  // set the buffer count to zero
      memset(&uart0_TxBuf[0],0,MAX_TX_UART0_BUF_SIZE);                                    // clear the transmit buffer
      memset(&uart0_RxBuf[0],0,MAX_RX_UART0_BUF_SIZE);                                    // clear the receive buffer
// initialize timer1 for standard 8051 uart clock
      PCON          = PCON | SMOD2_ ;                                                     // 
      SM00          = 0;                                                                  // 
      SM01          = 1;                                                                  // Use serial port 0 in mode 1 with 8-bits data.
      REN0          = 1;                                                                  // Enable UART0 receiver.
      TMOD          = 0x20;		                                                      // Use timer 1 in mode 2, 8-bit counter with auto-reload.

      UART0_setBaudRate(baudrate);                                                        // set the baudrate

      TH2           = RLDH;                                                               // load the registers with the reload values
      TL2           = RLDL;                                                               //
      ES0           = 1;                                                                  // enable serial port Interrupt request
      T2CON         = 0X30;                                                               // set uart0 to use timer 2 for rclk and tclk
      TR2           = 1;                                                                  // run timer 2
} // End of uart0_Init //

// ---------------------------------------------------------------------------------------
// Name    : static word uart0_PutChar(char c)
// Purpose : UART0 output function. This function puts one byte data into the
//           software character buffer.
// Params  : c - one byte character.
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
word uart0_PutChar(char c)
{
      word count = 0;
        do
          {
          EA = 0;
          count = uart0_TxCount; 
          EA = 1;
          } 
        while (count == MAX_TX_UART0_BUF_SIZE);
        uart0_TxBuf[uart0_TxHead] = c;
        EA = 0;
        uart0_TxCount++;
        EA = 1;
        uart0_TxHead++;
        uart0_TxHead &= MAX_TX_UART0_MASK;
      if (!uart0_TxFlag)
        {
        uart0_TxFlag = 1;
        SBUF0 = uart0_TxBuf[uart0_TxTail];
        }
      return c;
}

// ---------------------------------------------------------------------------------------
// Name    : static char uart0_GetKey(void)
// Purpose : UART0 input function. This function replies one byte data from the
//           software character buffer.
// Params  : none
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
word uart0_GetKey (void)
{
      byte c;

      if (uart0_RxCount==0) return 0x100;

      EA = 0;
      uart0_RxCount--;
      EA = 1;
      c = uart0_RxBuf[uart0_RxTail];
      uart0_RxTail++;
      uart0_RxTail &= MAX_RX_UART0_MASK;
      return c;
}

// ---------------------------------------------------------------------------------------
// Name    : UART0_TxChk
// Purpose : UART0 input function. This function replies one byte data from the
//           software character buffer. But it only check the buffer one time.
//           If no data, it will reply a FALSE condition.
// Params  : none
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
byte  UART0_TxChk(void)
{
   return(uart0_TxFlag);
}

// ---------------------------------------------------------------------------------------
// Name    : void UART0_PutStr(byte* pData, byte cnt)
// Purpose : copy the string data to the local UART0 buffer from the
//           source buffer.
// Params  : pointer to the source buffer, and the number of bytes to copy
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART0_PutStr(byte* pData, byte cnt)
{
      word count = 0;
      word offset;

//printd("stx msg cnt = %bu\n\r",cnt);

      do
        {
        EA = 0;
        count = uart0_TxCount; 
        EA = 1;
        } 
      while ((count + cnt) == MAX_TX_UART0_BUF_SIZE);
      if((uart0_TxHead + cnt) > MAX_TX_UART0_BUF_SIZE)
        {
        offset = MAX_TX_UART0_BUF_SIZE - uart0_TxHead;
        memmove(&uart0_TxBuf[uart0_TxHead], pData, offset);
        memmove(&uart0_TxBuf[0], pData+offset, cnt - offset);
        }
      else
        memmove(&uart0_TxBuf[uart0_TxHead], pData, cnt);
      EA = 0;
      uart0_TxCount += cnt;
      EA = 1;
      uart0_TxHead += cnt;
      uart0_TxHead &= MAX_TX_UART0_MASK;
      if (!uart0_TxFlag)
        {
        uart0_TxFlag = 1;
        SBUF0 = uart0_TxBuf[uart0_TxTail];
//      HalfDuplex = 1;
        }
}

// ---------------------------------------------------------------------------------------
// Name    : void UART0_GetBuf(byte* pData, byte cnt)
// Purpose : copy the recieved data from the local UART0 buffer to the
//           destination buffer.
// Params  : pointer to the destination buffer, and the number of bytes to copy
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART0_GetBuf(byte* pData, byte cnt)
{
      word offset;
      
      if((uart0_RxTail + cnt) > MAX_RX_UART0_BUF_SIZE)
        {
        offset = MAX_RX_UART0_BUF_SIZE - uart0_RxTail;
        memmove(pData, &uart0_RxBuf[uart0_RxTail], offset);
        memmove(pData+offset, &uart0_RxBuf[0], cnt - offset);
        }
      else
        memmove(pData, &uart0_RxBuf[uart0_RxTail], cnt);
}

word UA0_GetCount(void)
{
   return(uart0_RxCount);
}

word UA1_GetCount(void)
{
   return(uart1_RxCount);
}

// ---------------------------------------------------------------------------------------
// Name    : void UART0_RxBufUpdate(byte cnt)
// Purpose : update the received data count and the tail index
// Params  : the number of bytes that were copied
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART0_RxBufUpdate(byte cnt)
{
      uart0_RxTail = uart0_RxTail + cnt;
      uart0_RxTail &= MAX_RX_UART0_MASK;
      EA = 0;
      uart0_RxCount = uart0_RxCount - cnt;      
      EA = 1;
}

// ---------------------------------------------------------------------------------------
// Name    : void UART0_RxPtrSet(byte c)
// Purpose : find the first occurence of the character in c
// Params  : the character to search for
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
byte UART0_RxPtrSet(byte c)
{
      word length;
      byte *pData1, *pData2;
      
      length = MAX_TX_UART0_BUF_SIZE - uart0_RxTail;
      pData1 = memchr(&uart0_RxBuf[uart0_RxTail], c, length);
      pData2 = &uart0_RxBuf[MAX_TX_UART0_BUF_SIZE];
      length = *(pData1) - *(pData2);
      uart0_RxTail = length;
      return uart0_RxBuf[uart0_RxTail];
}

// ---------------------------------------------------------------------------------------
// Name    : void UART0_RxBufSet(byte c)
// Purpose : find the first occurence of the character in c
// Params  : the character to search for
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART0_RxBufSet(byte c, word len)
{
      word offset;
      
      if((uart0_RxTail + len) > MAX_RX_UART0_BUF_SIZE)
        {
        offset = MAX_RX_UART0_BUF_SIZE - uart0_RxTail;
        memset(&uart0_RxBuf[uart0_RxTail], c, offset);
        memset(&uart0_RxBuf[0], c, len - offset);
        }
      else
        memset(&uart0_RxBuf[uart0_RxTail], c, len);
//    memset(&uart0_RxBuf[uart0_RxTail], c, len);
}

// ---------------------------------------------------------------------------------------
// Name    : byte UART0_RxBufCmp(byte c)
// Purpose : compary the two arrays
// Params  : pointer to the array to compare, number of bytes to compare
// Returns : 0 if equal !0 if not equal
// Note    : none
// ---------------------------------------------------------------------------------------
byte UART0_RxBufCmp(byte *pData,byte len)
{
      word offset;
      byte result;
      
      if((uart0_RxTail + len) > MAX_RX_UART0_BUF_SIZE)
        {
        offset = MAX_RX_UART0_BUF_SIZE - uart0_RxTail;
        result = memcmp(&uart0_RxBuf[uart0_RxTail], pData, offset);
        result |= memcmp(&uart0_RxBuf[0], pData+offset, len - offset);
        }
      else
        result = memcmp(&uart0_RxBuf[uart0_RxTail], pData, len);

      return result;//memcmp(&uart0_RxBuf[uart0_RxTail], pData, len);
}

// ---------------------------------------------------------------------------------------
// Name    : UART0_GetByteCount
// Purpose : return the number of bytes waiting in the buffer.
// Params  : none
// Returns : number of characters in the buffer
// Note    : none
// ---------------------------------------------------------------------------------------
word UART0_GetByteCount(void)
{
      return uart0_RxCount;
}

// ---------------------------------------------------------------------------------------
// Name    : uart0_DisplayRxPointers
// Purpose : display in the debug window the ring buffer pointers to the user.
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART0_DisplayRxPointers(void)
{
      printd("--RxHead %du, RxTail %du, RxCnt %du--\n\r",uart0_RxHead ,uart0_RxTail ,uart0_RxCount);
}

// ---------------------------------------------------------------------------------------
// Name    : uart0_ShowRxBuffer
// Purpose : display in the debug window the current contents of the uart ring buffer
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART0_ShowRxBuffer(void)
{
      word i;

      for(i = 0;i < MAX_RX_UART0_BUF_SIZE;i++)
        {
        if((i % 16) == 0) printd("\n\r%04dx :",i);
        printd(" %02bx",uart0_RxBuf[i]);
        }
      printd("\n\r");
}

// ---------------------------------------------------------------------------------------
// Name    : static char uart0_NoBlockGetkey(void)
// Purpose : UART0 input function. This function replies one byte data from the
//           software character buffer. But it only check the buffer one time.
//           If no data, it will reply a FALSE condition.
// Params  : none
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
char uart0_NoBlockGetkey (void)
{
    char c = 0;

    if (uart0_RxCount !=0 )
      {
      EA   = 0;
      uart0_RxCount--;
      EA   = 1;
      c    = uart0_RxBuf[uart0_RxTail];
      uart0_RxTail++;
      uart0_RxTail &= MAX_RX_UART0_MASK;
      return c;
      }
      return FALSE;
}
#endif

#if UART1_ENABLE
////////////////////////////////////////////////////////////////////////////////
// ---------------------------------------------------------------------------------------
// Name    : static void UART1_ISR(void)
// Purpose : UART1 interrupt service routine. For sending out, it puts data
//           from software buffer into hardware serial buffer register (SBUF1).
//           For receiving, it gets data from hardware serial buffer register
//           (SBUF1) and stores into software buffer.
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
static void uart1_ISR(void) interrupt 6
{
      if (RI1)
        {
        EA              = 0;
        if (uart1_RxCount != MAX_RX_UART1_BUF_SIZE) 
          {
          uart1_RxBuf[uart1_RxHead] = SBUF1;
          uart1_RxCount++;
          uart1_RxHead++;
          uart1_RxHead &= MAX_RX_UART1_MASK;
          }
        RI1             = 0;
        EA              = 1;
        } // End of if(RI0) //

      if (TI1)
        {
        EA              = 0;
        uart1_TxTail++;
        uart1_TxTail   &= MAX_TX_UART1_MASK;
        uart1_TxCount--;
        if (uart1_TxCount > 0)
          {
          SBUF1         = uart1_TxBuf[uart1_TxTail];                  
          }
        else
          uart1_TxFlag  = 0;
        TI1             = 0;
        EA              = 1;
        } // End of if(TI0) //
}

// ---------------------------------------------------------------------------------------
// Name    : uart1_Init
// Purpose : Setting operation mode of UART1 and initiating the global values.
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
static void uart1_Init(byte baudrate)
{
      byte  sysClk  = 0;

      uart1_TxHead  = 0;
      uart1_TxTail  = 0;
      uart1_TxCount = 0;
      uart1_TxFlag  = 0;
      uart1_RxHead  = 0;
      uart1_RxTail  = 0;
      uart1_RxCount = 0;
      memset(&uart1_TxBuf[0],0,MAX_TX_UART1_BUF_SIZE);
      memset(&uart1_RxBuf[0],0,MAX_RX_UART1_BUF_SIZE);
      baudrate      = baudrate;

       // Initialize TIMER1 for standard 8051 UART clock
      PCON       = PCON;//| SMOD1_;                                                       // Enable BaudRate doubler.
      SM10       = 0;                   // ckcon.4 1=4,0=12,0 is default                  // Use serial port 1 in mode 1 with 8-bits data.
      SM11       = 1;                   // ckcon.4 1=4,0=12,0 is default                  // Use serial port 1 in mode 1 with 8-bits data.
      REN1       = 1;                   // 100MHz / 4  = 25MHz   = fsysclk                // Enable UART1 receiver.
      TMOD       = 0x20;                // 100MHz / 12 = 8.33MHz = fsysclk                // Use timer 1 in mode 2, 8-bit counter with auto-reload.

      sysClk     = CSREPR & 0xC0;       // baud = T1overflow/(32/2^(smod))
      switch (sysClk)                   // t1overflow = fsysclk / (32*(256 - th1))
        {
        case SCS_100M :            TH1  = 0xE5;   break;      // E4h = 228                // Baud rate =  9600 @ 100MHz.
        case SCS_50M  :            TH1  = 0xF2;   break;      // F2h = 242                // Baud rate =  9600 @ 50MHz.
        case SCS_25M  :            TH1  = 0xF9;   break;      // F9h = 249                // Baud rate =  9600 @ 25MHz.
        default       :            TH1  = 0xF9;   break;                                  // Baud rate =  9600 @ 25MHz.
        }
      ES1        = 1;                                                                     // Enable serial port Interrupt request
      TR1        = 1;                                                                     // Run Timer 1
      TI1        = 0;                                                                     // Clear transmitted flag
}

// ---------------------------------------------------------------------------------------
// Name    : static word uart1_PutChar(char c)
// Purpose : UART1 output function. This function puts one byte data into the
//           software character buffer.
// Params  : c - one byte character.
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
word uart1_PutChar(char c)
{
      word count                = 0;

      do
        {
        EA                      = 0;
        count                   = uart1_TxCount; 
        EA                      = 1;
        }
      while (count == MAX_TX_UART1_BUF_SIZE);
      uart1_TxBuf[uart1_TxHead] = c;
      EA                        = 0;
      uart1_TxCount++;
      EA                        = 1;
      uart1_TxHead++;
      uart1_TxHead             &= MAX_TX_UART1_MASK;
      if (!uart1_TxFlag)
        {
        uart1_TxFlag            = 1;
        SBUF1                   = uart1_TxBuf[uart1_TxTail];
        }
      return c;
}

// ---------------------------------------------------------------------------------------
// Name    : static word uart1_GetKey(void)
// Purpose : UART1 input function. This function replies one byte data from the
//           software character buffer.
// Params  : none
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
word uart1_GetKey (void)
{
      byte c;

      if (uart1_RxCount==0) return 0x100;
      EA            = 0;
      uart1_RxCount--;
      EA            = 1;
      c             = uart1_RxBuf[uart1_RxTail];
      uart1_RxTail++;
      uart1_RxTail &= MAX_RX_UART1_MASK;
      return c;
}

// ---------------------------------------------------------------------------------------
// Name    : void UART1_PutStr(byte* pData, byte cnt)
// Purpose : copy the string data to the local UART1 buffer from the
//           source buffer.
// Params  : pointer to the source buffer, and the number of bytes to copy
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART1_PutStr(byte* pData, byte cnt)
{
      word count = 0;
      word offset;

      do
        {
        EA = 0;
        count = uart1_TxCount; 
        EA = 1;
        } 
      while ((count + cnt) == MAX_TX_UART1_BUF_SIZE);
      if((uart1_TxHead + cnt) > MAX_TX_UART1_BUF_SIZE)
        {
        offset = MAX_TX_UART1_BUF_SIZE - uart1_TxHead;
        memmove(&uart1_TxBuf[uart1_TxHead], pData, offset);
        memmove(&uart1_TxBuf[0], pData+offset, cnt - offset);
        }
      else
        memmove(&uart1_TxBuf[uart1_TxHead], pData, cnt);
      EA = 0;
      uart1_TxCount += cnt;
      EA = 1;
      uart1_TxHead += cnt;
      uart1_TxHead &= MAX_TX_UART1_MASK;
      if (!uart1_TxFlag)
        {
        uart1_TxFlag = 1;
        SBUF1 = uart1_TxBuf[uart1_TxTail];
//      HalfDuplex = 1;
        }
}

// ---------------------------------------------------------------------------------------
// Name    : void UART1_GetBuf(byte* pData, byte cnt)
// Purpose : copy the recieved data from the local UART1 buffer to the
//           destination buffer.
// Params  : pointer to the destination buffer, and the number of bytes to copy
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART1_GetBuf(byte* pData, byte cnt)
{
      word offset;
      
      if((uart1_RxTail + cnt) > MAX_RX_UART1_BUF_SIZE)
        {
        offset = MAX_RX_UART1_BUF_SIZE - uart1_RxTail;
        memmove(pData, &uart1_RxBuf[uart1_RxTail], offset);
        memmove(pData+offset, &uart1_RxBuf[0], cnt - offset);
        }
      else
        memmove(pData, &uart1_RxBuf[uart1_RxTail], cnt);
}

// ---------------------------------------------------------------------------------------
// Name    : void UART1_RxBufUpdate(byte cnt)
// Purpose : update the received data count and the tail index
// Params  : the number of bytes that were copied
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART1_RxBufUpdate(byte cnt)
{
//      printd("pre  update %02bx %04dx %04dx\n\r",cnt, uart1_RxTail, uart1_RxCount);
      uart1_RxTail = uart1_RxTail + cnt;
      uart1_RxTail &= MAX_RX_UART1_MASK;
      EA = 0;
      uart1_RxCount = uart1_RxCount - cnt;      
      EA = 1;
//      printd("post update %02bx %04dx %04dx\n\r",cnt, uart1_RxTail, uart1_RxCount);
}

// ---------------------------------------------------------------------------------------
// Name    : void UART1_RxPtrSet(byte c)
// Purpose : find the first occurence of the character in c
// Params  : the character to search for
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
byte UART1_RxPtrSet(byte c)
{
      word length;
      byte *pData1, *pData2;
      
      length = MAX_TX_UART1_BUF_SIZE - uart1_RxTail;
      pData1 = memchr(&uart1_RxBuf[uart1_RxTail], c, length);
      pData2 = &uart1_RxBuf[MAX_TX_UART1_BUF_SIZE];
      length = *(pData1) - *(pData2);
      uart1_RxTail = length;
      return uart1_RxBuf[uart1_RxTail];
}

// ---------------------------------------------------------------------------------------
// Name    : void UART1_RxBufSet(byte c)
// Purpose : find the first occurence of the character in c
// Params  : the character to search for
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART1_RxBufSet(byte c, word len)
{
      word offset;
      
      if((uart1_RxTail + len) > MAX_RX_UART1_BUF_SIZE)
        {
        offset = MAX_RX_UART1_BUF_SIZE - uart1_RxTail;
        memset(&uart1_RxBuf[uart1_RxTail], c, offset);
        memset(&uart1_RxBuf[0], c, len - offset);
        }
      else
        memset(&uart1_RxBuf[uart1_RxTail], c, len);
//    memset(&uart1_RxBuf[uart1_RxTail], c, len);
}

// ---------------------------------------------------------------------------------------
// Name    : byte UART1_RxBufCmp(byte c)
// Purpose : compary the two arrays
// Params  : pointer to the array to compare, number of bytes to compare
// Returns : 0 if equal !0 if not equal
// Note    : none
// ---------------------------------------------------------------------------------------
byte UART1_RxBufCmp(byte *pData,byte len)
{
      word offset;
      byte result;
      
      if((uart1_RxTail + len) > MAX_RX_UART1_BUF_SIZE)
        {
        offset = MAX_RX_UART1_BUF_SIZE - uart1_RxTail;
        result = memcmp(&uart1_RxBuf[uart1_RxTail], pData, offset);
        result |= memcmp(&uart1_RxBuf[0], pData+offset, len - offset);
        }
      else
        result = memcmp(&uart1_RxBuf[uart1_RxTail], pData, len);

      return result;//memcmp(&uart1_RxBuf[uart1_RxTail], pData, len);
}

// ---------------------------------------------------------------------------------------
// Name    : UART1_GetByteCount
// Purpose : return the number of bytes waiting in the buffer.
// Params  : none
// Returns : number of characters in the buffer
// Note    : none
// ---------------------------------------------------------------------------------------
word UART1_GetByteCount(void)
{
      return uart1_RxCount;
}

// ---------------------------------------------------------------------------------------
// Name    : uart1_DisplayRxPointers
// Purpose : display in the debug window the ring buffer pointers to the user.
// Params  : byte crlf
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART1_DisplayRxPointers(byte crlf)
{
      printd("--RxHead %04dx, RxTail %04dx, RxCnt %du-- ",uart1_RxHead ,uart1_RxTail ,uart1_RxCount);
      if(crlf == 0)printd("\n\r");
}

// ---------------------------------------------------------------------------------------
// Name    : uart_ShowBuffer
// Purpose : display in the debug window the current contents of the uart ring buffer
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void UART_ShowBuffer(byte* pd, byte eol)
{
      word i;

      for(i = 0;i < eol;i++)
        {
        if((i % 16) == 0) printd("\n\r%04dx :",i);
        printd(" %02bx",*(pd+i));
        }
      printd("\n\r************************************************************************\n\r");
}

// ---------------------------------------------------------------------------------------
// Name    : static char uart1_NoBlockGetkey(void)
// Purpose : UART1 input function. This function replies one byte data from the
//           software character buffer. But it only check the buffer one time.
//           If no data, it will reply a FALSE condition.
// Params  : none
// Returns : c - one byte character.
// Note    : none
// ---------------------------------------------------------------------------------------
char uart1_NoBlockGetkey (void)
{
    char c = 0;

      if (uart1_RxCount !=0 )
        {
        EA = 0;
        uart1_RxCount--;
        EA = 1;
        c = uart1_RxBuf[uart1_RxTail];
        uart1_RxTail++;
        uart1_RxTail &= MAX_RX_UART1_MASK;
        return c;
        }
      else
        {
        return FALSE;
        }
}

// ---------------------------------------------------------------------------------------
// Name    : byte UART1_TxChk(void)
// Purpose : Initiating the UART1 to work in power management mode.
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
byte  UART1_TxChk(void)
{
   return(uart1_TxFlag);
}

#endif

// EXPORTED SUBPROGRAM BODIES //

// ---------------------------------------------------------------------------------------
// Name    : uart_Init
// Purpose : UART initial function. It will call a real initial function
//           corresponding to the used UART port.
// Params  : which port, & baudrate
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
void uart_Init(byte port, baudrate)
{
      if (port == 0)
        {
        uart0_Init(baudrate);
        }
      else if (port == 1)
        {
        uart1_Init(baudrate);
        }
}

// ---------------------------------------------------------------------------------------
// Name    : bool UART_ParityChk(char checkByte)
// Purpose : UART parity checked function in one byte transfer.
// Params  : checkByte - one byte character.
// Returns : TRUE - odd parity ; FALSE - even parity.
// Note    : none
// ---------------------------------------------------------------------------------------
bool UART_ParityChk(char checkByte)
{
      byte  oneNum = 0;
      word i;

      oneNum = 0;
      for (i=0 ; i<=7 ; i++)
      {
            if (checkByte & (BIT0<<i))
            {
                  oneNum ++;
            }
      }
      if ((oneNum % 2) == 0)
            return FALSE;                                                                 // if '1' number is even, return 0
      
      return TRUE;                                                                        // if '1' number is odd, return 1
}

// ---------------------------------------------------------------------------------------
// Name    : bool UART_SetPort(byte portNum)
// Purpose : Setting which UART port will be used.
// Params  : portNum - uart port number (0~2).
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
bool UART_SetPort(byte portNum)
{
      uartPort = portNum & 0x03;
      uart_Init(0,0);
      return TRUE;
}

// ---------------------------------------------------------------------------------------
// Name    : void PMM_Uart0Init(void)
// Purpose : Initiating the UART0 to work in power management mode.
// Params  : none
// Returns : none
// Note    : none
// ---------------------------------------------------------------------------------------
#if UART_PMM_ENABLE
void PMM_Uart0Init(void)
{
      word i;

      uart0_TxHead = 0;
      uart0_TxTail = 0;
      uart0_TxCount = 0;
      uart0_TxFlag = 0;
      uart0_RxHead = 0;
      uart0_RxTail = 0;
      uart0_RxCount = 0;
      memset(&uart0_TxBuf[0],0,MAX_TX_UART0_BUF_SIZE);
      memset(&uart0_RxBuf[0],0,MAX_RX_UART0_BUF_SIZE);
      // Initialize TIMER1 for standard 8051 UART clock
      PCON  = BIT7;                                                                       // Enable BaudRate doubler.
      SM01  = 1;                                                                          // Use serial port 0 in mode 1 with 8-bits data.
      REN0  = 1;                                                                          // Enable UART0 receiver.
      TMOD  = 0x20;                                                                       // Use timer 1 in mode 2, 8-bit counter with auto-reload.

      TH1   = 0xFE;                                                                       // Baud rate = 1200 @ 25MHz.

      ES0   = 1;                                                                          // Enable serial port Interrupt request
      TR1   = 1;                                                                          // Run Timer 1
      TI0   = 0;
}
#endif

// End of uart.c //