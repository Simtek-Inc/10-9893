//========================================================================================
//     Copyright (c) 2005     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//========================================================================================
//========================================================================================
// name     : util_menus.h
// purpose  : 
// author   : Robin Lee
// date     : 2005-03-31
// notes    :
// $Log     : util_menus.h,v $
//----------------------------------------------------------------------------------------
// revision : 1.10  2006/03/20 01:13:50  borbin
//            version V0.14
//----------------------------------------------------------------------------------------
// revision : 1.9  2006/03/13 07:34:40  robin6633
//            Changed the function int UTIL_StrCmp(char *Str1, char *Str2) to export function
//----------------------------------------------------------------------------------------
// revision : 1.8  2005/12/15 03:01:31  borbin
//            version V0.13
//----------------------------------------------------------------------------------------
// revision : 1.7  2005/11/25 10:07:05  borbin
//            version v0.12
//----------------------------------------------------------------------------------------
// revision : 1.6  2005/09/28 10:21:24  borbin
//            version V0.10
//----------------------------------------------------------------------------------------
// revision : 1.5  2005/09/22 03:06:48  robin6633
//            V0.9
//----------------------------------------------------------------------------------------
// revision : 1.4  2005/09/09 09:50:52  robin6633
//            version 0.8
//----------------------------------------------------------------------------------------
// revision : 1.3  2005/08/18 05:37:18  robin6633
//            Change to V0.7
//----------------------------------------------------------------------------------------
// revision : 1.2  2005/08/17 06:48:49  robin6633
//            no message
//----------------------------------------------------------------------------------------
// revision : 1.1  2005/06/14 02:36:20  arthur
//            move directory
//----------------------------------------------------------------------------------------
// revision : 1.1.1.1  2005/06/06 05:55:57  robin6633
//            no message
//========================================================================================

#ifndef UTIL_MENUS_H
#define UTIL_MENUS_H

// INCLUDE FILE DECLARATIONS //

// MACRO DECLARATIONS //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
//                bit masks                                                               //
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~//
#define DebugModeMask                           0x0004                                    //
#define TransmitModeMask                        0x0008                                    //
#define DeadManCntMask                          0x0010                                    //
#define ReportMethodMask                        0x0020                                    //

#define     SIMTEK_DEBUG_UTIL_VER         1.00

#define CHECK_RANGE(Value, Min, Max, ErrMsg) \
{ \
      if ((Value < Min) || (Value > Max)) \
      { \
            printf("%s\n", ErrMsg); \
            return; \
      } \
}

// TYPE DECLARATIONS //
typedef void (*MENU_FUNC)(void *CmdLine);

typedef struct    menu_opt                                                                // structure define to describe each menu item
{
      char              *OptName;                                                         // the menu name
      MENU_FUNC         OptFunc;                                                          // routine name to execute the option
      char              *OptDesc;                                                         // description of the option
}     MENU_OPT;

typedef struct    cmd_menu                                                                // structure define to describe each menu item
{
      char              *Command;                                                         // user command
      char              *Shortcut;                                                        // user command shortcut
      MENU_FUNC          Exe;                                                             // executable
      char              *Description;                                                     // command functional description
}     CMD_MENU;

// GLOBAL VARIABLES //

// EXPORTED SUBPROGRAM SPECIFICATIONS //
bool  UTIL_MenuInit(void);
void  UTIL_MenuExecute(void);
bool  UTIL_MenuSetup(CMD_MENU *NewItem);                                                  // add new menu

void  UTIL_Stooges(void);                                                                 // called when system breaks
char  *UTIL_NextArg(char *Argp);

int   UTIL_WaitKey(void);
bool  UTIL_StrToByte(byte *ConStr, byte *ConRes);
bool  UTIL_StrToWord(byte *ConStr, word *ConRes);
bool  UTIL_StrToULong(byte *ConStr, ulong *ConRes);

int   UTIL_StrCmp(char *Str1, char *Str2);

#endif // End of UTIL_MENUS_H //