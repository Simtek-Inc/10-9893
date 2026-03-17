//========================================================================================//
//     Copyright (c) 2006     ASIX Electronic Corporation      All rights reserved.       //
//                                                                                        //
//     This is unpublished proprietary source code of ASIX Electronic Corporation         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//========================================================================================//
//
//========================================================================================//
// Module Name     : gudpdat.c                                                            //
// Purpose         :                                                                      //
// Author          :                                                                      //
// Date            :                                                                      //
// Notes           :                                                                      //
// $Log            : gudpdat.c,v $                                                        //
//                                                                                        //
//========================================================================================//

//========================================================================================//
//          include file declarations                                                     //
//========================================================================================//
#include "main.h"                                                                         //
#include "adapter.h"                                                                      //
#include "gudpdat.h"                                                                      //
#include "gs2e.h"                                                                         //
#include "gconfig.h"                                                                      //
#include "tcpip.h"                                                                        //
#include "mstimer.h"                                                                      //
#include "stoe.h"                                                                         //
#include "uart.h"                                                                         //
//#include "hsuart.h"                                                                       //
#include "printd.h"                                                                       //
#include <string.h>                                                                       //
#include "reg80390.h"                                                                     //
#include "delay.h"                                                                        //
#include "Hardware.H"                                                                     //
#include "ax11000.h"                                                                      //
#include "intrins.h"                                                                      //
#include "instrument.h"                                                                   //
//#include "main.h"                                                                         //
//========================================================================================//
//                naming constant declarations                                            //
//========================================================================================//
#define GUDPDAT_MAX_DATA_LEN              1472                                            // maximun UDP payload length
#define GUDPDAT_MAX_CONNS                 5                                               // maximun UDP connections, current version only supports 
                                                                                          // one connection
#define GUDPDAT_NO_NEW_CONN               0xFF                                            //
#define GUDPDAT_STATE_FREE                0                                               //
#define GUDPDAT_STATE_CONNECTING          1                                               //
#define GUDPDAT_STATE_WAIT                2                                               //
#define GUDPDAT_STATE_CONNECTED           3                                               //
                                                                                          //
#define GUDPDAT_CLOSE_INDICATOR           0x30                                            //
#define GUDPDAT_CONNECT_INDICATOR         0x31                                            //
#define GUDPDAT_DATA_INDICATOR            0x32                                            //
#define GUDPDAT_FLOW_CONTROL_INDICATOR    0x33                                            //
#define GUDPDAT_MAIN_CONNECT_TIME         (3 * 60 * 100)                                  // 3 minutes //
                                                                                          //
//----------------------------------------------------------------------------------------//
//                global variables declarations                                           //
//----------------------------------------------------------------------------------------//
extern byte gtcpdat_TxBuf[];                                                              //
extern byte uip_findarptable(word* ipaddr);                                               //
//----------------------------------------------------------------------------------------//
//                local variables declarations                                            //
//----------------------------------------------------------------------------------------//
static GUDPDAT_CONN gudpdat_Conns[GUDPDAT_MAX_CONNS];                                     //
byte gudpdat_TxBuff[GUDPDAT_TX_BUF_SIZE];                                                 //
static byte  gudpdat_InterAppId;                                                           //
static byte  gudpdat_LastConnId;                                                           //
static word  gudpdat_TxDatLen;                                                             //
static ulong gudpdat_TxWaitTime;                                                          //
static byte *gudpdat_TxBuf = gudpdat_TxBuff;                                              //
static word  gudpdat_Port;                                                                 //
static word  gudpdat_EthernetTxTimer;                                                      //
static byte  gudpdat_UdpClient;                                                            //
static ulong elapse, time;                                                                //
static byte  txBuf[6];                                                                     //
//----------------------------------------------------------------------------------------//
//                local subprogram prototype declarations                                 //
//----------------------------------------------------------------------------------------//
static byte gudpdat_CheckAllConnStatusIdle(void);                                         //
static void gudpdat_MaintainConnection(byte id);                                          //
static void gudpdat_SendData(byte id);                                                    //
static word gudpdat_GetData(byte id);                                                     //
static void gudpdat_Client(byte id);                                                      //
       word GUDPDAT_GetRandomPortNum(void);                                               //
static word gudpdat_GetData(byte id);                                                     //
//----------------------------------------------------------------------------------------//
//                global subprogram prototype declarations                                //
//----------------------------------------------------------------------------------------//
word  main_readEthChar(byte* pDat);                                                       //
//========================================================================================//
//== -                       local subprogram definitions                             - ==//
//========================================================================================//

// ---------------------------------------------------------------------------------------//
// Function Name : GUDPDAT_GetRandomPortNum                                               //
// Purpose       :                                                                        //
// Params        :                                                                        //
// Returns       :                                                                        //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
word GUDPDAT_GetRandomPortNum(void)
{
      word port;

      port = (word)SWTIMER_Tick();

      if (port < 5000)
        port += 5000;
      return port;
} // End of GUDPDAT_GetRandomPortNum() //

// ---------------------------------------------------------------------------------------//
// Function Name : gudpdat_ClientOpen()                                                   //
// Purpose       : create a client connection with the host                               //
// Params        : socket id                                                              //
// Returns       : void                                                                   //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void gudpdat_ClientOpen(byte id)
{
      gudpdat_Client(id);
      if (gudpdat_Conns[id].State == GUDPDAT_STATE_CONNECTING)
        gudpdat_Conns[id].State = GUDPDAT_STATE_CONNECTED;
}

// ---------------------------------------------------------------------------------------//
// Function Name : gudpdat_Client()                                                       //
// Purpose       : create a client connection with the host                               //
// Params        : socket id                                                              //
// Returns       : void                                                                   //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void gudpdat_Client(byte id)
{
      ulong dip;
      byte tmp;

      if (GS2E_GetTaskState() != GS2E_STATE_UDP_DATA_PROCESS)
        {
        word   dport = GCONFIG_GetClientDestPort();                                       // get the host server port number
        dip = GCONFIG_GetClientDestIP();                                                  // get the host server ip address
        // get destination ip //
        if (dip == 0)
          return;
        if (gudpdat_Conns[id].State == GUDPDAT_STATE_FREE)                                // confirm that the socket is free
          {
          gudpdat_Conns[id].State = GUDPDAT_STATE_CONNECTING;                             // set the task state to connecting
          gudpdat_Conns[id].Timer = SWTIMER_Tick();                                       // save the current time count
          gudpdat_Conns[id].Ip = dip;                                                     // save the IP address
          gudpdat_Conns[id].Port = dport;                                                 // save the port number
          gudpdat_Conns[id].UdpSocket = TCPIP_UdpNew(gudpdat_InterAppId,                  //
                                                     id,                                  // socket id
                                                     dip,                                 // host ip address
                                                     GUDPDAT_GetRandomPortNum(),          // random port
                                                     dport);                              // host server port
          // Send ARP request to build IP/MAC entry in ARP table //
          if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (dip & STOE_GetSubnetMask()))  //
            dip = STOE_GetGateway();
          ETH_SendArpRequest(dip);                                                        // send an ARP message to the host
          GS2E_SetTaskState(GS2E_STATE_UDP_DATA_PROCESS);                                 // set the task state
          tmp = printd("UDP host client socket init ");                                   //
          if(gudpdat_Conns[id].UdpSocket == TCPIP_NO_NEW_CONN)                            // check to make sure the socket was created
            printd("**ERROR! UNABLE TO CREATE SOCKET**\n\r");                             // let the user know the the port is not initailized, there was an error
          else
            {
            printd("ok. port : %du\n\r", (word) dport);                                   // print the port number
            }          
          }
        }
}

// ---------------------------------------------------------------------------------------//
// Function Name : gudpdat_GetData()                                                      //
// Purpose       : transfer data from the main loop buffer into the TOE buffer            //
// Params        : socket id                                                              //
// Returns       : the number of bytes still left in the main loop buffer                 //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
word gudpdat_GetData(byte id)
{
      byte dat;
      word temp;

      temp = readEthTxBufCount();
      while (temp > 0)
        {
        if (gudpdat_TxDatLen >= GUDPDAT_MAX_DATA_LEN)
          break;
        temp = main_readEthChar(&dat);
        gudpdat_TxBuf[gudpdat_TxDatLen++] = dat;
        gudpdat_Conns[id].UrRxBytes++;
        }
      return temp;
} // End of gudpdat_GetData() //

// ---------------------------------------------------------------------------------------//
// Function Name : gudppdat_MaintainConnection()                                          //
// Purpose       : Maintain connection with host                                          //
// Params        : socket id                                                              //
// Returns       : void                                                                   //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void gudpdat_MaintainConnection(byte id)
{
      ulong time;//,dip;
      time = SWTIMER_Tick();
      
// Maintain the connection: send ARP request each GUDPDAT_MAIN_CONNECT_TIME //
      if (time >= gudpdat_Conns[id].Timer)
        elapse = time - gudpdat_Conns[id].Timer;
      else
        elapse = (0xFFFFFFFF - gudpdat_Conns[id].Timer) + time;
      
      if (elapse >= GUDPDAT_MAIN_CONNECT_TIME)
        {
        if ((STOE_GetIPAddr() & STOE_GetSubnetMask()) != (gudpdat_Conns[id].Ip & STOE_GetSubnetMask()))
          {
          ETH_SendArpRequest(STOE_GetGateway());
          }
        else
          {
          ETH_SendArpRequest(gudpdat_Conns[id].Ip);
          }
        gudpdat_Conns[id].Timer = time;
        }
} // End of gudpdat_MaintainConnection() //

// ---------------------------------------------------------------------------------------//
// Function Name : gudpdat_SendData()                                                     //
// Purpose       : send any data to the host                                              //
// Params        : socket id                                                              //
// Returns       : void                                                                   //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void gudpdat_SendData(byte id)
{
      ulong elapse, time;

      if (gudpdat_TxDatLen >= GUDPDAT_MAX_DATA_LEN)
        {
        TCPIP_UdpSend(gudpdat_Conns[id].UdpSocket, 0, 0, gudpdat_TxBuf, GUDPDAT_MAX_DATA_LEN);
        gudpdat_TxDatLen = 0;                                                             //
        gudpdat_TxWaitTime = 0;
        }
      else if (gudpdat_TxDatLen > 0)
        {
        time = SWTIMER_Tick();
        if (gudpdat_TxWaitTime == 0)
          { gudpdat_TxWaitTime = time; }
        else
          {
          if (time >= gudpdat_TxWaitTime)
            elapse = time - gudpdat_TxWaitTime;
          else
            elapse = (0xFFFFFFFF - gudpdat_TxWaitTime) + time;
          if ((elapse * SWTIMER_INTERVAL) >= gudpdat_EthernetTxTimer)
            {
            TCPIP_UdpSend(gudpdat_Conns[id].UdpSocket, 0, 0, gudpdat_TxBuf, gudpdat_TxDatLen);
            gudpdat_TxDatLen   = 0;
            gudpdat_TxWaitTime = 0;
            }
          }
        }
} // End of gudpdat_SendData() //

// ---------------------------------------------------------------------------------------//
// Function Name : GUDPDAT_Task                                                           //
// Purpose       :                                                                        //
// Params        :                                                                        //
// Returns       :                                                                        //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void GUDPDAT_Task(void)                                                                   //
{
      byte id;                                                                            //
      word temp;                                                                          //

      for (id = 0; id < GUDPDAT_MAX_CONNS; id++)                                          //
        {
        switch (gudpdat_Conns[id].State)                                                  //
          {
          case GUDPDAT_STATE_FREE :                                                       //
              if (gudpdat_UdpClient == TRUE)                                              //
                {
                gudpdat_Client(id);                                                       //
                }
              break;                                                                      //
          case GUDPDAT_STATE_CONNECTING :                                                 //
              if (gudpdat_UdpClient == TRUE)                                              //
                {
                gudpdat_Conns[id].State = GUDPDAT_STATE_CONNECTED;                        //
                }
              break;                                                                      //
          case GUDPDAT_STATE_CONNECTED :                                                  //
              {
              if(fTransmitMode == Server)                                                 // check for a not a num response
                {
                temp = gudpdat_GetData(id);                                               //
                gudpdat_SendData(id);                                                     //
                }
              else                                                                        //
                {
                temp = gudpdat_GetData(TrxSocket);                                        //
                gudpdat_SendData(TrxSocket);                                              //
                }
              gudpdat_MaintainConnection(id);                                             //
              }
              break;                                                                      //
          default:                                                                        //
              break;                                                                      //
          }
        }
} // End of GUDPDAT_Task() //

// ---------------------------------------------------------------------------------------//
// Function Name : GUDPDAT_Init()                                                         //
// Purpose       : Initialization                                                         //
// Params        :                                                                        //
// Returns       :                                                                        //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void GUDPDAT_Init(word localPort)                                                         //
{
      byte  i;                                                                            //

      gudpdat_UdpClient = FALSE;                                                          //
      for (i = 0; i < GUDPDAT_MAX_CONNS; i++)                                             //
        {
        gudpdat_Conns[i].State = GUDPDAT_STATE_FREE;                                      //
        gudpdat_Conns[i].UrRxBytes = 0;                                                   //
        gudpdat_Conns[i].UrTxBytes = 0;                                                   //
        }
      gudpdat_LastConnId = 0;                                                             // make sure the last connection starts off with the first socket
      gudpdat_InterAppId = TCPIP_Bind(GUDPDAT_NewConn, GUDPDAT_Event, GUDPDAT_Receive);   // bind the UDP functions to the TOE
      // unicast packet //
      printd("UDP data ");                                                                // let the user know the the port is initailized
	if (gudpdat_InterAppId == TCPIP_NO_NEW_CONN)                                        // check if the binding failed
        printd("port           : **BINDING ERROR!**\n\r");                                                 // let the user know the the port is initailized
      else
        {
        if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_CLIENT) != GCONFIG_NETWORK_CLIENT)    // is the TOE going to setup as a Client or Server
          {
          i = TCPIP_UdpListen(localPort, gudpdat_InterAppId);                             // setup the local Port
          if(i == 1)
            printd("server port    : %du\n\r",localPort);                                 // set listen socket completed!
          else
          printd("server port    : * BINDING ERROR! *\n\r");                                               // set listen socket FAILED! socket ERROR
          }
        else                                                                              //
          {
          gudpdat_UdpClient = TRUE;                                                       // set the Client flag for the firmware
          printd("client port      : %du\n\r",localPort);                                 // let the user know the port is setup as a Client
          }
        }
      gudpdat_TxDatLen = 0;                                                               // initailize the received data array index to the first cell
      gudpdat_TxWaitTime = 0;                                                             // initailize the process timeout
      gudpdat_EthernetTxTimer = GCONFIG_GetEthernetTxTimer();                             // set the start time
} // End of GUDPDAT_Init() //

// ---------------------------------------------------------------------------------------//
// Function Name : GUDPDAT_NewConn                                                        //
// Purpose       :                                                                        //
// Params        :                                                                        //
// Returns       :                                                                        //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
byte GUDPDAT_NewConn(ulong XDATA* pip, word remotePort, byte socket)                      //
{
      byte i;                                                                             //

      gudpdat_LastConnId = 0;                                                             // make sure the last connection starts off with the first socket

      if (gudpdat_Conns[gudpdat_LastConnId].State == GUDPDAT_STATE_CONNECTED)             //
        {
        gudpdat_Conns[gudpdat_LastConnId].State = GUDPDAT_STATE_FREE;                     //
        TCPIP_UdpClose(gudpdat_Conns[gudpdat_LastConnId].UdpSocket);                      //
        }
      for (i = 0; i < GUDPDAT_MAX_CONNS; i++)                                             //
        {
        if (gudpdat_Conns[i].State == GUDPDAT_STATE_FREE)                                 //
          {
          GS2E_SetTaskState(GS2E_STATE_UDP_DATA_PROCESS);                                 //
          gudpdat_Conns[i].State = GUDPDAT_STATE_CONNECTED;                               //
          gudpdat_Conns[i].Timer = SWTIMER_Tick();                                        //
          gudpdat_Conns[i].Ip = *pip;                                                     //
          gudpdat_Conns[i].Port = remotePort;                                             //
          gudpdat_Conns[i].UdpSocket = socket;                                            //
          printd("New UDP connection : id=%bu, remotePort=%du, socket=%bu\n\r",           //
                  i, remotePort, socket);                                                 //
          gudpdat_LastConnId++;                                                           //
          gudpdat_LastConnId = gudpdat_LastConnId & 0x03;                                 //
          return i;                                                                       //
          }
        }
      return GUDPDAT_NO_NEW_CONN;                                                         //
} // End of GUDPDAT_NewConn() //

// ---------------------------------------------------------------------------------------//
// Function Name : GUDPDAT_Event                                                          //
// Purpose       :                                                                        //
// Params        :                                                                        //
// Returns       :                                                                        //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void GUDPDAT_Event(byte id, byte event)                                                   //
{
      if (event == TCPIP_CONNECT_ACTIVE)                                                  //
        {
        }
      else if (event == TCPIP_CONNECT_CANCEL)                                             //
        {
        gudpdat_Conns[id].State = GUDPDAT_STATE_FREE;                                     //
        GS2E_SetTaskState(GS2E_STATE_IDLE);                                               //
        printd("TCPIP_UdpClose(id=%bu, socket=%bu)\n\r", id, gudpdat_Conns[id].UdpSocket);//
        TCPIP_UdpClose(gudpdat_Conns[id].UdpSocket);                                      //
        }
} // End of GUDPDAT_Event() //

// ---------------------------------------------------------------------------------------//
// Name          : GUDPDAT_Receive                                                        //
// Purpose       : off load the data from the TOE                                         //
// Params        : pointer to data, number of bytes in buffer, port id                    //
// Returns       : void                                                                   //
// Note          :                                                                        //
// ---------------------------------------------------------------------------------------//
void GUDPDAT_Receive(byte XDATA* pData, word length, byte id)                             //
{
      word        i;                                                                      //
      byte        EthRxBuf[128];                                                          //
      byte        Chngs;                                                                  //

      if (gudpdat_Conns[id].State != GUDPDAT_STATE_CONNECTED)                             // make sure that there is a open port before copying the data packet
        return;                                                                           // return to parent routine
      memcpy(&EthRxBuf[0], pData, length);                                                // copy the new data from the ethernet buffer to the received buffer
      gudpdat_Conns[id].UrTxBytes += length;                                              // update pointer to recieved data
      i     = 0;                                                                          // initailize index variable
      Chngs = 0;                                                                          // make sure change flag is zero before continuing
      while(i < length)                                                                   // keep going until the whole ethernet packet is processed
        {
        switch(EthRxBuf[i])                                                               // check the current byte to see if it is a command byte
          {
          case ResetRequest : {
                    if(acResetRequest < 0xffff)                                           // make sure the maximum number of action requests is not pending
                      {acResetRequest++;}                                                 // trigger a reset action
//                    UART1_PutStr(&EthRxBuf[i],m2sResetMsgLength);                         // move command string to transmit buffer
                    memset(&EthRxBuf[i], 0, m2sResetMsgLength);                           // make sure to clear the buffer
                    i += m2sResetMsgLength;  break; }                                     // update byte index to next location
          case StatusRequest : {
                    if(acStatusRequest < 0xffff)                                          // make sure the maximum number of action requests is not pending
                      { acStatusRequest++; }                                              // trigger a status action in the main loop
                    memset(&EthRxBuf[i], 0, m2sStatusMsgLength);                          // make sure to clear the buffer
                    i += m2sStatusMsgLength;  break; }                                    // update byte index to next location
          case IndicatorRequest : {
                    UART1_PutStr(&EthRxBuf[i],m2sIndicatorMsgLength);                     // move command string to transmit buffer
                    memset(&EthRxBuf[i], 0, m2sIndicatorMsgLength);                       // make sure to clear the buffer
                    i += m2sIndicatorMsgLength;  break; }                                 // update byte index to next location
          case DimmingRequest : {
                    if(PortLeaseTimeoutVal == 0) printd("\r                     \r");
                    PortLeaseTimeoutVal = PortLeaseTimeout;                               // set the port lease timeout for an approximately 5 second delay
                    memmove(&bufDimmingTmp[ix01],&EthRxBuf[i],4);                         //

//                    UART1_PutStr(&EthRxBuf[i],m2sDimmingMsgLength);                       // move command string to transmit buffer

                    if(acDimmingRequest < 0xffff)                                         // make sure that the max number of firmware requests is not pending
                      { acDimmingRequest++; }                                             // trigger a firmware action in the main loop
                    memset(&EthRxBuf[i], 0, m2sDimmingMsgLength);                         // make sure to clear the buffer
                    i += m2sDimmingMsgLength;  break; }                                   // update byte index to next location
          case DisplayRequest : {
                    UART1_PutStr(&EthRxBuf[i],m2sDisplayMsgLength);                       // move command string to transmit buffer
                    memset(&EthRxBuf[i], 0, m2sDisplayMsgLength);                         // make sure to clear the buffer
                    i += m2sDisplayMsgLength;  break; }                                   // update byte index to next location
          case FirmwareRequest : {
                    if(acFirmwareRequest < 0xffff)                                        // make sure that the max number of firmware requests is not pending
                      { acFirmwareRequest++; }                                            // trigger a firmware action in the main loop
                    memset(&EthRxBuf[i], 0, m2sFirmwareMsgLength);                        // make sure to clear the buffer
                    i += m2sFirmwareMsgLength;  break; }                                  // update byte index to next location
          default  :
                    EthRxBuf[i] = 0;                                                      // make sure to clear the buffer
                    i++;                                                                  // update byte index to next location
                    break;                                                                // finish processing this case
          }
        }
} // End of GUDPDAT_Receive() //

// End of gudpdat.c //
