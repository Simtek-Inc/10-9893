//----------------------------------------------------------------------------------------
//     Copyright (c) 2005   ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//----------------------------------------------------------------------------------------
// Module Name : hsuart.c
// Purpose     : The High Speed UART module driver. It manages the character
//               buffer and handles the ISR.
// Author      : Robin Lee
// Date        : 2006-01-09
// Notes       :
// $Log: hsuart.c,v $
// Revision 1.20  2006/07/25 03:41:23  robin6633
// 1.Add Xon/Xoff counter for debugging.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.19  2006/07/24 06:40:51  robin6633
// 1.Fixed the XON/XOFF ASCII is put into hardware transmitting register driectly.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.18  2006/07/21 11:24:55  robin6633
// 1.Add flow control check in hsurFlowOn==1.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.17  2006/07/19 08:51:24  robin6633
// 1.Fixed Hardware flow control function.
// 2.Extern a function HSUR_GetLineStatus() to get UR2_LSR.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.16  2006/07/18 08:03:59  robin6633
// 1.Fixed Tx flow control in software flow control mode.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.15  2006/06/30 06:17:01  robin6633
// 1.Fixed software flow control in first initial.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.14  2006/06/27 10:45:24  robin6633
// 1.Add flow control for transmitting.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.13  2006/06/19 14:20:57  robin6633
// 1.Protect transmit counter and pointer.
// 2.Force transmit more data if transmit fifo empty.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.12  2006/06/11 02:18:49  robin6633
// 1.Add Xon/Xoff.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.11  2006/06/05 06:21:43  robin6633
// 1.Fixed function declaration.
//    U8_T HSUR_RxBufFlowControl(void) -> void HSUR_RxBufFlowControl(void)
// 2.Fixed hsur_ReadLsr() & hsur_ReadMsr().
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.10  2006/06/05 01:30:44  robin6633
// 1.Fixed software flow control and prevented the hardware FIFO overrun.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.9  2006/05/24 05:40:34  robin6633
// 1.Add flow control water mark.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.8  2006/05/23 11:07:51  robin6633
// 1.Enable flow control code.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.7  2006/05/22 13:23:42  robin6633
// 1.Fixed buffer overwrite.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.6  2006/05/22 07:39:28  robin6633
// 1.Disable flow control.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.5  2006/05/19 06:37:13  robin6633
// 1.Fixed the baudrate setup for default configuration.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.4  2006/05/19 03:48:37  robin6633
// 1.Add Xon/Xoff flow control.
// 2.Expand an external function HSUR_ChangeBaud(U8_T baud).
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.3  2006/05/18 09:57:55  Louis
// no message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.2  2006/05/18 02:27:56  robin6633
// 1.Removed printf in UART2.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.1  2006/05/15 03:06:52  robin6633
// no message
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.3  2006/05/03 02:42:30  robin6633
// Changed the the function name HSUR_GetBufCount()
// to HSUR_GetRxBufCount() .
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.2  2006/05/02 01:44:07  robin6633
// Add an expanding function to get the counter value of UART2 software buffer.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Revision 1.1  2006/04/07 11:38:18  robin6633
// no message
//----------------------------------------------------------------------------------------

// INCLUDE FILE DECLARATIONS //
#include    "reg80390.h"
#include    "types.h"
#include    "uart.h"
#include    "mstimer.h"
#include    "printd.h"
#include    "hsuart.h"
#include    "string.h"

// STATIC VARIABLE DECLARATIONS //
static U8_T       hsurRxBuffer[UR2_MAX_RX_SIZE];
static U8_T       hsurTxBuffer[UR2_MAX_TX_SIZE];
static U16_T      hsurRxBufNum = 0;
static U16_T      hsurTxBufNum = 0;
static U8_T       hsurRxTrigLvl = 0;
static U16_T      hsurRxCount = 0;
static S16_T      hsurTxCount = 0;
static U16_T      hsurGetPtr = 0;
static U16_T      hsurPutPtr = 0;
static U8_T       hsurFlowCtrl = 0;
static U8_T       hsurFlowOn = 0;
static U8_T       hsurTxTransmit = 0;
static U8_T       hsurTxFlag = 0;
static U8_T       hsurTxState = 0;
static U16_T      hsurErrorCount = 0;
static U32_T      hsurErrTimeStart = 0;
static U32_T      hsurErrTimeStop = 0;
static U8_T       hsurErrBlocking = 0;
static U16_T      hsurBaudRateDivisor = 0;
static U8_T       hsurLineStatusValue = 0;
static U8_T       hsurModemStatusValue = 0;     
static U8_T       hsurLineControlValue = 0;
static U8_T       hsurModemControlValue = 0;
static U8_T       hsurFifoControlValue = 0;
static U8_T       hsurAppFlowOn = 0;
static U8_T       hsurFlowCtrlXoff = FALSE;
static U8_T       hsurFlowCtrlXon = FALSE;
static U8_T       hsurFlowCtrlHwCTSon = FALSE;
static U8_T       hsurInitIerRec = 0;

#if DEBUG_DISPLAY_DIAG
U32_T hsurXonCounter = 0;
U32_T hsurXoffCounter = 0;
#endif
// LOCAL SUBPROGRAM DECLARATIONS //
static void       hsur_ReadLsr(void);
static void       hsur_RcvrTrig(void);
static void       hsur_Rcvr(void);
static void       hsur_Xmit(void);
static void       hsur_ReadMsr(void);

// LOCAL SUBPROGRAM BODIES //

//----------------------------------------------------------------------------------------
// Name    : hsur_ReadLsr()
// Purpose : Read Line Status Register and display.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
static void hsur_ReadLsr(void)
{
      hsurLineStatusValue = UR2_LSR;

      if (hsurLineStatusValue & UR2_OE)
        {           hsurErrorCount ++;      }// Overrun Error
      if (hsurLineStatusValue & UR2_PE)
        {           hsurErrorCount ++;      }// Parity Error
      if (hsurLineStatusValue & UR2_FE)
        {           hsurErrorCount ++;      }// Framing Error
      if (hsurLineStatusValue & UR2_BI)
        {           hsurErrorCount ++;      }// Break Interrupt Occured
      if (hsurLineStatusValue & UR2_FRAME_ERR)
        {           hsurErrorCount ++;      }// Mixing Error
      if (hsurErrorCount)
        {
        UR2_FCR |= UR2_RXFIFO_RST;
        if (hsurErrorCount >=30)
          {
          hsurErrTimeStop = SWTIMER_Tick();
          if (((hsurErrTimeStop - hsurErrTimeStart) * SWTIMER_INTERVAL) <= 2000)
            {
            UR2_IER = 0;
            hsurErrBlocking = 1;
            }
          else
            {
            hsurErrBlocking = 0;
            hsurErrTimeStart = 0;
            hsurErrorCount = 0;
            }
          }
        else// if (hsurErrorCount < 30)
          {
          U8_T trash;

          while (UR2_LSR & UR2_DR)
            {                       trash = UR2_RBR;                  }
          if ((hsurErrTimeStart == 0) && (hsurErrorCount < 5))
            {                        hsurErrTimeStart = SWTIMER_Tick();                  }
          else
            {
            hsurErrTimeStop = SWTIMER_Tick();
            if (((hsurErrTimeStop - hsurErrTimeStart) * SWTIMER_INTERVAL) > 2000)
              {
              hsurErrBlocking = 0;
              hsurErrTimeStart = 0;
              hsurErrorCount = 0;
              }
            }
          }
      }
}

//----------------------------------------------------------------------------------------
// Name    : void hsur_RcvrTrig()
// Purpose : Get data and put into the receiver buffer continuously until trigger bytes
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
static void hsur_RcvrTrig(void)
{
      U8_T  c, trash;
      U16_T i;

      if (hsurRxCount >= UR2_MAX_RX_SIZE - hsurRxTrigLvl) // buffer full
      {
            for (i=0 ; i<hsurRxTrigLvl ; i++)
            {
                  trash = UR2_RBR;
            }
            return;
      }

      if (hsurFlowCtrl != UR2_FLOW_CTRL_X)
      {
            for (i=0 ; i<hsurRxTrigLvl ; i++)
            {
                  hsurRxBuffer[hsurRxBufNum] = UR2_RBR;
                  hsurRxBufNum ++;
                  hsurRxCount ++;
                  if (hsurRxBufNum == UR2_MAX_RX_SIZE)
                        hsurRxBufNum = 0; 
            }
      }
      else
      {
            for (i=0 ; i<hsurRxTrigLvl ; i++)
            {
                  c = UR2_RBR;
                  if (c != ASCII_XOFF && c != ASCII_XON)
                  {
                        hsurRxBuffer[hsurRxBufNum] = c;
                        hsurRxBufNum ++;
                        hsurRxCount ++;
                        if (hsurRxBufNum == UR2_MAX_RX_SIZE)
                              hsurRxBufNum = 0;
                  }
                  else
                  {
                        if (c == ASCII_XOFF)
                              hsurFlowCtrlXoff = TRUE;
                        else if (c == ASCII_XON)
                              hsurFlowCtrlXon = TRUE;
                  }
            }
      }

      if (hsurFlowCtrl != UR2_FLOW_CTRL_NO)
      {
            HSUR_RxBufFlowControl();

            if (hsurFlowOn == FALSE)
            {
                  if (hsurRxCount >= UR2_FLOW_HI_WATERMARK) // check software buffer status
                  {
                        hsurFlowOn = TRUE;
            }     
            }     
      }
}

//----------------------------------------------------------------------------------------
// Name    : void hsur_Rcvr()
// Purpose : Receiving the byte data from hardware register into software buffer.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
static void hsur_Rcvr(void)
{
      U8_T trash, lineStatus = 0;

      if (hsurFlowCtrl != UR2_FLOW_CTRL_X)
        {
        while (1)
          {
          lineStatus = UR2_LSR;
          hsurLineStatusValue = lineStatus;
          if (lineStatus & UR2_DR)
            {
            if (hsurRxCount >= UR2_MAX_RX_SIZE)
              {
              trash = UR2_RBR;
              break;
              }
            hsurRxBuffer[hsurRxBufNum] = UR2_RBR;
            hsurRxBufNum ++;
            hsurRxCount ++;
            if (hsurRxBufNum == UR2_MAX_RX_SIZE)
              hsurRxBufNum = 0;
            }
          else                    
            break;
          }           
        }
      else 
        {
        U8_T  c = 0;

        while (1)
          {
          lineStatus = UR2_LSR;
          hsurLineStatusValue = lineStatus;
      
          if (lineStatus & UR2_DR)
            {
            c = UR2_RBR;

            if (c != ASCII_XOFF && c != ASCII_XON)
              {
              if (hsurRxCount >= UR2_MAX_RX_SIZE)
                {
                trash = UR2_RBR;
                break;
                }
              hsurRxBuffer[hsurRxBufNum] = c;
              hsurRxBufNum ++;
              hsurRxCount ++;
              if (hsurRxBufNum == UR2_MAX_RX_SIZE)
                hsurRxBufNum = 0;
              }
            else
              {
              if (c == ASCII_XOFF)
                hsurFlowCtrlXoff = TRUE;
              else if (c == ASCII_XON)
                hsurFlowCtrlXon = TRUE;
              }
            }
           else
                        break;
            }
      }

      if (hsurFlowCtrl != UR2_FLOW_CTRL_NO)
            HSUR_RxBufFlowControl();
}

//----------------------------------------------------------------------------------------
// Name    : void hsur_Xmit()
// Purpose : Transmitting the byte data from software buffer into hardware register.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
static void hsur_Xmit(void)
{
      U8_T  i = 0;

      for (i=0 ; i<UR2_TX_BUF_LEVEL ; i++)
      {
            if (hsurTxCount > 0)
            {
                  UR2_THR = hsurTxBuffer[hsurTxBufNum];
                  hsurTxBufNum ++;
                  hsurTxCount --;

                  if (hsurTxBufNum == UR2_MAX_TX_SIZE)
                        hsurTxBufNum = 0; 

                  if (hsurFlowCtrl != UR2_FLOW_CTRL_NO)
                  {
                        HSUR_TxBufFlowControl();
                  }
            
                  if (hsurTxCount <= 0)
                  {           
                        UR2_IER &= ~UR2_THRI_ENB;
                        return;
                  }
            }
            else
            {
                  UR2_IER &= ~UR2_THRI_ENB;
                  return;
            }
      }
}

//----------------------------------------------------------------------------------------
// Name    : void hsur_ReadMsr()
// Purpose : Reading the modem status register.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
static void hsur_ReadMsr(void)
{
      hsurModemStatusValue = UR2_MSR;

      if (hsurModemStatusValue & UR2_DCTS)
      {
            if (hsurModemStatusValue & UR2_CTS)
            {
                  //HSUR_SetRTS();
                  hsurFlowCtrlHwCTSon = TRUE;
            }
            else
            {
                  //HSUR_ClearRTS();
                  hsurFlowCtrlHwCTSon = FALSE;
            }
      }

      if (hsurModemStatusValue & UR2_DDSR)
      {
            if (hsurModemStatusValue & UR2_DSR)
            {
                  HSUR_SetDTR();
            }
            else
            {
                  HSUR_ClearDTR();
            }
      }

      if (hsurModemStatusValue & UR2_TERI)
      {
            if (hsurModemStatusValue & UR2_RI)
            {
                  UR2_MCR |= UR2_OUT1;
                  hsurModemControlValue |= UR2_OUT1;
            }
            else
            {
                  UR2_MCR &= ~UR2_OUT1;
                  hsurModemControlValue &= ~UR2_OUT1;
            }
      }

      if (hsurModemStatusValue & UR2_DDCD)
      {
            if (hsurModemStatusValue & UR2_DCD)
            {
                  UR2_MCR |= UR2_OUT2;
                  hsurModemControlValue |= UR2_OUT2;
            }
            else
            {
                  UR2_MCR &= ~UR2_OUT2;
                  hsurModemControlValue &= ~UR2_OUT2;
            }
      }
}


// EXPORTED SUBPROGRAM BODIES //

//----------------------------------------------------------------------------------------
// Name    : HSUR_Func()
// Purpose : HSUR interrupt function which checks interrupt status.
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_Func(void)
{
      U8_T  intrStatus = 0;

      intrStatus = UR2_IIR & 0x0F;

      if (intrStatus == UR2_RLS_INTR)
      {
            hsur_ReadLsr();
      }
      else if (intrStatus == UR2_RD_TRIG_INTR)
      {
            UR2_IER &= ~UR2_RDI_ENB;
            hsur_RcvrTrig();
            UR2_IER |= UR2_RDI_ENB;
      }
      else if (intrStatus == UR2_RD_TI_INTR)
      {
            UR2_IER &= ~UR2_RDI_ENB;
            hsur_Rcvr();
            UR2_IER |= UR2_RDI_ENB;
      }
      else if (intrStatus == UR2_THRE_INTR)
      {
            hsur_Xmit();
      }
      else if (intrStatus == UR2_MS_INTR)
      {
            hsur_ReadMsr();
      }
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
// Purpose : Setup the HSUR mode. Before using the HSUR of AX11000, this function
//           must be executed to initiate.
// Params  : divisor - A divisor value used to calculate the baud rate of HSUR.
//           lCtrl - Line control register value.
//           intEnb - Interrupt enable register value.
//           fCtrl - FIFO control register value.
//           mCtrl - Modem control register value.
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_Setup(U16_T divisor, U8_T lCtrl, U8_T intEnb, U8_T fCtrl, U8_T mCtrl)
{
      U16_T i;

      for (i=0 ; i<UR2_MAX_RX_SIZE ; i++)
      {
            hsurRxBuffer[i] = 0;
      }
      for (i=0 ; i<UR2_MAX_TX_SIZE ; i++)
      {
            hsurTxBuffer[i] = 0;
      }

      hsurRxBufNum = 0;
      hsurTxBufNum = 0;
      hsurRxCount = 0;
      hsurTxCount = 0;
      hsurGetPtr = 0;
      hsurPutPtr = 0;

      switch (fCtrl & 0xC0)
      {
            case UR2_TRIG_01 :
                  hsurRxTrigLvl = 1;
                  break;
            case UR2_TRIG_04 :
                  hsurRxTrigLvl = 4;
                  break;
            case UR2_TRIG_08 :
                  hsurRxTrigLvl = 8;
                  break;
            case UR2_TRIG_14 :
                  hsurRxTrigLvl = 14;
                  break;
      }

      UR2_LCR = UR2_DLAB_ENB;
      UR2_DLL = (U8_T)(divisor & 0x00FF);
      UR2_DLH = (U8_T)((divisor & 0xFF00) >> 8);
      UR2_LCR &= ~UR2_DLAB_ENB;
      UR2_LCR = lCtrl;
      UR2_FCR = fCtrl;
//#if HSUR_RS485_ENABLE
//    UR2_MCR = (mCtrl | UR2_RS485_RECEIVE);
//#else
      UR2_MCR = mCtrl;
//#endif
//    UR2_IER = intEnb;
      hsurInitIerRec = intEnb;

      hsurLineControlValue = lCtrl;
      hsurFifoControlValue = fCtrl;
      hsurModemControlValue = mCtrl;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_Start()
// Purpose : Enable the HSUR interrupt to start servicing the interrupt
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_Start(void)
{
      UR2_IER = hsurInitIerRec;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetChar()
// Purpose : Getting one byte data from the software receiver buffer,
//           which stores data received from a serial bus.
// Params  :
// Returns : ch - one byte character in buffer.
// Note    :
//----------------------------------------------------------------------------------------
S8_T HSUR_GetChar(void)
{
      U8_T  ch = 0;
    
      while (hsurRxCount == 0) ;
      
      ch = hsurRxBuffer[hsurGetPtr];
      EA = 0;
      hsurGetPtr ++;
      hsurRxCount --;
      if (hsurGetPtr == UR2_MAX_RX_SIZE)
            hsurGetPtr = 0;
      EA = 1;

      if (hsurFlowOn)
      {
            if (hsurRxCount <= UR2_FLOW_LO_WATERMARK)
            {
                  hsurFlowOn = FALSE;

                  if (hsurFlowCtrl == UR2_FLOW_CTRL_HW)
                  {
                        HSUR_SetRTS();
                  }
                  else if (hsurFlowCtrl == UR2_FLOW_CTRL_X)
                  {
                        UR2_THR = ASCII_XON;
                  }
            }
      }

      return ch;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_PutChar(S8_T ch)
// Purpose : Putting  one byte data into the software transmitter buffer,
//           which stores data that be sent to a serial bus.
// Params  : ch - one byte data will be put into buffer.
// Returns : ch - the same data value will be returned.
// Note    :
//----------------------------------------------------------------------------------------
S8_T HSUR_PutChar(S8_T ch)
{
      U16_T tIdleStart = 0;
      U16_T tIdleStop = 0;

      if (hsurTxCount >= UR2_MAX_TX_SIZE)
        {
        printd("U2-TX-Full\r\n");
        return FALSE;
        }

      UR2_IER &= ~UR2_THRI_ENB;           
      hsurTxBuffer[hsurPutPtr] = ch;
      hsurPutPtr ++;
      hsurTxCount ++;

      if (hsurPutPtr == UR2_MAX_TX_SIZE)
        hsurPutPtr = 0;

      UR2_IER |= UR2_THRI_ENB;    // trigger hardware interrupt if hw FIFO empty //

#if UR2_APP_FLOW_CTRL
      if (hsurTxCount >= UR2_FLOW_HI_WATERMARK)
        return FALSE;
      
      return TRUE;
#else
      return ch;
#endif
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_InitValue()
// Purpose : Initiating all globe value in this driver function.
// Params  :
// Returns :
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_InitValue(void)
{
      U8_T  sysClk = 0;
      U16_T i;

      memset(&hsurRxBuffer[0],0,UR2_MAX_RX_SIZE);
      memset(&hsurTxBuffer[0],0,UR2_MAX_TX_SIZE);

      hsurRxBufNum = 0;
      hsurTxBufNum = 0;
      hsurRxCount = 0;
      hsurTxCount = 0;
      hsurGetPtr = 0;
      hsurPutPtr = 0;

      sysClk = CSREPR & 0xC0;
      switch (sysClk)
      {
            case SCS_100M :               break;
            case SCS_50M  :               break;
            case SCS_25M  :               break;
            default       :               break;
      }

} // End of HSUR_InitValue //

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetCharNb()
// Purpose : Getting data from the software receiver buffer, which stores data 
//           received from a serial bus. This function is similar to HSUR_GetChar,
//           but this function only check buffer one time.
// Params  :
// Returns : ch - one byte data in buffer will be returned if having.
// Note    :
//----------------------------------------------------------------------------------------
S8_T HSUR_GetCharNb(void)
{
      S8_T  ch = -1;
 
      if (hsurRxCount > 0)
      {
            UR2_IER &= ~UR2_RDI_ENB;

            ch = hsurRxBuffer[hsurGetPtr];            
            hsurGetPtr++;
            hsurRxCount --;
            if (hsurGetPtr == UR2_MAX_RX_SIZE)
                  hsurGetPtr = 0;

            if (hsurFlowOn)
            {
                  if (hsurRxCount <= UR2_FLOW_LO_WATERMARK)
                  {
                        hsurFlowOn = FALSE;

                        if (hsurFlowCtrl == UR2_FLOW_CTRL_HW)
                        {
                              HSUR_SetRTS();
                        }
                        else if (hsurFlowCtrl == UR2_FLOW_CTRL_X)
                        {
                              UR2_THR = ASCII_XON;
                        }
                  }
            }

            UR2_IER |= UR2_RDI_ENB;
      }

      return ch;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetByteNb(U8_T* pDat)
// Purpose : Getting data from the software receiver buffer, which stores data 
//           received from a serial bus. This function is similar to HSUR_GetChar,
//           but this function only check buffer one time.
// Params  : pDat - pointer to a buffer for storing the byte data.
// Returns : TRUE - success
//           FALSE - fail     
// Note    :
//----------------------------------------------------------------------------------------
U8_T HSUR_GetByteNb(U8_T* pDat)
{ 
      if (hsurRxCount > 0)
      {
            UR2_IER &= ~UR2_RDI_ENB;

            *pDat = hsurRxBuffer[hsurGetPtr];         
            hsurGetPtr++;
            hsurRxCount --;
            if (hsurGetPtr == UR2_MAX_RX_SIZE)
                  hsurGetPtr = 0;

            if (hsurFlowOn)
            {
                  if (hsurRxCount <= UR2_FLOW_LO_WATERMARK)
                  {
                        hsurFlowOn = FALSE;

                        if (hsurFlowCtrl == UR2_FLOW_CTRL_HW)
                        {
                              HSUR_SetRTS();
                        }
                        else if (hsurFlowCtrl == UR2_FLOW_CTRL_X)
                        {
                              UR2_THR = ASCII_XON;
                        }
                  }
            }

            UR2_IER |= UR2_RDI_ENB;

            return TRUE;
      }
      
      return FALSE;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetRxBufCount()
// Purpose : Getting the remain character number in UART2 Rx buffer.
// Params  : void
// Returns : hsurRxCount - the number of remain character in UART2 Rx buffer.
// Note    :
//----------------------------------------------------------------------------------------
U16_T HSUR_GetRxBufCount(void)
{
      return hsurRxCount;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_SetFlowControl(U8_T mode)
// Purpose : Setting the UART2 flow control mode.
// Params  : mode - flow control mode.
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetFlowControl(U8_T mode)
{
      hsurFlowCtrl = mode;
}

//----------------------------------------------------------------------------------------
// BOOL HSUR_CheckTxComplete()
// Purpose : Check the hardware FIFO empty and software Tx buffer empty
// Params  : void
// Returns : TRUE - both empty.
// Note    :
//----------------------------------------------------------------------------------------
BOOL HSUR_CheckTxComplete(void)
{
      if ((UR2_LSR & UR2_THRE) && (hsurTxCount == 0))
            return TRUE;
      
      return FALSE;
}


//----------------------------------------------------------------------------------------
// Name    : HSUR_RxBufFlowControl()
// Purpose : Check the receiving buffer space and return the flow control character.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_RxBufFlowControl(void)
{
      if (hsurFlowCtrl == UR2_FLOW_CTRL_HW)
      {
            if (hsurRxCount >= UR2_FLOW_HI_WATERMARK)
            {
                  hsurFlowOn = TRUE;
                  UR2_MCR &= ~UR2_RTS;
            }
            else if ((hsurFlowOn) && (hsurRxCount <= UR2_FLOW_LO_WATERMARK))
            {
                  hsurFlowOn = FALSE;
                  UR2_MCR |= UR2_RTS;
            }
      }
      else if (hsurFlowCtrl == UR2_FLOW_CTRL_X)
      {
            if (hsurRxCount >= UR2_FLOW_HI_WATERMARK)
            {
                  hsurFlowOn = TRUE;
                  UR2_THR = ASCII_XOFF;
            }
            else if ((hsurFlowOn) && (hsurRxCount <= UR2_FLOW_LO_WATERMARK))
            {
                  hsurFlowOn = FALSE;
                  UR2_THR = ASCII_XON;
            }
      }
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_TxBufFlowControl()
// Purpose : Check the transmitting flow control character.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_TxBufFlowControl(void)
{     
      U16_T delayCounter = 0;

      if (hsurFlowCtrl == UR2_FLOW_CTRL_HW)
      {
            if (hsurFlowCtrlHwCTSon == TRUE)
            {
                  return;
            }

            while (1)
            {
                  if (hsurFlowCtrlHwCTSon == TRUE)
                  {
                        return;
                  }
                  else
                  {
                        if (++delayCounter == 0xFFFF) // timeout
                        {
                              return;
                        }
                  }
            }
      }
      else if (hsurFlowCtrl == UR2_FLOW_CTRL_X)
      {
            if (hsurFlowCtrlXoff == TRUE)
            {
                  hsurFlowCtrlXoff = FALSE;
      
#if DEBUG_DISPLAY_DIAG
                  EA = 0;
                  hsurXoffCounter ++;
                  EA = 1;
#endif

                  while (1)
                  {
                        if (hsurFlowCtrlXon == TRUE)
                        {
                              hsurFlowCtrlXon = FALSE;
#if DEBUG_DISPLAY_DIAG
                              EA = 0;
                              hsurXonCounter ++;
                              EA = 1;
#endif
                    break;
                        }
                        else
                        {
                              if (++delayCounter == 0xFFFF) // timeout
                              {
                                    return;                                   
                              }
                        }
                  }
            }
      }
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_ErrorRecovery()
// Purpose : Checking the HSUR Line Status Register value of errors and
//           re-enable interrupt of receiving and line status.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_ErrorRecovery(void)
{
      if (hsurErrBlocking)
      {
            U8_T  lsrVal = UR2_LSR;
            U8_T  trash = 0;

            printd("hsurErrBlocking = 1\n\r");
            if (lsrVal & UR2_DR)
            {
                  trash = UR2_RBR;
            }
            else if (!(lsrVal & UR2_DR) && (hsurErrorCount >= 30))
            {
                  printd("clear hsurErrBlocking = 0\n\r");
                  EA = 0;
                  UR2_IER |= (UR2_RDI_ENB | UR2_RLSI_ENB | UR2_MSI_ENB);
                  hsurErrBlocking = 0;
                  hsurErrorCount = 0;
                  EA = 1;
            }
      }
}

//----------------------------------------------------------------------------------------
// BOOL HSUR_SetBaudRate(U16_T divBaudRate)
// Purpose : Setting the HSUR baudrate.
// Params  : divBaudRate - divisor of the baudrate.
// Returns : TRUE - setting complete.
// Note    :
//----------------------------------------------------------------------------------------
BOOL HSUR_SetBaudRate(U16_T divBaudRate)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_LCR |= UR2_DLAB_ENB;
      UR2_DLL = (U8_T)(divBaudRate & 0x00FF);
      UR2_DLH = (U8_T)((divBaudRate & 0xFF00) >> 8);
      UR2_LCR &= ~UR2_DLAB_ENB;

      hsurBaudRateDivisor = divBaudRate;

      switch (CSREPR & 0xC0)
      {
            case SCS_100M :
                  P3 = 0x10;
                  break;
            case SCS_50M :
                  P3 = 0x50;
                  break;
            case SCS_25M :
                  P3 = 0x25;
                  break;
      }

      UR2_FCR |= UR2_RXFIFO_RST | UR2_TXFIFO_RST;
      UR2_IER = recIntrEnb;

      return TRUE;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_SetLineControl(U8_T value)
// Purpose : Setting the HSUR Line control register with "value".
// Params  : value - variable to be set.
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetLineControl(U8_T value)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_LCR = value;
      hsurLineControlValue = value;
      UR2_IER = recIntrEnb;
}

//----------------------------------------------------------------------------------------
// BOOL HSUR_SetupPort(U16_T divBaudRate, U8_T lctrl)
// Purpose : Setting the HSUR baudrate.
// Params  : divBaudRate - divisor of the baudrate.
//           lctrl - line control 
// Returns : 
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetupPort(U16_T divBaudRate, U8_T lctrl)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_LCR |= UR2_DLAB_ENB;
      UR2_DLL = (U8_T)(divBaudRate & 0x00FF);
      UR2_DLH = (U8_T)((divBaudRate & 0xFF00) >> 8);
      UR2_LCR &= ~UR2_DLAB_ENB;

      hsurBaudRateDivisor = divBaudRate;

      switch (CSREPR & 0xC0)
      {
            case SCS_100M :
                  P3 = 0x10;
                  break;
            case SCS_50M :
                  P3 = 0x50;
                  break;
            case SCS_25M :
                  P3 = 0x25;
                  break;
      }

      UR2_FCR |= UR2_RXFIFO_RST | UR2_TXFIFO_RST;

    UR2_LCR = lctrl;
      hsurLineControlValue = lctrl;

      UR2_IER = recIntrEnb;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_SetFifoControl(U8_T value)
// Purpose : Setting the HSUR FIFO control register with "value".
// Params  : value - variable to be set.
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetFifoControl(U8_T value)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_FCR = value;
      hsurFifoControlValue = value;
      UR2_IER = recIntrEnb;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_SetModemControl(U8_T value)
// Purpose : Setting the HSUR Modem control register with "value".
// Params  : value - variable to be set.
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetModemControl(U8_T value)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_MCR = value;
      hsurModemControlValue = value;
      UR2_IER = recIntrEnb;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_SetRTS()
// Purpose : Only setting the HSUR RTS bit of Modem control register.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetRTS(void)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_MCR |= UR2_RTS;
      hsurModemControlValue |= UR2_RTS;
      UR2_IER = recIntrEnb;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_SetDTR()
// Purpose : Only setting the HSUR DTR bit of Modem control register.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_SetDTR(void)
{
      U8_T  recIntrEnb = UR2_IER;

      UR2_IER = 0;
      UR2_MCR |= UR2_DTR;
      hsurModemControlValue |= UR2_DTR;
      UR2_IER = recIntrEnb;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_ClearRTS()
// Purpose : Only clear the HSUR RTS bit of Modem control register.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_ClearRTS(void)
{
      UR2_MCR &= ~UR2_RTS;
      hsurModemControlValue &= ~UR2_RTS;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_ClearDTR()
// Purpose : Only clear the HSUR DTR bit of Modem control register.
// Params  : void
// Returns : void
// Note    :
//----------------------------------------------------------------------------------------
void HSUR_ClearDTR(void)
{
      UR2_MCR &= ~UR2_DTR;
      hsurModemControlValue &= ~UR2_DTR;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetBaudRate()
// Purpose : Getting the HSUR baudrate divisor.
// Params  : void
// Returns : divBaudRate - baudrate divisor.
// Note    :
//----------------------------------------------------------------------------------------
U16_T HSUR_GetBaudRate(void)
{
      U16_T divBaudRate = 0;

      switch (CSREPR & 0xC0)
        {
        case SCS_100M :
             UR2_LCR |= UR2_DLAB_ENB;
             switch (hsurBaudRateDivisor)
               {
               case UR2_BR100_921600 :     divBaudRate = UR2_BR100_921600;  break;
               case UR2_BR100_115200 :     divBaudRate = UR2_BR100_115200;  break;
               case UR2_BR100_57600  :     divBaudRate = UR2_BR100_57600;   break;
               case UR2_BR100_38400  :     divBaudRate = UR2_BR100_38400;   break;
               case UR2_BR100_19200  :     divBaudRate = UR2_BR100_19200;   break;
               case UR2_BR100_9600   :     divBaudRate = UR2_BR100_9600;    break;
               case UR2_BR100_7200   :     divBaudRate = UR2_BR100_7200;    break;
               case UR2_BR100_4800   :     divBaudRate = UR2_BR100_4800;    break;
               case UR2_BR100_3600   :     divBaudRate = UR2_BR100_3600;    break;
               case UR2_BR100_2400   :     divBaudRate = UR2_BR100_2400;    break;
               case UR2_BR100_1200   :     divBaudRate = UR2_BR100_1200;    break;
               }
             UR2_LCR &= ~UR2_DLAB_ENB;
             break;
        case SCS_50M :
             UR2_LCR |= UR2_DLAB_ENB;
             switch (hsurBaudRateDivisor)
               {
               case UR2_BR50_921600 :      divBaudRate = UR2_BR50_921600;   break;
               case UR2_BR50_115200 :      divBaudRate = UR2_BR50_115200;   break;
               case UR2_BR50_57600  :      divBaudRate = UR2_BR50_57600;    break;
               case UR2_BR50_38400  :      divBaudRate = UR2_BR50_38400;    break;
               case UR2_BR50_19200  :      divBaudRate = UR2_BR50_19200;    break;
               case UR2_BR50_9600   :      divBaudRate = UR2_BR50_9600;     break;
               case UR2_BR50_7200   :      divBaudRate = UR2_BR50_7200;     break;
               case UR2_BR50_4800   :      divBaudRate = UR2_BR50_4800;     break;
               case UR2_BR50_3600   :      divBaudRate = UR2_BR50_3600;     break;
               case UR2_BR50_2400   :      divBaudRate = UR2_BR50_2400;     break;
               case UR2_BR50_1200   :      divBaudRate = UR2_BR50_1200;     break;
               }
             UR2_LCR &= ~UR2_DLAB_ENB;
             break;
        case SCS_25M :
             UR2_LCR |= UR2_DLAB_ENB;
             switch (hsurBaudRateDivisor)
               {
               case UR2_BR25_921600 :      divBaudRate = UR2_BR25_921600;   break;
               case UR2_BR25_115200 :      divBaudRate = UR2_BR25_115200;   break;
               case UR2_BR25_57600  :      divBaudRate = UR2_BR25_57600;    break;
               case UR2_BR25_38400  :      divBaudRate = UR2_BR25_38400;    break;
               case UR2_BR25_19200  :      divBaudRate = UR2_BR25_19200;    break;
               case UR2_BR25_9600   :      divBaudRate = UR2_BR25_9600;     break;
               case UR2_BR25_7200   :      divBaudRate = UR2_BR25_7200;     break;
               case UR2_BR25_4800   :      divBaudRate = UR2_BR25_4800;     break;
               case UR2_BR25_3600   :      divBaudRate = UR2_BR25_3600;     break;
               case UR2_BR25_2400   :      divBaudRate = UR2_BR25_2400;     break;
               case UR2_BR25_1200   :      divBaudRate = UR2_BR25_1200;     break;
               }
             UR2_LCR &= ~UR2_DLAB_ENB;
             break;
       }
       return TRUE;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetLineControl()
// Purpose : Getting the HSUR Line Control register variables.
// Params  : void
// Returns : varLineCtrl.
// Note    :
//----------------------------------------------------------------------------------------
U8_T HSUR_GetLineControl(void)
{
      U8_T  varLineCtrl = 0;

      varLineCtrl = UR2_LCR;
      return varLineCtrl;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetFifoControl()
// Purpose : Getting the HSUR FIFO Control register variables.
// Params  : void
// Returns : hsurFifoControlValue.
// Note    :
//----------------------------------------------------------------------------------------
U8_T HSUR_GetFifoControl(void)
{
      return hsurFifoControlValue;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetModemControl()
// Purpose : Getting the HSUR Modem Control register variables.
// Params  : void
// Returns : hsurModemControlValue.
// Note    :
//----------------------------------------------------------------------------------------
U8_T HSUR_GetModemControl(void)
{
      return hsurModemControlValue;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetLineStatus(U8_T intrSynth)
// Purpose : Getting the HSUR Line Status .
// Params  : intrSynth - 1: Get LSR according to an interrupt trigger.
//                       0: Get LSR by directly reading the register.
// Returns : varLineStatus - Line status register variable.
// Note    :
//----------------------------------------------------------------------------------------
U8_T HSUR_GetLineStatus(U8_T intrSynth)
{
      U8_T  varLineStatus = 0;

      if (intrSynth == TRUE)
        VarLineStatus = hsurLineStatusValue;
      else if (intrSynth == FALSE)
        varLineStatus = UR2_LSR;
      return varLineStatus;
}

//----------------------------------------------------------------------------------------
// Name    : HSUR_GetModemStatus(U8_T intrSynth)
// Purpose : Getting the HSUR Modem Status .
// Params  : intrSynth - 1: Get MSR according to an interrupt trigger.
//                       0: Get MSR by directly reading the register.
// Returns : varModemStatus - Modem status register variable.
// Note    :
//----------------------------------------------------------------------------------------
U8_T HSUR_GetModemStatus(U8_T intrSynth)
{
      U8_T  varModemStatus = 0;

      if (intrSynth == TRUE)
        varModemStatus = hsurModemStatusValue;
      else if (intrSynth == FALSE)
        varModemStatus = UR2_MSR;
      return varModemStatus;
}

//----------------------------------------------------------------------------------------
// BOOL HSUR_XonByApp()
// Purpose : Application issue flow control characters Xon to driver for
//           stopping or starting the current transfer.
// Params  : void
// Returns : TRUE - driver is already in Xon/Xoff flow control.
//           FLASE - driver is not in Xon/Xoff flow control.
// Note    :
//----------------------------------------------------------------------------------------
BOOL HSUR_XonToApp(void)
{
      if (hsurFlowCtrl != UR2_FLOW_CTRL_X)
        {        return FALSE;        }
      hsurAppFlowOn = TRUE;
      return TRUE;
}

//----------------------------------------------------------------------------------------
// BOOL HSUR_XoffByApp()
// Purpose : Application issue flow control characters Xoff to driver for
//           stopping or starting the current transfer.
// Params  : void
// Returns : TRUE - driver is already in Xon/Xoff flow control.
//           FLASE - driver is not in Xon/Xoff flow control.
// Note    :
//----------------------------------------------------------------------------------------
BOOL HSUR_XoffByApp(void)
{
      if (hsurFlowCtrl != UR2_FLOW_CTRL_X)
        {        return FALSE;        }
      hsurAppFlowOn = FALSE;
      return TRUE;
}

// End of hsuart.c //
