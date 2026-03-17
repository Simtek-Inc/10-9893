//========================================================================================//
//     Copyright (c) 2016     Simtek, Incorporated      All rights reserved.              //
//                                                                                        //
//     This is unpublished proprietary source code of Simtek, Inc                         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//========================================================================================//
// 75-3032  for 10-8436-01                                                                //
// Circuit boards used : 51-5330-01                                                       //
// Used the programs from C. Mason 10-8149 to start with                                  //
//                                                                                        //
//========================================================================================//
// Module Name : Main.c                                                                   //
// Purpose     : Main loop                                                                //
//========================================================================================//
// Author      : C. Mason                                                                 //
// Date        : July 20, 2016                                                            //
// Revision    : -                                                                        //
// Notes       : Shipped with instrument.                                                 //
//========================================================================================//

//========================================================================================//
//          include file declarations                                                     //
//========================================================================================//
#include "reg80390.h"                                                                     //
#include "main.h"                                                                         //
#include "fpga.h"                                                                         //
#include "adapter.h"                                                                      //
#include "mstimer.h"                                                                      //
#include "ax11000.h"                                                                      //
#include "interrupt.h"                                                                    //
#include "stoe.h"                                                                         //
#include "dma.h"                                                                          //
#include "tcpip.h"                                                                        //
#include "printd.h"                                                                       //
#include "delay.h"                                                                        //
#include "intrins.h"                                                                      //
#include "Hardware.h"                                                                     //
#include "uart.h"                                                                         //
#include "gconfig.h"                                                                      //
#include "phy.h"                                                                          //
#include "mac.h"                                                                          //
#include "gudpdat.h"                                                                      //
#include "uart.h"                                                                         //
#include "gs2e.h"                                                                         //
#include <string.h>                                                                       //
#include <stdio.h>                                                                        //
#include "CIRCUIT.H"                                                                      //
#include "instrument.h"                                                                   //
#include <math.h>                                                                         //
#include "dhcpc.h"                                                                        //
#include <stdlib.h>
#include "util_menus.h"                                                                   //
//========================================================================================//
//                naming #define declarations                                             //
//========================================================================================//
#define  RS422                            0                                               //
#define  RS232                            1                                               //
#define  DEDisable                        0                                               //
#define  DEEnable                         1                                               //

#ifdef   DEBUG                                                                            //
#define  DBGMSG(A) {A}                                                                    //
#else                                                                                     //
#define  DBGMSG(A) {}                                                                     //
#endif                                                                                    //
#define  TIME_OUT_COUNTER                 (20/SWTIMER_INTERVAL)                           //

#define Serial                             0                                              //
#define Ethernet                           1                                              //
#define Host_Port                         (Ethernet)                                      //

// MACRO DECLARATIONS //
#if   ( Host_Port == Serial )
#define     wrHost(p,c)    UART1_PutStr(p,c)
#else
#define     wrHost(p,c)    wrStr2EthTxBuffer(p,c)
#endif

//----------------------------------------------------------------------------------------//
//          GLOBAL VARIABLES DECLARATIONS                                                 //
//----------------------------------------------------------------------------------------//
typedef struct app_buf {                                                                  //
      ulong       ipaddr;                                                                 //
      byte        buf[100];                                                               //
      word        uip_len;                                                                //
      word        PayLoadOffset;                                                          //
      byte        wait;                                                                   //
}APP_BUF;                                                                                 //

APP_BUF           app_arp_buf;                                                            //

ulong  processEthernetTraffic(ulong ptc, dls);                                            //
//----------------------------------------------------------------------------------------//
//          static variable declarations                                                  //
//----------------------------------------------------------------------------------------//
//#if DebugBuild
byte              EthTxBuffer[EthMaxTxSize];                                              //
word              EthTxBufHead            = 0;                                            // array index to write data into the temporary buffer
word              EthTxBufTail            = 0;                                            // array index to transfer data into the actual transmit buffer
word              EthTxCount              = 0;                                            // number of bytes in the temporary buffer to transmit
//#endif
word              HrtBeatCnt              = 0x00;                                         //
       word       MinMsgLen;                                                              //
       byte       HardwareSettings;                                                       //
//----------------------------------------------------------------------------------------//
//          hardware specific definitions                                                 //
//----------------------------------------------------------------------------------------//
// ACC (0xE0) Bit Registers                                                               //
sbit A_0                      = 0xE0;                                                     //
sbit A_1                      = 0xE1;                                                     //
sbit A_2                      = 0xE2;                                                     //
sbit A_3                      = 0xE3;                                                     //
sbit A_4                      = 0xE4;                                                     //
sbit A_5                      = 0xE5;                                                     //
sbit A_6                      = 0xE6;                                                     //
sbit A_7                      = 0xE7;                                                     //

       word idata Word1    _at_ 0x20;                                                     //
       byte bdata HiByte   _at_ 0x20;                                                     //
       byte bdata LoByte   _at_ 0x21;                                                     //
    sbit          bits00  = LoByte^0;                                                     //
    sbit          bits01  = LoByte^1;                                                     //
    sbit          bits02  = LoByte^2;                                                     //
    sbit          bits03  = LoByte^3;                                                     //
    sbit          bits04  = LoByte^4;                                                     //
    sbit          bits05  = LoByte^5;                                                     //
    sbit          bits06  = LoByte^6;                                                     //
    sbit          bits07  = LoByte^7;                                                     //
    sbit          bits08  = HiByte^0;                                                     //
    sbit          bits09  = HiByte^1;                                                     //
    sbit          bits0A  = HiByte^2;                                                     //
    sbit          bits0B  = HiByte^3;                                                     //
    sbit          bits0C  = HiByte^4;                                                     //
    sbit          bits0D  = HiByte^5;                                                     //
    sbit          bits0E  = HiByte^6;                                                     //
    sbit          bits0F  = HiByte^7;                                                     //
//========================================================================================//
//          port pin name definition                                                      //
//========================================================================================//
byte PinDefStr0[] = PORT0_PIN_NAMES;                                                      //
byte PinDefStr1[] = PORT1_PIN_NAMES;                                                      //
byte PinDefStr2[] = PORT2_PIN_NAMES;                                                      //
byte PinDefStr3[] = PORT3_PIN_NAMES;                                                      //
//========================================================================================//
//          ethernet message frames                                                       //
//========================================================================================//
byte bufResetReq[bufResetMsgLength];                                                      //
byte bufStatusRsp[bufStatusMsgLength];                                                    //
byte bufStatusReq[bufStatusMsgLength];                                                    //
byte bufIndicatorMsg[bufIndicatorMsgLength];                                              //

byte bufDimmingMsg[bufDimmingMsgLength];                                                  //
byte bufDimmingTmp[bufDimmingMsgLength];                                                  //

byte bufDisplayMsg[bufDisplayMsgLength];                                                  //
byte bufFirmwareReq[bufFirmwareMsgLength];                                                 //
byte bufFirmwareRsp[bufFirmwareMsgLength];                                                 //

byte DefaultDevName[16];                                                                  //
//----------------------------------------------------------------------------------------//

//----------------------------------------------------------------------------------------//
//                local variables declarations                                            //
//----------------------------------------------------------------------------------------//
word   PnlLgt                        = 4100;                                              //
//----------------------------------------------------------------------------------------//
bool   fDebugMode;                                                                        //
bool   fTransmitMode;                                                                     //

byte   ChngCnt;                                                                           //
byte   stage;                                                                             //
word   stagetimeout;                                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                serial variables                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
       byte  mRxBuf[LargestMsgLen];                                                       //
       word  mRxCnt;                                                                      //
       bool  fNormalRun;                                                                  //
       byte  baudrate;
//----------------------------------------------------------------------------------------//
//                                                                                        //
//----------------------------------------------------------------------------------------//
//          action counters                                                               //
//========================================================================================//
word  acResetRequest;                                  // host requested       //         //
word  acFirmwareRequest;                               // host requested       //         //
word  acStatusRequest;                                 // host requested       //         //
word  acDimmingRequest;                                // host requested       //         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                local subprogram prototype declarations                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
void  NOP(word loop);                                                                     //
//==== function used to access the TOE buffers and controls ==============================//
word  wrChr2EthTxBuffer(byte ch, word Count);                                             //
word  wrStr2EthTxBuffer(byte* pData, word cnt);                                           // write data to the ethernet transmit buffer
void  writeEthTxCount(word TempCount);                                                    //
static void UpdateIpSettings(ulong);                                                      // update the IP information in the ethernet DCB
word  readEthTxBufCount(void);                                                            //
//==== function used to access the UART buffers and controls =============================//
void  UART_Write(byte c);                                                                 //
//==== function used to send reponse and report messages to the Host system ==============//
//void  sendDimmingRsp(void);                                                               //
void  sendFirmwareRsp(void);                                                              //
void  sendSwStatusRsp(void);                                                              //
// ==== 
void  processResetRsp(void);                                                              //
 byte  realtimeswblock;                                                                   //
void  processDimmingRsp(void);                                                            //
void  processFirmwareRsp(void);                                                           //
void  processStatusRsp(void);                                                             //
void  processSerialData(void);                                                            //
// ==== 
void  slaveResetReq(void);                                                                //
void  slaveDimmingMsg(void);                                                              //
void  slaveStatusReq(void);                                                               //
void  slaveFirmwareReq(void);                                                             //
// ==== 
void  printFirmwareInformation();                                                         //
void  printDeviceConfigurationInformation();                                              //
ulong setupEthernet(byte enabled);                                                        //
// =======================================================================================//

//== ---------------------------------------------------------------------------------- ==//
//== -                       local subprogram definitions                             - ==//
//== ---------------------------------------------------------------------------------- ==//

//----------------------------------------------------------------------------------------//
// name    : NOP                                                                          //
// purpose : create a small runtime delay                                                 //
// params  : the number of times to loop                                                  //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------//
void NOP(word loop)                                                                       //
{
      word i;                                                                             //

      for (i = 0; i < loop; i++)                                                          //
       {    _nop_();    }                                                                 //
}

//----------------------------------------------------------------------------------------
// name    : wrChr2EthTxBuffer(byte ch, word Count)                                       //
// purpose : write data to the ring buffer to transmit over Ethernet                      //
// params  : Data byte to write, Count of bytes currently in buffer                       //
// returns : updated count of bytes in buffer                                             //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
word wrChr2EthTxBuffer(byte ch, word Count)                                               // write data to the ethernet transmit buffer
{
      EthTxBuffer[EthTxBufHead] = ch;                                                     // add the data to the ethernet buffer
      EthTxBufHead++;                                                                     // update the ethernet buffer index
      Count++;                                                                            // update the count showing the number of bytes in the buffer
      if (EthTxBufHead == EthMaxTxSize)                                                   // check to see if the index is at the maximum
        EthTxBufHead = 0;                                                                 // reset the index to the first byte of the buffer
      return Count;                                                                       // pass the buffer count back to the calling routine
}

//----------------------------------------------------------------------------------------
// name    : wrStr2EthTxBuffer(byte* pData, word cnt)                                     //
// purpose : write data to the ring buffer to transmit over Ethernet                      //
// params  : pointer to the data bytes to write, number of bytes to write                 //
// returns : number of bytes sucessfully written into the buffer                          //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
word wrStr2EthTxBuffer(byte* pData, word cnt)                                             // write data to the ethernet transmit buffer
{
      word TmpCnt;                                                                        //
      word offset;                                                                        //

      if((EthTxBufHead + cnt) > EthMaxTxSize)                                             // make sure that the number of bytes to write added to the number
        {                                                                                   //  of bytes in the buffer already is not greater then the buffer
        offset = EthMaxTxSize - EthTxBufHead;                                             // find the number of bytes that will fit in the buffer before overflow
        memmove(&EthTxBuffer[EthTxBufHead], pData, offset);                               // fill the remainder of the buffer with the given data
        memmove(&EthTxBuffer[ix01], pData+offset, cnt - offset);                          // add the remainder of the given data to the begining of the buffer
        }
      else                                                                                // else all of the given data will still fit in the buffer
        {
        memmove(&EthTxBuffer[EthTxBufHead], pData, cnt);                                  // place all of the given data into the buffer
        }
      EthTxBufHead += cnt;                                                                // make sure that the index points at the first empty byte
      EthTxBufHead &= EthMaxTxMask;                                                       // mask the index to wrap the buffer around into a ring
      TmpCnt        = readEthTxBufCount() + cnt;                                          // adjust the current buffer count
      writeEthTxCount(TmpCnt);
      return TmpCnt;                                                                      // return the number of bytes in the buffer
}

//----------------------------------------------------------------------------------------
// name    : writeEthTxCount(word TempCount)                                              //
// purpose : update the count checked by Ethernet process                                 //
// params  : updated count                                                                //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void writeEthTxCount(word TempCount)                                                      // update the global ethernet transmit buffer count
{
      EthTxCount = TempCount;                                                             // write the new count to the global buffer
}

//----------------------------------------------------------------------------------------
// name    : main_readEthChar(byte* pDat)                                                 //
// purpose : retrieve data from buffer to place in ethernet transmit buffer               //
// params  : pointer to variable to return                                                //
// returns : count of data left in buffer                                                 //
// note    : this routine replaced GetEthChar defined in main.c.                          //
//----------------------------------------------------------------------------------------
word main_readEthChar(byte* pDat)                                                         // read a data byte from the ethernet buffer
{
    word temp;                                                                            //

    temp = readEthTxBufCount();                                                           // get the current ethernet buffer count
    if (temp > 0)                                                                         // see if there are any bytes in the buffer
      {
      *pDat = EthTxBuffer[EthTxBufTail];                                                  // write the data to the location pointed to by the pointer
      EthTxBufTail++;                                                                     // update the ethernet ring buffer index
      if (EthTxBufTail == EthMaxTxSize)                                                   // check the index to make sure that the index remains within the buffer space
        {  EthTxBufTail = 0;  }                                                           // reset the index to the first byte of the buffer
      EthTxCount--;                                                                       // update the count of the bytes in the buffer
      return EthTxCount;                                                                  // pass the altered count back to the calling routine
      }
    return 0;                                                                             // write the default value for the count
}

//----------------------------------------------------------------------------------------
// name    : UpdateIpSettings                                                             //
// purpose : Update IP address, subnet mak, gateway IP address and DNS IP address         //
// params  : IP to use for update                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void UpdateIpSettings(ulong ip)                                                           // Update IP address, subnet mask, gateway IP address and DNS IP address
{
    if (ip > 0)                                                                           // check the IP to see if it is valid
      {
      printd("IP  : %bu.%bu.%bu.%bu\n\r", (byte) ((ip >> 24) & 0x000000FF),               // write the IP address to the user debug port
                                          (byte) ((ip >> 16) & 0x000000FF),               //
                                          (byte) ((ip >> 8)  & 0x000000FF),               //
                                          (byte) ( ip        & 0x000000FF));              //
      }
    else                                                                                  // the IP was valid
      if (ip == 0)                                                                        // check the IP to see if it is valid
        {
        ulong mask = GCONFIG_GetNetmask();                                                // retrieve the default net mask
        ip = GCONFIG_GetServerStaticIP();                                                 // retrieve the static IP
        TCPIP_SetIPAddr(ip);                                                              // write the default IP addess into the DCB
        STOE_SetIPAddr(ip);                                                               // write the default IP addess into the DCB
        TCPIP_SetSubnetMask(mask);                                                        // write the default net mask into the DCB
        STOE_SetSubnetMask(mask);                                                         // write the default net mask into the DCB
        }
    GS2E_Init();                                                                          //
} // End of UpdateIpSettings //

//----------------------------------------------------------------------------------------
// name    : readEthTxBufCount(void)                                                      //
// purpose : read the current number of bytes in the Ethernet ring buffer                 //
// params  : void                                                                         //
// returns : current number of bytes in the Ethernet ring buffer                          //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
word readEthTxBufCount(void)                                                              // retrieve the current number of bytes in the ethernet transmit buffer
{
    return EthTxCount;                                                                    // pass the ethernet transmit buffer to the calling routine
}

//----------------------------------------------------------------------------------------
// name    : sendDimmingRsp                                                               //
// purpose : add a message to the host interface transmit buffer                          //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
//void  sendDimmingRsp(void)                                                                //
//{
//    wrHost(&bufDimmingMsg[ix01], s2mFirmwareMsgLength);                                   // send the host a status response message
//}

//----------------------------------------------------------------------------------------
// name    : sendFirmwareRsp                                                              //
// purpose : add a message to the host interface transmit buffer                          //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  sendFirmwareRsp(void)                                                               //
{
    wrHost(&bufFirmwareRsp[ix01], s2mFirmwareMsgLength);                                  // send the host a status response message
}

//----------------------------------------------------------------------------------------
// name    : sendSwStatusRsp                                                              //
// purpose : add a message to the host interface transmit buffer                          //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  sendSwStatusRsp(void)                                                               // write a status message to the host transmit buffer
{
    wrHost(&bufStatusRsp[ix01], s2mStatusMsgLength);                                      // send the host a status response message
}

//----------------------------------------------------------------------------------------
// name    : processResetRsp                                                              //
// purpose : execute all steps required for a response                                    //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  processResetRsp(void)                                                               //
{
      int   i     = 0;

      if(acResetRequest > 0)                                                              // process an host triggered reset
        {
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initailize the message buffer                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        strncpy(&bufResetReq[ix01],resetdefault,sizeof(resetdefault));                    //
        strncpy(&bufFirmwareReq[ix01],firmwaredefault,sizeof(firmwaredefault));           //
        strncpy(&bufFirmwareRsp[ix01],firmwaredefault,sizeof(firmwaredefault));           //
        strncpy(&bufStatusReq[ix01],statusdefault,sizeof(statusdefault));                 //
        strncpy(&bufStatusRsp[ix01],statusdefault,sizeof(statusdefault));                 //
        strncpy(&bufIndicatorMsg[ix01],indicatordefault,sizeof(indicatordefault));        //
        strncpy(&bufDimmingMsg[ix01],dimmingdefault,sizeof(dimmingdefault));              //
        strncpy(&bufDimmingTmp[ix01],dimmingdefault,sizeof(dimmingdefault));              //
        strncpy(&bufDisplayMsg[ix01],displaydefault,sizeof(displaydefault));              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//          initailize the runtime action counters                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        acDimmingRequest          = 0;                                                    // clear the dimming response action count
        acFirmwareRequest         = 0;                                                    // clear the firmware response action count
        acStatusRequest           = 0;                                                    // clear the status response action count
        acResetRequest            = 0;                                                    // clear the display change request flag
        HrtBeatCnt                = 0;                                                    //
        slaveResetReq();                                                                  // reset the slave processes
        stagetimeout = 50000;                                                               // request the status of slave 0 user inputs
        while(((s2mFirmwareRspLength + s2mStatusMsgLength)*2) > UART1_GetByteCount())       // check for the minimum message size
          {
          if(stagetimeout == 1)
            {
            slaveFirmwareReq(); 
            slaveStatusReq();            stagetimeout = 50000;
            }
          else
            {          stagetimeout--;        }                                                        // make sure the cycle continues evan if slave does not respond
          }
        processSerialData();                                                                // check serial rx buffer and process any messages
        }
}

//----------------------------------------------------------------------------------------
// name    : processFirmwareRsp                                                           //
// purpose : execute all steps required for a response                                    //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  processFirmwareRsp(void)                                                            //
{
    if(acFirmwareRequest > 0x00)                                                          // check for a firmware message request
      {
      sendFirmwareRsp();                                                                  // write a firmware message to the RS422 transmit buffer
      acFirmwareRequest--;                                                                // clear the fimeware revision request flag
      }
}

//----------------------------------------------------------------------------------------
// name    : processSerialData                                                           //
// purpose : get the serial responses from the slave and process them                     //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  processSerialData(void)
{
      byte  len              = 0;                                                         //

      mRxCnt        = UART1_GetByteCount();                                             // 
      while(mRxCnt >= MinMsgLen)                                                        // check for the minimum message size
        {
//****************************************************************************************************************************************************
//
//****************************************************************************************************************************************************
        memset(&mRxBuf[ix01], 0, LargestMsgLen);                                        // make sure to zero out the buffer before read from the global receive buffer
        UART1_GetBuf(&mRxBuf[ix01], 1);                                                 // get the message data from the serial receive buffer
//****************************************************************************************************************************************************
//
//****************************************************************************************************************************************************
        switch(mRxBuf[ix01])                                                            //
          {
          case StatusRequest:                                                           //
              len = s2mStatusMsgLength;                                                                      //
              if(mRxCnt >= len)                                                         // check if the defined message length matches the received message length
                {
                UART1_GetBuf(&mRxBuf[ix01], len);                                       // get the message data from the serial receive buffer
                memmove(&bufStatusRsp[ix02], &mRxBuf[ix02], len-1);                     // copy the new display data into the display data buffer
                UART1_RxBufSet(0,len);                                                  //
                UART1_RxBufUpdate(len);                                                 // remove the message from the recieve buffer
                MinMsgLen = DefaultMsgLen;                                              // set the minimum message length to the default
                stagetimeout = 1;                                                       // make sure that the next request goes out right away
                }
              else
                {
                MinMsgLen = len;                                                        // set the minimum message length
                }//else if(mRxCnt >= StatusMsgS0Len)
              break;
          case FirmwareRequest:
              len = s2mFirmwareRspLength;                                                  // 
              if(mRxCnt >= len)                                                         // check if the defined message length matches the received message length
                {
                UART1_GetBuf(&mRxBuf[ix01], len);                                       // get the message data from the serial receive buffer
                memmove(&bufFirmwareRsp[ix06], &mRxBuf[ix02], len-1);                    // copy the new display data into the display data buffer
                UART1_RxBufSet(0,len);                                                  //
                UART1_RxBufUpdate(len);                                                 // remove the message from the recieve buffer
                MinMsgLen = DefaultMsgLen;                                              // set the minimum message length to the default
                }
              else
                {
                MinMsgLen = len;                                                        // set the minimum message length
                }//else if(mRxCnt >= FirmwareMsgS0Len)
              break;
          default   :
              MinMsgLen = DefaultMsgLen;                                                // set the minimum message length to the default
              UART1_RxBufSet(0,1);                                                      //
              UART1_RxBufUpdate(1);                                                     // remove the message from the recieve buffer
              break;
          }// switch(mRxBuf[ix01])
        mRxCnt        = UART1_GetByteCount();                                           // 
        }// if(mRxCnt >= MinMsgLen)
}

//----------------------------------------------------------------------------------------
// name    : processDimmingRsp                                                            //
// purpose : execute all steps required for a response                                    //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  processDimmingRsp(void)                                                             //
{
    int  t;                                                                             //

    if(acDimmingRequest > 0x00)                                                           // check for a firmware message request
      {
      t = bufDimmingTmp[ix03] - bufDimmingMsg[ix03];                                      //
      if ((t >  40) || (bufDimmingMsg[ix03] <  20)) bufDimmingMsg[ix03] +=  8;            //
      else  bufDimmingMsg[ix03] = bufDimmingTmp[ix03];

      t = bufDimmingTmp[ix02] - bufDimmingMsg[ix02];                                      //
      if ((t >  40) || (bufDimmingMsg[ix02] <  20)) bufDimmingMsg[ix02] +=  8;            //
      else  bufDimmingMsg[ix02] = bufDimmingTmp[ix02];

//      printd("\n\r%bu -- %du\n\r",bufDimmingMsg[ix03],t);                                                //
      slaveDimmingMsg();                                                                  // write a firmware message to the RS422 transmit buffer
      if((bufDimmingMsg[ix02] == bufDimmingTmp[ix02])&&(bufDimmingMsg[ix03] == bufDimmingTmp[ix03])) acDimmingRequest--;                                                                 // clear the fimeware revision request flag
      }
}

//----------------------------------------------------------------------------------------
// name    : processStatusRsp                                                             //
// purpose : execute all steps required for a response                                    //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  processStatusRsp(void)                                                              //
{
    if(acStatusRequest > 0x00)                                                            // check for a Switch Status request
      {
      sendSwStatusRsp();                                                                  // write a status message to the RS422 transmit buffer
      acStatusRequest--;                                                                  // update the Switch Status request flag
      }
}

//----------------------------------------------------------------------------------------
// name    : printFirmwareInformation                                                     //
// purpose : display the  firmware information in the debug window                        //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  printFirmwareInformation()                                                          //
{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    printd("uC Master Firmware   PN : ");                                                 // output the firmware data for the master processor
    printd("%02bx-",   bufFirmwareRsp[ix02]);                                             //
    printd("%02bx",    bufFirmwareRsp[ix03]);                                             //
    printd("%02bx",    bufFirmwareRsp[ix04]);                                             //
    printd(" : %c\n\r",bufFirmwareRsp[ix05]);                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    printd("uC Slave Firmware    PN : ");                                                 // output the firmware data for the slave 0 processor
    printd("%02bx-",   bufFirmwareRsp[ix06]);                                             //
    printd("%02bx",    bufFirmwareRsp[ix07]);                                             //
    printd("%02bx",    bufFirmwareRsp[ix08]);                                             //
    printd(" : %c\n\r",bufFirmwareRsp[ix09]);                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    printd("FPGA Slave Firmware  PN : ");                                                 // output the firmware data for the slave 0 processor
    printd("%02bx-",   bufFirmwareRsp[ix10]);                                             //
    printd("%02bx",    bufFirmwareRsp[ix11]);                                             //
    printd("%02bx",    bufFirmwareRsp[ix12]);                                             //
    printd(" : %c\n\r",bufFirmwareRsp[ix13]);                                             //
    printd("*****************************************************\n\r");
}

//----------------------------------------------------------------------------------------
// name    : printDeviceConfigurationInformation                                          //
// purpose : show the user the device information in the debug window                     //
// params  : void                                                                         //
// returns : void                                                                         //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
void  printDeviceConfigurationInformation()                                               //
{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                                                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    printd("Debug Routines          : ");                                                 //
    if(fDebugMode == OFF)                                                                 // get the state of the debug bit
      printd("disabled\n\r");                                                             // alert the user to port status
    else                                                                                  //
      printd("enabled\n\r");                                                              // alert the user to port status

    printd("Transmit Mode           : ");                                                 // alert the user to port status
    if(fDebugMode == OFF)                                                                 // get the state of the debug bit
      {
      if(fTransmitMode == Server)                                                         // get the state of the client port bit
        printd("Server\n\r");                                                             // alert the user to port status
      else                                                                                //
        printd("Client\n\r");                                                             // alert the user to port status
      }
    else                                                                                  //
      printd("Server\n\r");                                                               // alert the user to port status

    gconfig_ExDisplayConfigData();
    printd("*****************************************************\n\r");
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          display the firmware information for all programs in the debug window         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
    printFirmwareInformation();                                                           //
}

//----------------------------------------------------------------------------------------
// name    : processEthernetTraffic                                                       //
// purpose : check for new Ethernet traffic and process new data                          //
// params  : last software timer count, display link speed flag                           //
// returns : current timer count                                                          //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
ulong processEthernetTraffic(ulong ptc, dls)                                              //
{
    ulong tc;                                                                             //
    byte  linkSpeed = 0;                                                                  //
    
    tc              = (word)SWTIMER_Tick();                                               // get the current timer count
    if ((tc - ptc) >= TIME_OUT_COUNTER)                                                   // see if the difference in the curent & last time differs more then the timeout
      {
      ptc           = tc;                                                                 // set the last time to the current time
      TCPIP_PeriodicCheck();                                                              // check the for any TCPIP traffic
      }
    STOE_ProcessInterrupt();                                                              // process all TOE buffers input/output
    if (MAC_GetInterruptFlag())                                                           // get the data for this device
      { linkSpeed   = MAC_ProcessInterruptExt(); }                                        // find the linkage speed, and process device traffic
    if(dls == ON) printd("Ethernet Link Speed = bps\n\r", linkSpeed);                     // display the linkage speed
    return ptc;                                                                           // return the current timer count
}

//----------------------------------------------------------------------------------------
// name    : setupEthernet                                                                //
// purpose : initailize and start all of the Ethernet processes                           //
// params  : enabled                                                                      //
// returns : the last updated timer count                                                 //
// notes   :                                                                              //
//----------------------------------------------------------------------------------------
ulong setupEthernet(byte enabled)
{
    ulong ip;                                                                             //
    ulong mask;                                                                           //
    byte  cmdDhcpFlag      = 0;                                                           //
    ulong dhcpTimeStart    = 0;                                                           //
    ulong swtc;
    
    if(enabled == TRUE)
      {
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initialize network adapter                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      ETH_Init();                                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initailize the device configurationnitialization                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      GCONFIG_Init();                                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          iniailize the software timer                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      SWTIMER_Init();                                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          enable transmit socket for runtime mode                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      if ((fDebugMode == OFF) && fTransmitMode == Client)                                 //
        {   gudpdat_ClientOpen(TrxSocket);  }                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          Start the Ethernet Engine and Software Timer for DMA                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      ETH_Start();                                                                        // start the ethernet process
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          Start the Ethernet Engine and Software Timer for DMA                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      SWTIMER_Start();                                                                    // start the software timer
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initailize the hardware DCB                                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      mask    = GCONFIG_GetNetmask();                                                     //
      ip      = GCONFIG_GetServerStaticIP();                                              //
      TCPIP_SetIPAddr(ip);                                                                //
      STOE_SetIPAddr(ip);                                                                 //
      TCPIP_SetSubnetMask(mask);                                                          //
      STOE_SetSubnetMask(mask);                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          update device IP and port settings                                            //
      if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_DHCP_ENABLE) == GCONFIG_NETWORK_DHCP_ENABLE)
        {
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initialize network adapter                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        printd("DHCP     init           : ");                                             //
        if (DHCP_Init())                                                                  //
          printd("pass\n\r");                                                             //
        else
          printd("**ERROR**\n\r");                                                        //
        printd("DHCP Assigned IP    : ");                                                 // tell user that the DHCP process has started
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//    start the DHCP engine                                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        DHCP_Start();                                                                     // start the DHCP process
        cmdDhcpFlag   = 1;                                                                // set the DHCP initailization flag to start
        dhcpTimeStart = SWTIMER_Tick();                                                   // record the DHCP process start time
        }
      else                                                                                //
        {
        ip            = 0;                                                                // update the IP settings to the static IP and port
        cmdDhcpFlag   = 0;                                                                // set the DHCP initailization flag to bypass
        }
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          start DHCP process loop                                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      DELAY_Ms(1000);                                                                     // wait 1S to give the internal hardware time to start up
      while(cmdDhcpFlag == 1)                                                             // check the DHCP flag to see if the DHCP process has completed
        {
        printd("DHCP response -- ");
        if (DHCP_GetState() > DHCP_IDLE_STATE)                                            // see if the DHCP process is in the beginning idle state
          { DHCP_Send(); }                                                                // send a DHCP request to the DHCP host
        else                                                                              // the DHCP process is not in IDLE, execute the next step
          { 
          ulong ip = STOE_GetIPAddr();                                                    // reteieve the DHCP host proposed IP
          if (ip == 0)                                                                    // make sure the proposed IP was not invalid
            printd(" fail!\n\r");                                                         // let the host know that the DHCP process failed
          cmdDhcpFlag = 0;                                                                // set the DHCP process flag to complete
          }//if (DHCP_GetState() > DHCP_IDLE_STATE)
        swtc = processEthernetTraffic(0, ON);                                             // check for new Ethernet traffic and process new data
        }//while(cmdDhcpFlag == 1)do
      UpdateIpSettings(ip);                                                               // update IP address, subnet mask, gateway IP address and DNS IP address
      printd("*****************************************************\n\r");
      }
      return swtc;                                                                        // return the software timer count
}

//----------------------------------------------------------------------------------------
// name    : slaveResetReq                                                                //
// purpose : send the firmware message to the slave processors                            //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  slaveResetReq(void)                                                                 //
{
      UART1_PutStr(&bufResetReq[ix01], m2sResetMsgLength);                                // send the slave reset broadcast message
}

//----------------------------------------------------------------------------------------
// name    : slaveFirmwareReq                                                             //
// purpose : send the firmware message to the slave processors                            //
// params  : id of the slave processor to talk with                                       //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  slaveFirmwareReq(void)                                                              //
{
      UART1_PutStr(&bufFirmwareReq[ix01], m2sFirmwareMsgLength);                          // send the slave 0 unicast firmware request
}

//----------------------------------------------------------------------------------------
// name    : slaveStatusReq                                                               //
// purpose : send the firmware message to the slave processors                            //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  slaveStatusReq(void)                                                                //
{
      UART1_PutStr(&bufStatusReq[ix01], m2sStatusMsgLength);                              // send the slave reset broadcast message
}

//----------------------------------------------------------------------------------------
// name    : slaveIndicatorMsg                                                            //
// purpose : send the firmware message to the slave processors                            //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  slaveIndicatorMsg(void)                                                             //
{
      UART1_PutStr(&bufIndicatorMsg[ix01], m2sIndicatorMsgLength);                        // send the slave reset broadcast message
}

//----------------------------------------------------------------------------------------
// name    : slaveDimmingMsg                                                              //
// purpose : send the firmware message to the slave processors                            //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  slaveDimmingMsg(void)                                                               //
{
     
      UART1_PutStr(&bufDimmingMsg[ix01], m2sDimmingMsgLength);                            // send the slave reset broadcast message
}

//----------------------------------------------------------------------------------------
// name    : slaveDisplayMsg                                                              //
// purpose : send the firmware message to the slave processors                            //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void  slaveDisplayMsg(void)                                                               //
{
      UART1_PutStr(&bufDisplayMsg[ix01], m2sDisplayMsgLength);                            // send the slave reset broadcast message
}

//----------------------------------------------------------------------------------------
// name    : main                                                                         //
// purpose : execute the main loop of the program as long as powered                      //
// params  : void                                                                         //
// returns : void                                                                         //
// note    :                                                                              //
//----------------------------------------------------------------------------------------
void main(void)
{
      byte  sysInitFlag      = 1;                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      ulong preTimeCount;                                                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      ulong temp32           = 0;                                                         //
      word  t16              = 0;                                                         //
      byte  idx              = 0;                                                         //
      int   i                = 0;
//      byte  tBug = 0;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          Start Main Code                                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      ExecuteRuntimeFlag     = 1;                                                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          system initiation                                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      AX11000_Init();                                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          uart interface initiation                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      uart_Init(0,Baud___9600);                                                           // setup port 0 for 8,1,1n, and baudrate
      uart_Init(1,0);                                                                     // setup port 1 for 8,1,1n, and 9600bps
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          uart interface initiation                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      printd("Instrument Part Number  : %s\n\r",SIMTEK_Inst_Number);                      //
      printd("Firmware Last Revised   : %s %s\n\r",__DATE__,__TIME__);                    // c:\tftpd64
      printd("Ethernet Library Ver.   : %s\n\r", GCONFIG_VERSION_STRING);                 //
      printd("*****************************************************\n\r");                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          uart interface initiation                                                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      printd("UART-0   init           : pass    ");                                       //
      printd("baud=%2bx \n\r",Baud___9600);                                               // show the user the baudrate
      printd("UART-1   init           : pass\n\r");                                       //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initialize harware                                                            //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      acResetRequest = 1;                                                                 //
      processResetRsp();                                                                  //
      LogicRst = 1;                                                                       // activate FPGA reset
      DELAY_Ms(5);                                                                        // wait for the FPGA to see the reset signal
      LogicRst = 0;                                                                       // activate FPGA reset
      DELAY_Ms(5);                                                                        // wait for the FPGA to see the reset signal
      DELAY_Ms(5);                                                                        // wait for the FPGA to see the reset signal
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initailize buffers for interprocessor communication                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      memset(&mRxBuf[0], 0, LargestMsgLen);                                               // initailize the received message buffer to blank
      mRxCnt                    = 0;                                                      // initailize the received data byte count to zero
      realtimeswblock           = ON;                                                     // turn the realtime switch reports block on
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initailize slave devices                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      slaveResetReq();                                                                    //
      DELAY_Ms(500);                                                                      // give the slave processors time to reset
      slaveFirmwareReq();                                                                 // request slave 0 firmware number
      DELAY_Ms(35);                                                                       // wait for the slave to respond
      slaveStatusReq();                                                                   // request the status of slave 0 user inputs
      DELAY_Ms(50);                                                                       // wait for the slave to respond
      printd("Hardware init           : pass\n\r");                                       //
      MinMsgLen = DefaultMsgLen;                                                          // set the minimum message length

      HardwareSettings = 0;                                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initailize the device configurationnitialization                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      preTimeCount = setupEthernet(TRUE);                                                 // initailize and start all of the Ethernet processes
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          display device configuration data                                             //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      printDeviceConfigurationInformation();                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initialize the action counters to zero                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      stage                     = 0;                                                      // set the stage to the first status request
      stagetimeout              = 1;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          initialize the menu for debug port operations                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      UTIL_MenuInit();                                                                    //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          start main loop                                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
      while(1)                                                                            // start main loop
        {                                                                                 // begin
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          begin action request processing for debug commands                            //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        UTIL_MenuExecute();                                                               //
        P1_6                    = 0;                                                      //
        P1_7                    = 1;                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          process any received commands from the host                                   //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        processResetRsp();                                                                // reset the internal registers
        processFirmwareRsp();                                                             //
        processStatusRsp();                                                               //
        processDimmingRsp();                                                              //
//        printd("%03bu  \r",tBug);
//        tBug += 1;
//
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          the following routines implement the RSxxx interface between                  //
//          the device and the host system                                                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        processSerialData();                                                              // check serial rx buffer and process any messages
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          look for changes in the switch data, and process realtime messages            //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        if(stagetimeout == 1)
          {    slaveStatusReq();            stagetimeout = 5000;        }                 //
        else
          {          stagetimeout--;        }                                             // make sure the cycle continues evan if slave does not respond
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          process any recieved Ethernet data                                            //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
        preTimeCount = processEthernetTraffic(preTimeCount, OFF);                         // check for new Ethernet traffic and process new data
        GS2E_Task();                                                                      // retrieve and process data
        }//while(1)
} // End of main() //