//========================================================================================//
//========================================================================================//
//========================================================================================//
//     Copyright (c) 2018     Simtek, Incorporated      All rights reserved.              //
//                                                                                        //
//     This is unpublished proprietary source code of Simtek, Inc                         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//========================================================================================//
//========================================================================================//
//========================================================================================//
// Module Name     : revision.h                                                           //
// Purpose         : definition of firmware number.date.release.rev.build                 //
// Author          : C. Mason                                                             //
// Date            : April 20, 2018                                                       //
// Notes           :                                                                      //
//========================================================================================//
//========================================================================================//
//========================================================================================//
#ifndef INSTRUMENT_H                                                                      //
#define INSTRUMENT_H                                                                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          simtek microcontroller date and firmware revision number                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define SIMTEK_Inst_Number             "10-8665"                                          //
#define defaultServerStaticIP        0xC0A8885A                                           //
#define defaultClientDestIP          0xC0A88801                                           //
#define defaultsubnetmask            0xFFFFFF00                                           //
#define defaultDNSServer             0xa85f0101                                           //
#define defaultGateway               0xc0a80001                                           //
#define defaultDataPort                   51020                                           //
#define defaultHostPort                   51021                                           //
#define defaultDeviceName       "10_8665_01_1XX";                                         //
#define a2dCh1Max                          4095                                           //
#define a2dCh1Min                             0                                           //
#define a2dCh1Offset                         10                                           //
#define a2dCh1MaxReading                   3866                                           //
#define a2dCh1scalefactor     1.0619813278008298755186721991701                           // a2dCh1Max/(a2dCh1MaxReading - a2dCh1Offset)
#define DebugBuild                            0                                           //
// GLOBAL VARIABLES //
extern  byte DefaultDevName[16];                                                          //
extern uint  PortLeaseTimeoutVal;                                                         //
#define PortLeaseTimeout                  34000                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          port pin name static definition                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define PORT0_PIN_NAMES       "P0_0,P0_1,P0_2,P0_3,P0_4,P0_5,P0_6,P0_7,"                  //
#define PORT1_PIN_NAMES       "P1_0,P1_1,P1_2,P1_3,P1_4,P1_5,P1_6,P1_7,"                  //
#define PORT2_PIN_NAMES       "P2_0,P2_1,P2_2,P2_3,P2_4,P2_5,P2_6,P2_7,"                  //
#define PORT3_PIN_NAMES       "P3_0,P3_1,P3_2,P3_3,P3_4,P3_5,P3_6,P3_7,\0"                //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          host message header                                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define StartByte                          0xFF                                           // start byte
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          host command codes to the instrument                                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define ResetRequest                       0xF0                                           //
#define StatusRequest                      0xF1                                           //
#define IndicatorRequest                   0xF3                                           //
#define DimmingRequest                     0xF4                                           //
#define DisplayRequest                     0xF5                                           //
#define IntailizeRequest                   0xFD                                           //
#define FirmwareRequest                    0xFE                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          default message buffer definitions                                            //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                                           01  02  03  04  05  06  07  08  09  10  11  12  13  14  15  16  17  18  19
#define resetdefault                      "\xF0"                                          // F0
#define firmwaredefault                   "\xFE\x75\x50\x48\x2d\x00\x00\x00\x00\x00\x00\x00\x00"          // FE x9
#define statusdefault                     "\xF1\x00\x00\x00\x00\x00"                      // F1 x6
#define indicatordefault                  "\xF3\x00"                                      // F3
#define dimmingdefault                    "\xF4\x00\x00"                                  // F4
#define displaydefault                    "\xF5\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00"// F5
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          lengths of messages sent from and to the host                                 //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define m2sResetMsgLength                  sizeof(resetdefault) - 1                       // 
#define m2sFirmwareMsgLength               1                                              // 
#define m2sStatusMsgLength                 1                                              //
#define m2sIndicatorMsgLength              sizeof(indicatordefault) - 1                   //
#define m2sDimmingMsgLength                sizeof(dimmingdefault) - 1                     //
#define m2sDisplayMsgLength                sizeof(displaydefault) - 1                     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define s2mFirmwareMsgLength               sizeof(firmwaredefault) - 1                    // 
#define s2mFirmwareRspLength               s2mFirmwareMsgLength - 4                       // 
#define s2mStatusMsgLength                 sizeof(statusdefault) - 1                      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define IntailizeRequestLen                7                                              //
#define DefaultMsgLen                      sizeof(statusdefault) - 1                      // smallest of slave response messages
#define LargestMsgLen                      (sizeof(firmwaredefault)+sizeof(statusdefault))*2                        // 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// buffer length longest message between the request message and the response message     //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define bufResetMsgLength                  sizeof(resetdefault)                           // 
#define bufFirmwareMsgLength               sizeof(firmwaredefault)                        // 
#define bufStatusMsgLength                 sizeof(statusdefault)                          //
#define bufIndicatorMsgLength              sizeof(indicatordefault)                       //
#define bufDimmingMsgLength                sizeof(dimmingdefault)                         //
#define bufDisplayMsgLength                sizeof(displaydefault)                         //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          A2D number of samples to take and number of loops to delay between reads      //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define a2dsamples                            4                                           //
#define a2ddelay                             20                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                                                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define FPGADataLen                          20                                           // 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          data array index poinx                                                        //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define ix01                                  0                                           //
#define ix02                                  1                                           //
#define ix03                                  2                                           //
#define ix04                                  3                                           //
#define ix05                                  4                                           //
#define ix06                                  5                                           //
#define ix07                                  6                                           //
#define ix08                                  7                                           //
#define ix09                                  8                                           //
#define ix10                                  9                                           //
#define ix11                                 10                                           //
#define ix12                                 11                                           //
#define ix13                                 12                                           //
#define ix14                                 13                                           //
#define ix15                                 14                                           //
#define ix16                                 15                                           //
#define ix17                                 16                                           //
#define ix18                                 17                                           //
#define ix19                                 18                                           //
#define ix20                                 19                                           //
#define ix21                                 20                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          debug routines message constants                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define DebugStrLen                        0x10                                           // debug commands response string length
#define dbCmd                              ix07                                           // define the position of the debug response command byte
#define dbDat01                            ix08                                           // define the position of the 1st debug response data byte
#define dbDat02                            ix09                                           // define the position of the 2nd debug response data byte
#define dbDat03                            ix10                                           // define the position of the 3rd debug response data byte
#define dbDat04                            ix11                                           // define the position of the 4th debug response data byte
#define DebugDefStrLen                        8                                           // define the number of bytes to send for default command response
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          realtime message array index points                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define rt01                               ix02                                           //
#define rt02                               ix03                                           //
#define rt03                               ix04                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                bit masks                                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//#define DebugModeMask                      0x04                                           //
//#define TransmitModeMask                   0x08                                           //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          debug routines message constants                                              //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
static byte oP0;                                                                          //
static byte oP1;                                                                          //
static byte oP2;                                                                          //
static byte oP3;                                                                          //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//          realtime switch action codes                                                  //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define     SwXxPosX                       0x0B                                           // example
#endif      // end of INSTRUMENT_H