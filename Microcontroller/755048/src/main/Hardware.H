//========================================================================================//
//     Copyright (c) 2017     Simtek, Incorporated      All rights reserved.              //
//                                                                                        //
//     This is unpublished proprietary source code of Simtek, Inc                         //
//                                                                                        //
//     The copyright notice above does not evidence any actual or intended                //
//     publication of such source code.                                                   //
//========================================================================================//
//========================================================================================//
//========================================================================================//
// Module Name     : Hardware.H                                                           //
// Purpose         : Definition of universal variables                                    //
// Author          : C. Mason                                                             //
// Date            : Jult 31, 2017                                                        //
// Notes           :                                                                      //
//========================================================================================//
//========================================================================================//
// Simtek Revision : -                                                                    //
//========================================================================================//
#include "INSTRUMENT.H"                                                                   //
//========================================================================================//
//                NAMING CONSTANT DECLARATIONS                                            //
//========================================================================================//

//========================================================================================//
//                GLOBAL VARIABLES                                                        //
//========================================================================================//
extern bool  fNormalRun;                                                                  //
extern byte  HalfDuplex;                                                                  //
////////////////////////////////////////////////////////////////////////////////////////////
//          GLOBAL VARIABLES                                                              //
////////////////////////////////////////////////////////////////////////////////////////////
extern word  acResetRequest;                                                              //
extern word  acFirmwareRequest;                                                           //
extern word  acStatusRequest;                                                             //
extern word  acDimmingRequest;                                                            //

extern bool  fDebugMode;                                                                  //
extern bool  fTransmitMode;                                                               //

extern byte  bufResetReq[bufResetMsgLength];                                              //
extern byte  bufStatusRsp[bufStatusMsgLength];                                            //
extern byte  bufStatusReq[bufStatusMsgLength];                                            //
extern byte  bufIndicatorMsg[bufIndicatorMsgLength];                                      //
extern byte  bufDimmingMsg[bufDimmingMsgLength];                                          //
extern byte  bufDimmingTmp[bufDimmingMsgLength];                                          //
extern byte  bufDisplayMsg[bufDisplayMsgLength];                                          //
extern byte  bufFirmwareReq[bufFirmwareMsgLength];                                        //
extern byte  bufFirmwareRsp[bufFirmwareMsgLength];                                        //
