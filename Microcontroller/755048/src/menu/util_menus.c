//========================================================================================
//     Copyright (c) 2005     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//========================================================================================
//========================================================================================
// name     : util_menus.c
// purpose  : This menu system is used to alter the device settings through the usb port
// author   : c.mason
// date     : 2018-09-27
// notes    :
//----------------------------------------------------------------------------------------
// revision : 0.0  2018/09/27 c. mason
//========================================================================================

// INCLUDE FILE DECLARATIONS //
#include <stdio.h>
#include "types.h"
#include "reg80390.h"
#include "util_menus.h"
#include "uart.h"
#include "ax11000.h"
#include "delay.h"
#include "si2c.h"
#include "string.h"
#include <stdlib.h>
#include <gconfig.h>
#include <stdio.h>
#include "eeprom.h"
#include "printd.h"                                                                       //
#include "ax11000_cfg.h"                                                                  //
#include "instrument.h"                                                                   //
#include "HARDWARE.h"                                                                     //

// NAMING CONSTANT DECLARATIONS //
#define MAX_MENUS                   50                                                    // maximum menu items
#define CBUFLEN                     128                                                   // size of cbuf
#define BUF_LEN                     64                                                    // 
#define memlen                      30                                                    // 

#define IPv4AddrLen                 4                                                     // 
#define IPv4PortLen                 2                                                     // 
#define len_byte                    1                                                     // 
#define len_name                    16                                                    // 
#define len_macx                    6                                                     // 
#define len_dcbx          (IPv4AddrLen*9)+(IPv4PortLen*11)+(len_byte*4)+len_name+len_macx // 

// EEPROM address offsets
#define pwTimeout                   900000
#define runtimesettings             0x30                                                  // 
#define defaultsettings             0x80                                                  // 
#define rtChecksum                  runtimesettings                                       // length  2 bytes = 2
#define rtCfgLength                 rtChecksum + IPv4PortLen                              // length  2 bytes = 4
#define rtDevName                   rtCfgLength + IPv4PortLen                             // length 16 bytes = 20
#define rtMAC                       rtDevName + len_name                                  // length  6 bytes = 26

#define rtDevServerIpAddr           rtMAC + len_macx                                      // length  4 bytes = 30
#define rtDevClientIPAddr           rtDevServerIpAddr + IPv4AddrLen                       // length  4 bytes = 34
#define rtHostServerIPAddr          rtDevClientIPAddr + IPv4AddrLen                       // length  4 bytes = 38
#define rtHostClientIPAddr          rtHostServerIPAddr + IPv4AddrLen                      // length  4 bytes = 42
#define rtSubNetmask                rtHostClientIPAddr + IPv4AddrLen                      // length  4 bytes = 46
#define rt3Ip                       rtSubNetmask + IPv4AddrLen                            // length  4 bytes = 50
#define rt4Ip                       rt3Ip + IPv4AddrLen                                   // length  4 bytes = 54

#define rtDevServerPort             rt4Ip + IPv4AddrLen                                   // length  2 bytes = 56
#define rtDevUnicastPort            rtDevServerPort + IPv4PortLen                         // length  2 bytes = 58
#define rtDevBroadcastPort          rtDevUnicastPort + IPv4PortLen                        // length  2 bytes = 60
#define rtDevClientPort             rtDevBroadcastPort + IPv4PortLen                      // length  2 bytes = 62
#define rtHostServerPort            rtDevClientPort + IPv4PortLen                         // length  2 bytes = 64
#define rtHostClientPort            rtHostServerPort + IPv4PortLen                        // length  2 bytes = 66
#define rt4Port                     rtHostClientPort + IPv4PortLen                        // length  2 bytes = 68

#define rtBinaryFlags               rt4Port + IPv4PortLen                                 // length  1 bytes = 70
#define rtNetwork                   rtBinaryFlags + len_byte                              // length  2 bytes = 71
#define rtOption                    rtNetwork + IPv4PortLen                               // length  1 bytes = 73
#define rtEthernetTxTimer           rtOption + len_byte                                   // length  2 bytes = 74

const byte                          Check_Value = 0x0a;
// STATIC VARIABLE DECLARATIONS //
static      char              *Prompt    = ">> ";
static      CMD_MENU          *Menus[MAX_MENUS];                                          // app's array of ptrs to menu groups
static      char              *NextCbuf = NULL;                                           // pointer to next available byte in cbuf
static      char              CmdBuf[CBUFLEN];                                            // user command line buffer
static      int               MenuNum = 0;                                                // number of items in Menus[]
//
static      char              *pwPrompt  = "Enter Password ";
static      char              *pwaPrompt = " !!UNSECURE!! ";
//static      byte              enterpw = FALSE;
//            long              pwcnt = 0;
byte        idata             fwAutoUpdated _at_ 0x31;

// LOCAL SUBPROGRAM DECLARATIONS //
static      int               UTIL_DoCmd(void *CmdLine);
static      void              UTIL_MenuShow(CMD_MENU *ptMenu);
static      void              UTIL_MenuHelp(void *CmdLine);
static      void              Simtek_New(void *CmdLine);
static      void              UTIL_FwUpdate(void);
static      int               UTIL_StrCmp(char *Str1, char *Str2);
static      int               UTIL_StrLen(char *Str);
bool                          UTIL_StrToIPv4Num(void *cmdLine, byte *num);
bool                          UTIL_StrToPortNum(void *cmdLine, byte *num);
static      void              UTIL_Checksum(void);
static      void              UTIL_CfgLen(void *cmdLine);
//static      void              UTIL_EraseFlash(void *cmdLine);
static      void              UTIL_EraseEEPROM(void);
// moved to header to make globally accessiable
//bool                          UTIL_PortNumToStr(char *hdr, word *num);
//bool                          UTIL_ByteAryToStr(char *hdr, byte *num, byte len);

static      void              UTIL_WriteI2C(void *cmdLine);
static      void              UTIL_ReadI2C(void *cmdLine);
static      void              UTIL_Fill(void *cmdLine);
static      void              UTIL_Reset(void);
static      void              UTIL_Reboot(void);
static      void              Show_DCB(void);

            void              PortNumber(byte portlen,byte portaddr,char *title, void *cmdLine);
            void              binaryflagcontrol(byte celladdr, word mask, char *title, char *defalt, char *nondefault, byte len, void *cmdLine);
            void              IPv4Address(byte epromaddr,char *Title,void *cmdLine);

static      void              MACAddress(void *cmdLine);
static      void              SubnetMask(void *cmdLine);
static      void              DeviceName(void *cmdLine);
static      void              DHCPClientCtrl(void *cmdLine);
static      void              UDPProtoCtrl(void *cmdLine);
static      void              TCPProtoCtrl(void *cmdLine);
static      void              BroadcastCtrl(void *cmdLine);
static      void              TrxModeCtrl(void *cmdLine);
static      void              ReportMethodCtrl(void *cmdLine);
static      void              UTIL_Rst_EEPROM(void);
static      void              DebugModeCtrl(void *cmdLine);
static      void              DeadmanCntCtrl(void *cmdLine);
static      void              MessageFrames(void);
static      void              Com1RxPtr(void);
static      void              Com1RxBuf(void);

static      void              DeviceIP(void *cmdLine);
static      void              HostIP(void *cmdLine);

static      void              DeviceSIP(void *cmdLine);
static      void              DeviceCIP(void *cmdLine);
static      void              HostCIP(void *cmdLine);
static      void              HostSIP(void *cmdLine);

static      void              DevServerPort(void *cmdLine);
static      void              DevClientPort(void *cmdLine);
static      void              DevUnicastPort(void *cmdLine);
static      void              DevBroadcastPort(void *cmdLine);
static      void              HostServerPort(void *cmdLine);
static      void              HostClientPort(void *cmdLine);

            word              gconfig_Checksum(word *pBuf, ulong length);

static      void              UTIL_Set(void);
static      void              UTIL_PWPrompt(void);
static      void              UTIL_Password(void *cmd);
static      void              UTIL_pwExit(void);

static      CMD_MENU          MainMenu[] = 
{
            {"command",               "shortcut", NULL,        "command functional description"},
//td("%s\n\r ---------------------------- - ----------------------------------------------------\n\r",str);
            {"help",                  "?",   UTIL_MenuHelp,    "display help messages for menus"},
            {"reboot",                "rb",  UTIL_Reboot,      "Reboot the device without changing settings"},
            {"message_frames",        "mf",  MessageFrames,    "output the data arrays holding the host data"},
            {"Uart1_RX_Ptr",          "up1", Com1RxPtr,        "display the UART1 Pointers"},
            {"Uart1_RX_Buf",          "ub1", Com1RxBuf,        "display the UART1 buffer"},

            {NULL,                    NULL,  NULL,             NULL},
            {" ",                     NULL,  UTIL_Stooges,     "set sub-menu"},
            {"mask",                  NULL,  SubnetMask,       "read or set the IP subnet mask"},
            {"device_sip",            "dsip",DeviceSIP,        "read or set the device server IP address"},
            {"device_server_port",    "dsp", DevServerPort,    "read or set the device server port number"},
            {"device_unicast_port",   "dup", DevUnicastPort,   "read or set the device client port number"},
            {"device_broadcast_port", "dbp", DevBroadcastPort, "read or set the device client port number"},
            {"device_cip",            "dcip",DeviceCIP,        "read or set the device client IP address"},
            {"device_client_port",    "dcp", DevClientPort,    "read or set the device client port number"},
            {"host_cip",              "hcip",HostCIP,          "read or set the host client IP address"},
            {"host_client_port",      "hcp", HostClientPort,   "read or set the host client port number"},
            {"host_sip",              "hsip",HostSIP,          "read or set the host server IP address"},
            {"host_server_port",      "hsp", HostServerPort,   "read or set the host server port number"},
            {"deadman_count_reset",   "dmc", DeadmanCntCtrl,   "turn deadman counter reset on/off"},
            {"dhcp_client",           "dhcp",DHCPClientCtrl,   "read or set the DHCP client state"},

            {"set_defaults",          "sd",  UTIL_Set,         "save the current dcb settings as the default dcb"},
            {"firmware_update",       "fwu", UTIL_FwUpdate,    "update firmware"},
            {"fill",                  NULL,  UTIL_Fill,        "fill memory between two addresses with data"},
            {"reset",                 "rst", UTIL_Reset,       "Reset device settings to defaults"},
            {"reset_eeprom",          "rem", UTIL_Rst_EEPROM,  "Reset eprom to the oem settings"},
            {"checksum",              "cs",  UTIL_Checksum,    "recalculate the dcb checksum and save to dcb"},
            {"configuration_length",  "cl",  UTIL_CfgLen,      "change the length of the dcb (max "},
            {"name",                  NULL,  DeviceName,       "read or set the device name"},
            {"mac",                   NULL,  MACAddress,       "read or set the MAC address"},
            {"transmit_mode",         "txm", TrxModeCtrl,      "set the device to host data transmit method"},
            {"report_method",         "irm", ReportMethodCtrl, "set the device user input reporting method"},
            {"debug_mode",            "dbg", DebugModeCtrl,    "turn debug moded on/off"},
            {"show_device_config",    "dcb", Show_DCB,         "show the device configuration block"},
            {"user_datagram_proto",   "udp", UDPProtoCtrl,     "read or set the DHCP client state"},
            {"trans_control_proto",   "tcp", TCPProtoCtrl,     "read or set the DHCP client state"},
            {"broadcast",             "bcc", BroadcastCtrl,    "read or set the DHCP client state"},
//            {"erase",                 "ers", UTIL_EraseFlash,   "erase memory contents, set to 0xFF"},

//            {"password",              NULL,  UTIL_PWPrompt,    "enter password to reach extended function"},
//***************************************************************************************************************************
//    password protected commands 
//***************************************************************************************************************************
//            {"exit",                  "xx",  UTIL_pwExit,      "exit unsecure mode"},
            {"update",                "uu",  UTIL_FwUpdate,    "update firmware"},
            {"read",                  "rd",  UTIL_ReadI2C,     "read I2C EEPROM memory"},
            {"write",                 "wr",  UTIL_WriteI2C,    "write data to the I2C EEPROM"}
};

// LOCAL SUBPROGRAM BODIES //

//----------------------------------------------------------------------------------------
// name      : static int   UTIL_DoCmd(void//CmdLine)
// purpose   : Decide which command is used and jump to its corresponding function.
// params    : void//CmdLine
// returns   : 0:found command ; -1:command error
// note      :
//----------------------------------------------------------------------------------------
static int   UTIL_DoCmd(void *CmdLine)
{
      CMD_MENU    *ptMenuCur = NULL;
      CMD_MENU    *ptMenuTmp = NULL;
      char        *InBuf     = NULL;
      word         Len       = 0;
      word         i;

      ptMenuCur = NULL;                                                                   // clear menu command pointer
      InBuf     = (char *)CmdLine;
      if (InBuf[0] == '?')                                                                // is user after help ?
        {
        UTIL_MenuHelp(CmdLine);                                                           // display the help for the command line argument
        return 0;                                                                         // return a match to calling routine
        }
//      if(enterpw == TRUE)                                                                 // 
//        {
//        UTIL_Password(CmdLine);                                                           // 
//        return 0;                                                                         // return a match to calling routine
//        }
      // scan the various menus for the user command: //
      for (i = 0; i < MenuNum; i ++)                                                      // menu loop, MenuNum initailized to number of items in the menu during setup
        {
        ptMenuTmp = Menus[i];                                                             // get pointer to next menu list
        if (!ptMenuTmp)                                                                   // check that the menu item does not equal a NULL
          {    break;    }                                                                // the menu item does equal NULL, get out of the loop
        ptMenuTmp ++;                                                                     // jump past menu ID
//        while ((ptMenuTmp->Command) && (ptMenuTmp->Command != "set_defaults"))            // scan this menu's for a valid command word
        while ((ptMenuTmp->Command))// || (pwcnt != 0))                                      // scan this menu's for a valid command word
          {      
          Len = UTIL_StrCmp(ptMenuTmp->Command, InBuf);                                   // check for a command match
          if ((Len == 0) || (Len > UTIL_StrLen(InBuf)))                                   // got a match?
            {
            if (ptMenuCur)                                                                // we already had a match
              {
              printd("Ambiguous command: %s \n\r", InBuf);                                // tell the user of the failure
              return -1;                                                                  // return a fail for no menu item match found
              }
            ptMenuCur = ptMenuTmp;                                                        // remember the matching entry
            }
          else                                                                            // the command did not match
            {
            Len = UTIL_StrCmp(ptMenuTmp->Shortcut, InBuf);                                // check for a shortcut match
            if ((Len == 0) || (Len > UTIL_StrLen(InBuf)))                                 // got a match?
              {
              if (ptMenuCur)                                                              // we already had a match
                {
                printd("Ambiguous command: %s \n\r", InBuf);                              // tell the user of the failure
                return -1;                                                                // return a fail for no menu item match found
                }
              ptMenuCur = ptMenuTmp;                                                      // remember the matching entry
              }
            }
          ptMenuTmp ++;                                                                   // move the pointer to the next menu item
          }
        }
      if (!ptMenuCur)                                                                     // check for and undefined pointer
        {
        printd("Unknown command: %s\n\r", InBuf);                                         // let user know no matching command was found
        return -1;                                                                        // let the calling routine know a match was not found
        }
      if (ptMenuCur->Exe)                                                                 // make sure the is a function define for this command
        {   (ptMenuCur->Exe)(CmdLine);   }                                                // there is a function defined for this command, call the function
      return 0;                                                                           // the command was found
}

//----------------------------------------------------------------------------------------
// name      : UTIL_MenuShow
// purpose   : show the util menu to the user
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void UTIL_MenuShow(CMD_MENU *ptMenu)                                                 // menu to display
{
      char  str[30];
      byte  len;
      
      printd("\nSIMTEK INC. device configuration menu (%s %s)\n\r\n\r  ", __DATE2__, __TIME__); 
      ptMenu ++;
      strcpy(str, ptMenu->Command);
      strcat(str, " (");
      strcat(str, ptMenu->Shortcut);
      strcat(str, ")             ");
      strcat(str, ptMenu->Description);
      strcat(str, "\0");
      ptMenu ++;
      printd("%s\n\r ----------------------------   -------------------------------------------\n\r",str);
      while ((ptMenu->Command))
        {
//        if((pwcnt == 0) & (ptMenu->Command == "password"))break;
        strcpy(str, ptMenu->Command);
        if(ptMenu->Shortcut)
          {
          strcat(str, " (");
          strcat(str, ptMenu->Shortcut);
          strcat(str, ")");
          }
        strcat(str, "\0");
        len = strlen(str);  
        printd("  %-27s - %s\n\r", str, ptMenu->Description);
        ptMenu ++;
        }
      printd("\n\r");
}

//----------------------------------------------------------------------------------------
// name      : UTIL_MenuHelp
// purpose   : show the user the help menu
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void UTIL_MenuHelp(void *CmdLine)
{
      char        *Argp = NULL;
      word        Len   = 0;
      word        i;

      // help command line should still be in inbuf //
      Argp = UTIL_NextArg((char *)CmdLine);                                               // get first arg after "help" or '?' //
      // handle simple '?' or 'help' cases first //
      if (!*Argp)                                                                         // check to see if there are no arguments
        {
        UTIL_MenuShow(Menus[0]);
        for (i = 0; i < MenuNum; i++)                                                     // menu loop //
          {
          if (!Menus[i])
            {            break;            }
          }
        return;
        }
      // else - should be a parm after help //
      for (i = 0; i < MenuNum; i++)                                                       // menu loop //
        {
            if (!Menus[i])
              {
                  break;
              }
            Len = UTIL_StrCmp(Argp, Menus[i]->Command);
            
            if ((Len == 0) || (Len > UTIL_StrLen(Argp)))                                  // found a menu matching parm //
              {
                  UTIL_MenuShow(Menus[i]);
                  return;
              }
        }
      printd("Unknown help parameter %s\n\r", Argp);

      return;
}
//----------------------------------------------------------------------------------------
// name      : Simtek_New
// purpose   : show the user the help menu
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void Simtek_New(void *CmdLine)
{
      CmdLine = CmdLine;
}

//----------------------------------------------------------------------------------------
// name      : UTIL_ReadI2C ()
// purpose   : 
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void UTIL_ReadI2C(void *cmdLine)
{
      byte *cmd    = NULL;
      ulong device = 0;
      ulong addr   = 0;
      ulong len    = 0;
      ulong i;
      char  memdata[256];
  
      cmd = UTIL_NextArg(cmdLine);
      if (*cmd == 0)
        {
        printd("Usage : read <device address> <memory address> <memory length>\n\r");
        return;
        }
      if (UTIL_StrToULong(cmd, &device) == FALSE)
        {
        printd("Device address error.\n\r");
        return;
        }
      CHECK_RANGE(device, 0, 0x07, "The device address is out of range.\n\r");
      cmd = UTIL_NextArg(cmd);
      if (*cmd == 0)
        {
        printd("No memory address specified\n\r");
        printd("Usage : read <device address> <memory address> <memory length>\n\r");
        return;
        }
      if (UTIL_StrToULong(cmd, &addr) == FALSE)
        {
        printd("Memory address error.\n\r");
        return;
        }
      CHECK_RANGE(addr, 0, 0x07FF, "The memory address is out of range.\n\r");
      cmd = UTIL_NextArg(cmd);
      if (*cmd == 0)
        {
        printd("No read length specified\n\r");
        printd("Usage : read <device address> <memory address> <memory length>\n\r");
        return;
        }
      if (UTIL_StrToULong(cmd, &len) == FALSE)
        {
        printd("Memory length error.\n\r");
        printd("length = %lu\n\r",len);
        return;
        }
      CHECK_RANGE(len, 1, 0x07FF, "The length is out of range.\n\r");
      if((len / 16) > 0)
        { 
        printd(" Address    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n\r");
        printd("         |--------------------------------------------------\n\r");
        }
      else
        {
        printd(" Address    ");
        for(i = 0; i < len;i++)
          printd("%02bx ",(byte)i);
        printd("\n\r");  
        printd("         |--");
        for(i = 0; i < len;i++)
          printd("---");
        printd("\n\r");  
        }
      if (EEPROM_Read((byte)(device|0xA0), (byte)addr, &memdata, (word)len) == I2C_OK)
        {
        for (i = 0 ; i < len ; i ++)
          {
          if (((i) % 16) == 0)                    printd(" 0x%04dx  |  ",(word)(addr+i)); // display the address in from of the begining of the line, and then every 16th
          printd("%02bx ", (byte)(memdata[i]));                                          // display the contents of the memory cell
          if (((i+1) % 16) == 0)                  printd("\n\r");                         // wrap around after 16th cell
          }
        printd("\n\r");
        }
      else
        {        printd("Read Error.\n");        }
} // End of UTIL_ReadI2C () //

//----------------------------------------------------------------------------------------
// name      : UTIL_WriteI2C ()
// purpose   : 
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void UTIL_WriteI2C(void *cmdLine)
{
      // NAMING CONSTANT DECLARATIONS //

      // LOCAL VARIABLE DECLARATIONS //
      byte *cmd = 0;
      ulong device = 0;
      ulong Addr = 0;
      ulong datatemp = 0;
      byte  memdata[64];
      word  len = 0;
      byte  i;

      memset(&memdata[0],0,64);
  
// get argument
      cmd = UTIL_NextArg(cmdLine);
// check for arguments
      if (*cmd == 0)
        { printd("Usage : pwr <device address> <memory address> <memory data>\n\r"); return;}
// convert ascii chars to numbers
      if (UTIL_StrToULong(cmd, &device) == FALSE)
        { printd("Device address error.\n\r"); return;}
      CHECK_RANGE(device, 0, 0x07, "The device address is out of range.\n\r");
// get next argument
      cmd = UTIL_NextArg(cmd);
// check for arguments
      if (*cmd == 0)
        {
        printd("No memory address specified\n\r");
        printd("Usage : pwr <device address> <memory address> <memory data>\n\r");
        return;
        }
      if (UTIL_StrToULong(cmd, &Addr) == FALSE)
        { printd("Memory address error.\n\r"); return;}
// convert ascii chars to numbers
      CHECK_RANGE(Addr, 0, 0x07FF, "The memory address is out of range.\n\r");

// get next argument
      cmd = UTIL_NextArg(cmd);
// check for arguments
      if (*cmd == 0)
        {
        printd("No memory data specified\n\r");
        printd("Usage : pwr <device address> <memory address> <memory data>\n\r");
        return;
        }
      
      for (len = 0 ; len < 64 ; )
        {
        if (UTIL_StrToULong(cmd, &datatemp) == FALSE)
          { printd("Memory data error.\n\r"); return; }
        CHECK_RANGE(datatemp, 0, 255, "The data is out of range.\n\r");

        memdata[len] = datatemp;
        len ++;
        cmd = UTIL_NextArg(cmd);
        if (*cmd == 0)
          { break; }
        }
      if(EEPROM_Write((0xA0 | (byte)device), (byte)Addr, &memdata, (word)len) == I2C_OK)
        {
        printd("Write Address : %lx\n\r", Addr);
        printd("Write Data : ");
            for (i = 0 ; i < len ; i ++)
            {
                  printd("%02bx ", (byte)memdata[i]);
            }
            printd("\n\r");
        }
      else
        {
            printd("Write Error");
        }
        
} // End of UTIL_WriteI2C () //

//----------------------------------------------------------------------------------------
// name      : UTIL_Fill ()
// purpose   : 
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void UTIL_Fill(void *cmdLine)
{
      char *cmd;
      int   len   = 0;
      byte  start = 0;
      word  end   = 0;
      byte  ch    = 0;

// clear the buffer
      cmd = UTIL_NextArg(cmdLine);                                                       // get the address of the first character in the name
      if ((char *)cmd[0] == '?')                                                                 // check for the help character
        {
        len = UTIL_StrLen(cmdLine);                                                       // get the position of the end or 1st ' '
        cmd = malloc(len+1);                                                             // allocate enough ram to cover length of caommand + a null
        strncpy(cmd, cmdLine, len);                                                      // copy the command part of the command line
        printd("Usage          :\n\r");                                                   // tell the user what the message is meant for
        printd("write to prom  : %s <start_addr> <end_addr> <data>",cmd);                // display the fill command usage
        }
      else
        {
        if (UTIL_StrToByte(cmd, &start) == FALSE)                                        // convert the ascii characters into numbers
          {  printd("bad argument 1\n\r");   return;   }                                  // tell the user there is an error in the data
        cmd = UTIL_NextArg(cmd);                                                        // get the address of the first character in the name
        if (UTIL_StrToWord(cmd, &end)   == FALSE)                                        // convert the ascii characters into numbers
          {  printd("bad argument 2\n\r");   return;   }                                  // tell the user there is an error in the data
        cmd = UTIL_NextArg(cmd);                                                        // get the address of the first character in the name

        if (UTIL_StrToByte(cmd, &ch)    == FALSE)                                        // convert the ascii characters into numbers
          {  printd("bad argument 3\n\r");   return;   }                                  // tell the user there is an error in the data
        printd("character : %02bx\n\r",ch);
        len = (end - (word)start) + 1;                                                    // calculate the requested length
        printd("start     : %02bx\n\r",start);
        printd("end       : %02dx\n\r",end);
        printd("length    : %du\n\r",len);
        if(len < 0)                                                                       // check calculated length for an error
          {  printd("range error\n\r");      return;   }                                  // tell the user the start and end argument are wrong
        cmd = malloc(len);                                                               // attempt to allocate a block of ram the size of the requested length
        if(cmd == NULL)                                                                  // check that the memory block allocated properly
          { printd("unable to allocate required memory size");  }                         //
        else
          { 
          memset(cmd,ch,len);                                                            // fill block with with given character
          printd("len       : %dd\n\r%s",len,cmd);
          if(EEPROM_Write(0xA0, start, cmd, len) != I2C_OK)                              // write the data to the eeprom
            {   printd("Write Error");   }                                                // show user there was an error in the eeprom write
          }
        }
      printd("\n\r");
      free(cmd);
}

#ifdef RuntimeCodeAt24KH
//----------------------------------------------------------------------------------------
// name      : UTIL_FwUpdate ()
// purpose   : 
// params    :  
// returns   : none
// note      : 
//----------------------------------------------------------------------------------------
static void UTIL_FwUpdate(void)
{
//byte IDATA fwAutoUpdated _at_ 0x31;
      fwAutoUpdated = Check_Value | (~Check_Value << 4);
//const byte                    Check_Value = 0x0a;
      printd("\n\r");
      DELAY_Ms(100);
      AX11000_SoftReboot();
}
#endif

//----------------------------------------------------------------------------------------
// name      : static int   UTIL_StrLen(char//Str1)
// purpose   : Return length of string passed. space char counts as end of string.
// params    : char *Str
// returns   : return length of string passed.
// note      :
//----------------------------------------------------------------------------------------
static int  UTIL_StrLen(char *Str)
{
      int   StrLen = 0;

      while (*Str && (*Str != ' '))                                                       // find first NULL or space
        {
        Str ++;
        StrLen ++;
        }
      return (StrLen);
}


// EXPORTED SUBPROGRAM BODIES //

//----------------------------------------------------------------------------------------
// name      : bool UTIL_MenuInit(void)
// purpose   : Initialize the menu system.
// params    : none
// returns   : TRUE, FALSE
// note      :
//----------------------------------------------------------------------------------------
bool UTIL_MenuInit(void)
{
      word      MenuItem = 0;

//      enterpw = FALSE;
//      pwcnt   = 0;
      for (MenuItem = 0; MenuItem < MAX_MENUS; MenuItem++)
        {
        Menus[MenuItem] = NULL;
        }
      MenuNum  = 0;
      NextCbuf = &CmdBuf[0];
      return (UTIL_MenuSetup(MainMenu));
}

//----------------------------------------------------------------------------------------
// name      : void UTIL_MenuExecute(void)
// purpose   : Called by the system whenever a console character is ready. Calls
//             kbhit() to test, so it can be used for polling. If an enter ASCII
//             (13 or 10) is received, it calls do_command() with the currently
//             buffered input characters. 
// params    : none
// returns   : void
// note      :
//----------------------------------------------------------------------------------------
void UTIL_MenuExecute(void)
{
      word      Key = 0;

      // get keystroke //
      Key = uart0_NoBlockGetkey();
//      if(pwcnt > 1) pwcnt--;
//      if(pwcnt == 1) { pwcnt = 0; Key = 0x0d; }
      if (!Key)
        {    return;    }
//      if( pwcnt != 0  ) pwcnt = pwTimeout;  
      
//----------------------------------------------------------------------------------------
//  check if the key press was a carraige return
//----------------------------------------------------------------------------------------
      if ((Key == 0x0D) || (Key == 0x0A))
        {
        if (NextCbuf == CmdBuf)                                                           // no data in cbuf //
          {    printd("\n\r%s", Prompt); return;    }
//          {    printd("\n\r%s", Prompt); enterpw = FALSE;  return;    }
        // else there's data in cbuf //
        *NextCbuf = 0;                                                                    // null terminate string //
        printd("\n\r");
        UTIL_DoCmd((void *) &CmdBuf[0]);                                                  // lookup and execute command //      
        NextCbuf = CmdBuf;                                                                // move pointer to next command in buffer
        printd("\r");
//        if(enterpw == TRUE) printd(pwPrompt);
//        if(pwcnt > 0) { printd(pwaPrompt); }
        printd(Prompt);
        }
//----------------------------------------------------------------------------------------
// if not a carriage return check for a printable character and store press for decoding
//----------------------------------------------------------------------------------------
      if ((Key  >= ' ') && (Key < 123))                                                   // got printable char //
        {
        if ((Key  >= 'A') && (Key <= 'Z')) Key |= 0x20;                                   // character an upper case make it lower case
        if (NextCbuf >= &CmdBuf[CBUFLEN-1])                                               // check to make sure the command string pointer is not outside the character buffer
          {    return;    }                                                               // buffer full //
        *NextCbuf = (char)Key;                                                            // place the key press in the buffer
        NextCbuf ++;                                                                      // update pointer to next unused cell in command buffer
        printd("%c",(char)Key);//putchar(Key);                                            // write key presss to debug port/terminal window for user
        }
//------------
// not a carriage return, or printable character. check for a <backspace>
//------------
      else if (Key == 0x007f)                                                             // backspace
        {
        if (NextCbuf > CmdBuf)                                                            // remove last entered character from buffer
          {
          *NextCbuf = '\0';                                                               // place the key press in the buffer
          NextCbuf --;
          *NextCbuf = '\0';                                                               // place the key press in the buffer
          printd("\b \b");                                                                // move cursor back 1, overwrite character with space, and then move back again
          }
        Key = '\0';
        }
//----------------------
// none of the above, check for an <esc> press
//----------------------
      else if (Key == 0x1B)                                                               // escape
        {
        while (NextCbuf > CmdBuf)                                                         // reset buffer //
          {
          NextCbuf --;
          printd("\b \b");                                                                // move cursor back 1, overwrite character with space, and then move back again
          }
        CmdBuf[0] = 0x1B;                                                                 // leave ESC in first byte //
        CmdBuf[1] = 0;                                                                    // clean up string //
        // next char will overwrite ESC //
        }
// USE TO FIND UNKNOWN KEY CODES
//      else                                                                                // 
//        {
//        printd("%04dx",Key);                                                              // write key presss to debug port/terminal window for user
//        }
      return;           // return to main()
}

//----------------------------------------------------------------------------------------
// name      : bool UTIL_MenuSetup(MENU_OPT *NewItem)
// purpose   : Add new menu item to master list. 
// params    : MENU_OPT *NewItem
// returns   : Returns TRUE if OK, FALSE if no more slots
// note      :
//----------------------------------------------------------------------------------------
bool UTIL_MenuSetup(CMD_MENU *NewItem)
{
      if (MenuNum < MAX_MENUS)
      {
            Menus[MenuNum] = NewItem;
            MenuNum ++;
            return (TRUE);
      }
      else
      {
            printd("No more room for menu adding\n");
            return (FALSE);
      }
}

//----------------------------------------------------------------------------------------
// name      : void UTIL_Stooges
// purpose   : called when an unused menu routine hook is exececuted. This means
//           a serious programming error occured. 
// params    : none
// returns   : Returns TRUE forever.
// note      :
//----------------------------------------------------------------------------------------
void UTIL_Stooges(void)
{
      printd("System error, function <UTIL_Stooges> should not be called\n");
      return;
}

//----------------------------------------------------------------------------------------
// name      : char *UTIL_NextArg(int   *Argp)
// purpose   : Returns a pointer to next arg in string passed. args 
//             are printable ascii sequences of chars delimited by spaces. If 
//             string ends without more args, then a pointer to the NULL 
//             terminating the string is returned. 
// params    : char *Argp
// returns   : A pointer to next arg in string.
// note      :
//----------------------------------------------------------------------------------------
char *UTIL_NextArg(char *Argp)
{
      char *ss;      

      ss = Argp;
      ss = strchr(Argp,' ');
      while (*ss == ' ')
        {    ss ++;    }                                                                  // scan past current arg //
      return (ss);
}

//----------------------------------------------------------------------------------------
// name      : int   UTIL_WaitKey(void)
// purpose   : Returns the key pressed by user, if user does not press key, this
//             will call super loop functions hooked by main function to do other
//             jobs. Using this way to overcome lacking of RTOS.
// params    : none
// returns   : Key pressed by user
// note      :
//----------------------------------------------------------------------------------------
int   UTIL_WaitKey(void)
{
      while (!_getkey()) { }
      return (getchar());
}

//----------------------------------------------------------------------------------------
// name      : UTIL_PortNumToStr
// purpose   : convert word into ascii string with header
// params    : 
//   input   : pointer to word array storing the converted bytes of ascii numbera
//   output  : pointer to ascii string 
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
//bool                          UTIL_PortNumToStr(char *hdr, word *num)
//{
//      word  i;
//      byte *pi;
//      
//      pi = (byte *)num;
//      
//      i = *pi;
//      i = i << 8;
//      i |= *(pi + 1);
//
//      printd("%-24s: (w)  %du\n\r",hdr , i);
//      return TRUE;
//}

//----------------------------------------------------------------------------------------
// name      : UTIL_ByteAryToStr
// purpose   : convert hex number into ascii string
// params    : 
//   input   : pointer to byte array storing the converted bytes of ascii numbera
//   output  : pointer to ascii string 
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
bool                          UTIL_ByteAryToStr(char *hdr, byte *num, byte len)
{
      byte a;
      
      printd("%-24s:",hdr);
      for(a = 1; a < (len + 1); a++)
        {
        printd(" %02bx", (byte)(*num++));
        if((a % 16) == 0) printd("\n\r%-24s:","");
        }
      printd("\n\r");
      return TRUE;
}

//----------------------------------------------------------------------------------------
// name      : UTIL_Checksum
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_Checksum(void)
{
      GCONFIG_CFG_DATA block;
      GCONFIG_CFG_DATA *dcb;

      dcb   = &block;
      if (EEPROM_Read(0xA0, runtimesettings, (byte *)dcb, sizeof(block)) == I2C_OK)       // read data back for eeprom
     
//      dcb->CheckSum = exChecksum((word *)dcb, sizeof(block));                             //
      if (EEPROM_Write(0xA0, rtChecksum, (byte *)dcb, sizeof(dcb->CheckSum)) == I2C_OK)   // write the new calculated checksum to the prom
        {
        printd("checksum updated\n\r");
        }
//       (IPv4PortLen, rtChecksum, "DCB Checksum", cmdLine);                              //
}

//----------------------------------------------------------------------------------------
// name      : DevServerPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_CfgLen(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtCfgLength, "DCB Length", cmdLine);               //
}

//----------------------------------------------------------------------------------------
// name      : UTIL_StrToIPv4Num
// purpose   : convert ascii string into hex number
// params    : 
//   input   : pointer to ascii string 
//   output  : pointer to byte array storing the converted bytes of ascii numbera
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
bool                          UTIL_StrToIPv4Num(void *cmdLine, byte *num)
{
      byte  *cmd;
      int   i;
      byte  x;
// remove all '.'
      cmd  = UTIL_NextArg(cmdLine);                                                       // get the mac address digits
      do
        {
        cmd = strchr(cmdLine,'.');                                                        // check for invalid character
        if(*cmd != 0)   *cmd = ' ';                                                       // if there is a dot replace it with a space
        }
      while(*cmd != 0);                                                                   //
      cmd  = UTIL_NextArg(cmdLine);                                                       // get the mac address digits
      i    = strlen(cmd);                                                                 //
      if(i == strspn(cmd, "0123456789 "))                                                 // check to make sure the string does not have bad chars
        {
        for(i = 0;i != 4;i++)                                                             // loop through all the octets of the IP
          {
          x = strtoul(cmd, &cmd, 10);                                                     // convert the octet from ascii to decimal
          *num = x;                                                                       // put the octet in the buffer
          num++;                                                                          // move pointer to next buffer location
          cmd = UTIL_NextArg(cmd);                                                        // get the mac address digits
          }
        }
      else { printd("\n\rInvalid character in IP address. The valid character set is '0123456789 .'\n\r"); return FALSE; }
      return TRUE;                                                                        //
}

//----------------------------------------------------------------------------------------
// name      : UTIL_StrToPortNum
// purpose   : convert ascii string into hex number
// params    : 
//   input   : pointer to ascii string 
//   output  : pointer to byte array storing the converted bytes of ascii numbera
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
bool                          UTIL_StrToPortNum(void *cmdLine, byte *num)
{
      byte  *cmd;
      int   i;
      word  x;
// remove all '.'
      cmd  = UTIL_NextArg(cmdLine);                                                       // get the mac address digits
      i    = strlen(cmd);                                                                 //

      if(i == strspn(cmd, "0123456789 "))                                                 // check to make sure the string does not have bad chars
        {
        x = strtoul(cmd, &cmd, 10);                                                       // convert the number from ascii to decimal
        *num = (byte)(x << 8);                                                            // put the octet in the buffer
        num++;                                                                            // move pointer to next buffer location
        *num = (byte)x;                                                                   // put the octet in the buffer
        }
      else 
        { 
        printd("\n\rInvalid character in port number\n\r");                               // let the user know an invalid characcter has been found
        printd("The valid character set is '0123456789 '\n\r");                           // show the user the allowed character set
        return FALSE;                                                                     // return a fail condition to the calling routine
        }
      return TRUE;                                                                        //
}

//----------------------------------------------------------------------------------------
// name      : bool UTIL_StrToByte(byte *ConStr, byte *ConRes)
// purpose   : Convert hexadecimal string to unsigned character.
// params    :
//   input   : ConStr - converted hexadecimal string (prefix with or without 0x, 
//             no leading spaces, null, '-', or space ended).
//   output  : ConRes - convert result, unsigned character.  
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
bool UTIL_StrToByte(byte *ConStr, byte *ConRes)
{
      byte TmpStr[BUF_LEN];
      byte BufIndex, Len, ShiftIdx;
      byte *ConStart;
      byte TmpRes;
   
      for (BufIndex = 0; BufIndex < BUF_LEN; BufIndex ++)
        {
        TmpStr[BufIndex] = ConStr[BufIndex];
        if ((TmpStr[BufIndex] == 0) || (TmpStr[BufIndex] == ' ') || (TmpStr[BufIndex] == '-') || (TmpStr[BufIndex] == '.'))
          {
          break;
          }
        }
      if (BufIndex == BUF_LEN)
        {
        return (FALSE);
        }

      TmpStr[BufIndex] = 0;
      Len = UTIL_StrLen(TmpStr);

      // take off prefix zero or '0x' //
      ConStart = TmpStr;
      for (BufIndex = 0; BufIndex < Len; BufIndex ++)
      {
            if (TmpStr[BufIndex] == '0')
            {
                  ConStart ++;
            }
            else
            {
                  break;
            }
      }
      if (*ConStart == 'x')
      {
            ConStart++;
      }

      // check length of converted string //
      Len = UTIL_StrLen(ConStart);
      if (Len > (sizeof(byte) * 2))
      {
            return (FALSE);
      }
      // converting //
      *ConRes = 0;
      for (ShiftIdx = 0; Len != 0; ShiftIdx ++, Len --)
      {
            TmpRes = *(ConStart + Len - 1);

            if ((TmpRes >= '0') && (TmpRes <= '9'))
                  TmpRes -= '0';
            else if ((TmpRes >= 'a') && (TmpRes <= 'f'))
                  TmpRes = (TmpRes - 'a') + 10;
            else if ((TmpRes >= 'A') && (TmpRes <= 'F'))
                  TmpRes = (TmpRes - 'A') + 10;
            else
                  return (FALSE);
            *ConRes |= (TmpRes << (ShiftIdx * 4));
      }
      return (TRUE);
}

//----------------------------------------------------------------------------------------
// name      : bool UTIL_StrToWord(byte *ConStr, word *ConRes)
// purpose   : Convert hexadecimal string to unsigned short integer.
// params    :
//   input   : ConStr - converted hexadecimal string (prefix with or without 0x, 
//             no leading spaces, null or space ended).
//   output  : ConRes - convert result, unsigned short integer.    
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
bool UTIL_StrToWord(byte *ConStr, word *ConRes)
{
      byte  TmpStr[BUF_LEN];
      byte  BufIndex, Len, ShiftIdx;
      byte *ConStart;
      word  TmpRes;
   
      printd("");  // not sure why this needs to be here yet, but will not work properly without it
      for (BufIndex = 0; BufIndex < BUF_LEN; BufIndex ++)
        {
            TmpStr[BufIndex] = ConStr[BufIndex];
            if ((TmpStr[BufIndex] == 0) || (TmpStr[BufIndex] == ' '))
            {
                  break;
            }
        }
// printd("stage 2\n\r");
      if (BufIndex == BUF_LEN)
        {            return (FALSE);      }
// printd("stage 3\n\r");
      TmpStr[BufIndex] = 0;
      Len = UTIL_StrLen(TmpStr);

      // take off prefix zero or '0x' //
      ConStart = TmpStr;
      for (BufIndex = 0; BufIndex < Len; BufIndex ++)
        {
        if (TmpStr[BufIndex] == '0')
          {    ConStart ++;            }
        else
          {    break;                  }
        }
      if (*ConStart == 'x')
        {  ConStart ++;      }

      // check length of converted string //
      Len = UTIL_StrLen(ConStart);
      if (Len > (sizeof(word) * 2))
      {
            return (FALSE);
      }
// printd("stage 4\n\r");

      // converting //
      *ConRes = 0;
      for (ShiftIdx = 0; Len != 0; ShiftIdx ++, Len --)
      {
            TmpRes = *(ConStart + Len - 1);

            if ((TmpRes >= '0') && (TmpRes <= '9'))
                  TmpRes -= '0';
            else if ((TmpRes >= 'a') && (TmpRes <= 'f'))
                  TmpRes = (TmpRes - 'a') + 10;
            else if ((TmpRes >= 'A') && (TmpRes <= 'F'))
                  TmpRes = (TmpRes - 'A') + 10;
            else
                  return (FALSE);
            *ConRes |= (TmpRes << (ShiftIdx * 4));
      }
// printd("stage 5\n\r");
      return (TRUE);
}

//----------------------------------------------------------------------------------------
// name      : bool UTIL_StrToULong(byte *ConStr, ulong *ConRes)
// purpose   : Convert hexadecimal string to unsigned long integer.
// params    :
//   input   : ConStr - converted hexadecimal string (prefix with or without 0x, 
//             no leading spaces, null or space ended).
//   output  : ConRes - convert result, unsigned long integer.     
// returns   : TRUE: successful ; FALSE: failed
// note      :
//----------------------------------------------------------------------------------------
bool         UTIL_StrToULong(byte *ConStr, ulong *ConRes)
{
      byte  TmpStr[BUF_LEN];
      byte  BufIndex, Len, ShiftIdx;
      byte  *ConStart;
      ulong TmpRes;
      
      for (BufIndex = 0; BufIndex < BUF_LEN; BufIndex ++)  // BUF_LEN = 32
        {
        TmpStr[BufIndex] = ConStr[BufIndex];                                  // copy the next character into the temp string
        if ((TmpStr[BufIndex] == 0) || (TmpStr[BufIndex] == ' '))             // check if the character is a blank, or a NULL
          {     break;          }                                             // the character is a blank, or a NULL, stop copying
        }
      if (BufIndex == BUF_LEN)
        {       return (FALSE); }
      TmpStr[BufIndex] = 0;                                                   // set the last character to a NULL

      Len = UTIL_StrLen(TmpStr);                                              // calculate the length of the string
      // take off prefix zero or '0x' //
      ConStart = TmpStr;                                                      // assign the memory address of the first character to a variable
      for (BufIndex = 0; BufIndex < Len; BufIndex ++)                         // loop to remove all leading zeros
        {
        if (TmpStr[BufIndex] == '0')                                          // check if the character is a leading zero
          {     ConStart ++;    }                                             // the character is a leading zero, move to the next character
        else
          {     break;          }                                             // the character was no a leading zero, stop here
        }
      if (*ConStart == 'x')                                                   // check if the character is a hex signifier
        {
        ConStart++;                                                           // it was a hex identifier, move to the next character in the string
        }

      // check length of converted string //
      Len = UTIL_StrLen(ConStart);                                            // calculate the length of the string
      if (Len > (sizeof(ulong) * 2))                                          // check that the number of characters does not exceed the number of digit in an unsigned long
        {       return (FALSE); }                                             // the length is greater, return an error

      // converting //
      *ConRes = 0;                                                            // set the return value to zero
      for (ShiftIdx = 0; Len != 0; ShiftIdx ++, Len --)                       // convert each character in the string from ASCII to a number, starting with the right most character
        {
        TmpRes = *(ConStart + Len - 1);                                       // get the character
        if ((TmpRes >= '0') && (TmpRes <= '9'))                               // check if the character is a decimal digit
          TmpRes -= '0';                                                      // it was, convert for ASCII to decimal
        else if ((TmpRes >= 'a') && (TmpRes <= 'f'))                          // check if the character is a hexadecimal digit in lower case
          TmpRes = (TmpRes - 'a') + 10;                                       // it was, convert from ASCII to decimal
        else if ((TmpRes >= 'A') && (TmpRes <= 'F'))                          // check if the character is a hexadecimal digit in upper case
          TmpRes = (TmpRes - 'A') + 10;                                       // it was, convert from ASCII to decimal
        else                                                                  // the character was not a hexadecimal digit
          return (FALSE);                                                     // the conversion failed, report to calling routine
        *ConRes |= (TmpRes << (ShiftIdx * 4));                                // add the number to the value, in the current digit location
        }
      return (TRUE);                                                          // the conversion completed successfully, report to calling routine
}

//----------------------------------------------------------------------------------------
// name      : UTIL_StrCmp(char//Str1, char *Str2)
// purpose   : Special string compare routine. returns 0 if strings match, 
//             else returns char where 1st character mismatch occured. ie:
//             stcmp("help", "helper") returns a 5. stcmp("help", "nohelp") 
//             returns a 1. Also: treats space as end of string.
// params    : char *Str1, char *Str2
// returns   : 0 if strings match, else returns char where 1st mismatch occured.
// note      :
//----------------------------------------------------------------------------------------
int          UTIL_StrCmp(char *Str1, char *Str2)
{
      int   Posi = 1;                                                                     // byte being checked //

      while (*Str1 && *Str1 != ' ')                                                       // keep going until the character is a null, or a space
        {
        if ((*Str1 ++) != (*Str2 ++))                                                     // check chars for match, increment both char pointers if they match
          {     return (Posi);      }                                                     // character mismatch found, return failed condiition/position
        Posi ++;                                                                          // move position indicator to next character
        }
      if (*Str2)                                                                          // check to make sure there are no more characters in string 2
        {       return(Posi);       }                                                     // s2 was longer than s1 return fail condition
      return(0);                                                                          // return pass condition
}



//----------------------------------------------------------------------------------------
// name      : MACAddress
// purpose   : overwrite the mac address in the eeprom
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static void  MACAddress(void *cmdLine)
{
      char        memdata[20];
      byte       *cmd;
//      char       *help;
      byte        tmp;   
      short       i = 0;

      memset(&memdata[0],0,20);                                                           // clear the buffer
// get argument
      cmd = UTIL_NextArg(cmdLine);                                                        // get the mac address digits
//      help = (char *)cmd;                                                                 // 
      i = strlen(cmd);                                                                    // get the length of the user entered string
      if(i == 0)                                                                          //
        {
        if (EEPROM_Read(0xA0, rtMAC, &memdata[0], len_macx) == I2C_OK)                    //
          {
          printd("mac : ");                                                               //
          for (i = 0 ; i != len_macx ; i++)                                           // loop to until all bytes are output
            {   printd("%02bx ", (byte)memdata[i]);   }                               // display all bytes in eeprom order
          }
        else
          {   printd("Read Error.");   }                                                  // show user there was an error in the eeprom write
        }
      else if ((char *)cmd[0] == '?')
        {
        printd("Usage          :\n\r");
        printd("read from prom : mac\n\r");
        printd("write to prom  : mac <octet1> <octet2> <octet3> <octet4> <octet5> <octet6>");
        }
      else
        {
        for (i = 0 ; i < 20 ; i++)                                                        // scroll through all arguments
          {
          if (*cmd == 0)          { break; }                                              // if a null is reached break out of the loop
          if (UTIL_StrToByte(cmd, &tmp) == FALSE)                                         // convert the ascii characters into numbers
            {   printd("Memory data error.\n\r"); return;   }                             // tell the user there is an error in the data
          memdata[i] = tmp;                                                               // place the mac address into the the correct order
          cmd = UTIL_NextArg(cmd);                                                        // get the mac address digits
          }
        if(i >= 5)
          {
          if(EEPROM_Write(0xA0, rtMAC, &memdata, len_macx) == I2C_OK)                     // write the data to the eeprom
            {
            memset(&memdata[0],1,20);                                                     // clear the buffer
            if (EEPROM_Read(0xA0, rtMAC, &memdata, len_macx) == I2C_OK)                   //
              {
              printd("mac : ");                                                           //
              for (i = 0 ; i != len_macx ; i++)                                           // loop to until all bytes are output
                {   printd("%02bx ", (byte)memdata[i]);   }                               // display all bytes in eeprom order
              }
            else
              {   printd("Read-Back Error");   }                                          // show user there was an error in the eeprom write          
            }
          else
            {   printd("Write Error");   }                                                // show user there was an error in the eeprom write          
          }
        else
          {
          printd("Incomplete MAC address?\n\r");                                          // show user there was an error in the eeprom write
          printd("Usage : mac <octet1> <octet2> <octet3> <octet4> <octet5> <octet6>");    // show user there was an error in the eeprom write
          }                       
        }
        printd("\n\r");
}

//----------------------------------------------------------------------------------------
// name      : binaryflagcontrol
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static void  binaryflagcontrol(byte celladdr, word mask, char *title, char *defalt, char *nondefault, byte len, void *cmdLine)
{
      char  memdata[5];
      byte *cmd;
      word  i = 0;
      char *command;
      word *dat;

      dat = (word *)&memdata[0];
      i       = UTIL_StrLen(cmdLine);                                                     // get the position of the end or 1st ' '
      command = malloc(i+1);                                                              // allocate enough ram to cover length of command + a null
      strncpy(command, cmdLine, i);                                                       // copy the command part of the command line

      *dat = 0;                                                                           // clear the buffer
      cmd  = UTIL_NextArg(cmdLine);                                                       // get the address of the first character in the name
      i    = strlen(cmd);                                                                 // get the length of the user entered string
      if(i == 0)
        {
        if (EEPROM_Read(0xA0, celladdr, &memdata[0], len) == I2C_OK)                      // read back the data just written to the eeprom
          {
          if((*dat & mask) == 0)                                                          // check the transmit move
            printd("%s : %s (%04dx)",title,defalt,*dat);                                              // the default mode is active, show the user
          else
            printd("%s : %s (%04dx)",title,nondefault,*dat);                                          // the default mode is inactive, show the user
          }
        else
          {   printd("Read Error.");   }                                                  // show user there was an error in the eeprom write
        }
      else if ((char *)cmd[0] == '?')                                                            // check for a help command
        {
        printd("Usage          :\n\r");                                                   // tell the user what the message is meant for
        printd("read from prom : %s\n\r",command,defalt,nondefault);                      // display the read command usage
        printd("write to prom  : %s <%s/%s>",command,defalt,nondefault);                  // display the write command usage
        free(command);
        }
      else
        {
        if (EEPROM_Read(0xA0, celladdr, &memdata[0], len) != I2C_OK)                      // read the current state from the eeprom
          {   printd("Read Error");   }                                                   // show user there was an error in the eeprom write
        if(strstr(cmd, nondefault) == '\0')                                               // check for the default mode ocmmand
          *dat &= (~mask);                                                                // the default mode was requested clear the flag
        else
          *dat |= mask;                                                                   // the default mode was not requested set the flag

        if((EEPROM_Write(0xA0, celladdr, &memdata[0], len)) == I2C_OK)                    // write the data to the eeprom
          {
          if (EEPROM_Read(0xA0, celladdr, &memdata[0], len) != I2C_OK)                    // read back the data just written to the eeprom
            {   printd("Read Error.");   }                                                // show user there was an error in the eeprom write
          if((*dat & mask) == 0)                                                          // check the transmit move
            printd("%s : %s (%04dx)",title,defalt,*dat);                                               // the default mode is active, show the user
          else
            printd("%s : %s (%04dx)",title,nondefault,*dat);                                           // the default mode is inactive, show the user
          }
        else
          {   printd("Write Error");   }                                                  // show user there was an error in the eeprom write
        }
       printd("\n\r");
}

//----------------------------------------------------------------------------------------
// name      : DHCPClientCtrl
// purpose   : change state of dhcp client flag between disabled and enabled
// params    : eprom cell address, bit position mask, help title, default mode,
//             non-default mode, command line
// returns   : -
// notes     :
//----------------------------------------------------------------------------------------
static      void              DHCPClientCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, GCONFIG_NETWORK_DHCP_ENABLE, "DHCP Client", "disable", "enable", sizeof(GCONFIG_NETWORK_DHCP_ENABLE), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : UDPProtoCtrl
// purpose   : 
// params    : 
//             
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UDPProtoCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, GCONFIG_NETWORK_PROTO_UDP, "UDP Protocol", "off", "on", sizeof(GCONFIG_NETWORK_PROTO_UDP), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : TCPProtoCtrl
// purpose   : 
// params    : 
//             
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              TCPProtoCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, GCONFIG_NETWORK_PROTO_TCP, "TCP Protocol", "off", "on", sizeof(GCONFIG_NETWORK_PROTO_TCP), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : BroadcastCtrl
// purpose   : 
// params    : 
//             
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              BroadcastCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, GCONFIG_NETWORK_PROTO_UDP_BCAST, "UDP Brodacasts Recieved", "off", "on", sizeof(GCONFIG_NETWORK_PROTO_UDP_BCAST), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : TrxModeCtrl
// purpose   : change state of transmit style flag between server and client
// params    : eprom cell address, bit position mask, help title, default mode,
//             non-default mode, command line
// returns   : -
// notes     :
//----------------------------------------------------------------------------------------
static      void              TrxModeCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, GCONFIG_NETWORK_CLIENT, "Transmit Mode", "server", "client", sizeof(GCONFIG_NETWORK_CLIENT), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : ReportMethodCtrl
// purpose   : change state of transmit style flag between polled and realtime
// params    : eprom cell address, bit position mask, help title, default mode,
//             non-default mode, command line
// returns   : -
// notes     :
//----------------------------------------------------------------------------------------
static      void              ReportMethodCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, ReportMethodMask, "Report Method", "polled", "realtime", sizeof(ReportMethodMask), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : DebugModeCtrl
// purpose   : change state of debug mode flag between off and on
// params    : eprom cell address, bit position mask, help title, default mode,
//             non-default mode, command line
// returns   : -
// notes     :
//----------------------------------------------------------------------------------------
static      void              DebugModeCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, DebugModeMask, "Debug Mode", "off", "on", sizeof(DebugModeMask), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : DeadmanCntCtrl
// purpose   : change state of deadman counter flag between off and on
// params    : eprom cell address, bit position mask, help title, default mode,
//             non-default mode, command line
// returns   : -
// notes     :
//----------------------------------------------------------------------------------------
static      void              DeadmanCntCtrl(void *cmdLine)
{
      binaryflagcontrol(rtNetwork, DeadManCntMask, "Deadman Counter", "off", "on", sizeof(DeadManCntMask), cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : MessageFrames
// purpose   : 
// params    : 
//             
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              MessageFrames(void)
{
//      char  memdata[len_name];
//      byte *cmd;
//
//      memset(&memdata[0],0,len_name);                                                      // clear the buffer
//      printd("%-24s: ","device name");
//      if (EEPROM_Read(0xA0, rtDevName, &memdata[0], len_name) == I2C_OK)
//        {
//        cmd = &memdata[0];
//        printd("%s",cmd);
//        }
//      else
//        {   printd("Read Error.");   }                                                  // show user there was an error in the eeprom write
//      printd("\n\r");

//      UTIL_ByteAryToStr("ResetReq",     (byte *)&bufResetReq,     bufResetMsgLength);        //
//      UTIL_ByteAryToStr("StatusReq",    (byte *)&bufStatusRsp,    m2sStatusMsgLength);       //
      UTIL_ByteAryToStr("StatusRsp",    (byte *)&bufStatusReq,    bufStatusMsgLength);       //
//      UTIL_ByteAryToStr("IndicatorReq", (byte *)&bufIndicatorMsg, bufIndicatorMsgLength);    //
      UTIL_ByteAryToStr("DimmingReq",   (byte *)&bufDimmingMsg,   bufDimmingMsgLength);      //
      UTIL_ByteAryToStr("DisplayReg",   (byte *)&bufDisplayMsg,   bufDisplayMsgLength);      //
//      UTIL_ByteAryToStr("FirmwareReq",  (byte *)&bufFirmwareReq,  m2sFirmwareMsgLength);     //
      UTIL_ByteAryToStr("FirmwareRsp",  (byte *)&bufFirmwareRsp,  bufFirmwareMsgLength);     //
}

//----------------------------------------------------------------------------------------
// name      : Com1RxPtr
// purpose   : display the UART1 Pointers
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              Com1RxPtr(void)
{
      UART1_DisplayRxPointers(0);
}

//----------------------------------------------------------------------------------------
// name      : Com1RxBuf
// purpose   : display the UART1 Pointers
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              Com1RxBuf(void)
{
      UART_ShowBuffer(&uart1_RxBuf[0], MAX_RX_UART1_BUF_SIZE);
}

//----------------------------------------------------------------------------------------
// name      : UTIL_EraseEEPROM
// purpose   : erase flash memory in eeprom and microcontroller
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_EraseEEPROM(void)
{
      char *cmd;

      cmd = malloc(0x100);                                                                // attempt to allocate a block of ram the size of the requested length
      if(cmd == NULL)                                                                     // check that the memory block allocated properly
        { printd("unable to allocate required memory size");  }                           //
      else
        { 
        memset(cmd,0xff,0x100);                                                           // fill block with with given character
        if(EEPROM_Write(0xA0, 0, cmd, 0x100) != I2C_OK)                                   // write the data to the eeprom
          {   printd("Write Error");   }                                                  // show user there was an error in the eeprom write
        }
}

//----------------------------------------------------------------------------------------
// name      : UTIL_EraseFlash
// purpose   : erase flash memory in eeprom and microcontroller
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
//static      void              UTIL_EraseFlash(void *cmdLine)
//{
//      char *cmd;
//
//      cmd = UTIL_NextArg(cmdLine);                                                        // remove the earase command and get the first argument
//      if (strstr(cmd,"eeprom") != NULL)                                                   // check for eeprom only
//        {
//        UTIL_EraseEEPROM();                                                               // erase the contents of the configuration eeprom
//        printd("EEPROM \n\r");
//        }
//      else if (strstr(cmd,"config") != NULL)                                              // check for config only
//        {
//        IntFlashErase(ERA_CFG);                                                           // erase configuration block
//        printd("configuration data\n\r");
//        }
//      else if (strstr(cmd,"runtime") != NULL)                                             // check for runtime only
//        {
//        IntFlashErase(ERA_RUN);                                                           // erase runtime block
//        printd("RUNTIME \n\r");
//        }
//      else if (strstr(cmd,"all") != NULL)                                                 // check for all
//        {
////        UTIL_EraseEEPROM();                                                               // erase the contents of the configuration eeprom
//        IntFlashErase(ERA_RUN);                                                           // erase runtime block
////        IntFlashErase(ERA_CFG);                                                           // erase configuration block
////        IntFlashErase(ERA_BLD);                                                           // 
//        if(IntFlashErase(ERA_ALL))                                                        // erase runtime block
//          printd("ALL \n\r");
//        else
//          printd("!all fail!\n\r");
//        }
//      else if (strstr(cmd,"?") != NULL)                                                   // check for help
//        {
//        printd("Usage :\n\r");                                                            // tell the user what the message is meant for
//        printd("erase : %s <%s/%s/%s/%s>\n\r","erase","eeprom","config","runtime","all"); // display the command usage <variables/...>
//        }
//      else
//        {
//        printd("invalid variable : %s\n\r",cmd);
//        }
//}

//----------------------------------------------------------------------------------------
// name      : UTIL_Rst_EEPROM
// purpose   : reset the runtime settings in the EEPROM to defaults
// params    : -
// returns   : -
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_Rst_EEPROM(void)
{
      byte len;      //0x00  0x01  0x02  0x03  0x04  0x05  0x06  0x07  0x08  0x09  0x0A  0x0B  0x0C  0x0D  0x0E  0x0F  
      byte memdat[] = {0x21, 0xbc, 0x00, 0x00, 0x30, 0x00, 0x00, 0xb0, 0x68, 0xc2, 0x50, 0x00, 0xf2, 0x05, 0x10, 0xe0,  //0x00
                       0x1d, 0x19, 0x87, 0x00, 0xff, 0xff, 0xff, 0xff, 0x10, 0x07, 0x0c, 0xa8, 0xc0, 0x00, 0xff, 0xff,  //0x10
                       0xff, 0x04, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,  //0x20
                       0x03, 0xf4, 0x00, 0x4a, 0x31, 0x30, 0x2d, 0x38, 0x35, 0x36, 0x36, 0x2d, 0x30, 0x31, 0x20, 0x23,  //0x30
                       0x31, 0x30, 0x31, 0x00, 0x00, 0x50, 0xc2, 0x68, 0xb1, 0x29, 0xc0, 0xa8, 0x0c, 0x01, 0xc0, 0xa8,  //0x40
                       0x0c, 0x02, 0xc0, 0xa8, 0x0c, 0x04, 0xc0, 0xa8, 0x0c, 0x03, 0xff, 0xff, 0xff, 0x00, 0x00, 0x00,  //0x50
                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc7, 0x4c, 0x62, 0x24, 0x62, 0x22, 0xc7, 0x4e, 0xc7, 0x4f,  //0x60
                       0xc7, 0x4d, 0x00, 0x00, 0x00, 0x49, 0x00, 0x00, 0x00};                                           //0x70

      len = sizeof(memdat);
      if(EEPROM_Write(0xA0, 0, &memdat[0], len) != I2C_OK)                                // write the data to the eeprom
        {   printd("Write Error");   }                                                    // show user there was an error in the eeprom write
      UTIL_ReadI2C("rd 0 0 100\0");

}

//----------------------------------------------------------------------------------------
// name      : DeviceName
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DeviceName(void *cmdLine)
{
      char  memdata[len_name];
      byte *cmd;
      word  i = 0;

      memset(&memdata[0],0,len_name);                                                      // clear the buffer
      cmd = UTIL_NextArg(cmdLine);                                                        // get the address of the first character in the name
      i = strlen(cmd);                                                                    // get the length of the user entered string
      if(i == 0)
        {
        if (EEPROM_Read(0xA0, rtDevName, &memdata[0], len_name) == I2C_OK)
          {
          cmd = &memdata[0];
          printd("name : %s",cmd);
          }
        else
          {   printd("Read Error.");   }                                                  // show user there was an error in the eeprom write
        printd("\n\r");
        }
      else if ((char *)cmd[0] == '?')
        {
        printd("Usage          :\n\r");
        printd("read from prom : name\n\r");
        printd("write to prom  : name <char1> ... <char15>\n\r");
        }
      else
        {
        if(i > 15)i = 15;                                                                 // truncate the string if longer then the name space
        memcpy(&memdata[0],cmd, i);                                                       // copy yhe name string into the buffer
        if(EEPROM_Write(0xA0, rtDevName, &memdata, (len_name)) == I2C_OK)                 // write the data to the eeprom
          {
          cmd = &memdata[0];
          printd("name : %s",cmd);
          }
        else
          {   printd("Write Error");   }                                                  // show user there was an error in the eeprom write
        printd("\n\r");
        }
}

//----------------------------------------------------------------------------------------
// name      : IPv4Address
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              IPv4Address(byte epromaddr,char *Title,void *cmdLine)
{
      char *cmd;
      int   len   = 0;
      long  x;
      byte  *pbx;

      x    = 0;
      pbx  = (byte *)&x;
      cmd  = UTIL_NextArg(cmdLine);                                                       // get the address of the first character in the name
      if((strlen(cmd)) == 0)                                                              // check for no arguments
        {
        if (EEPROM_Read(0xA0, epromaddr, pbx, IPv4AddrLen) == I2C_OK)                     // retrieve the setting from the eeprom
          {
          printd("%s : %ip",Title,x);                                                     // let the user know what setting is being displayed
          }
        else
          {   printd("Read Error.");   }                                                  // show user there was an error in the eeprom write
        }
      else if ((char *)cmd[0] == '?')                                                            // check for the help character
        {
        len = UTIL_StrLen(cmdLine);                                                       // get the position of the end or 1st ' '
        cmd = malloc(len+1);                                                              // allocate enough ram to cover length of caommand + a null
        strncpy(cmd, cmdLine, len);                                                       // copy the command part of the command line
        printd("Usage          :\n\r");                                                   // tell the user what the message is meant for
        printd("read from prom : %s\n\r",cmd);                                            // display the read command usage
        printd("write to prom  : %s <octet1>.<octet2>.<octet3>.<octet4>",cmd);            // display the write command usage
        }
      else
        {
        if(UTIL_StrToIPv4Num(cmdLine, pbx) == TRUE)                                       // convert the IP string into numbers
          {
          if(EEPROM_Write(0xA0, epromaddr, pbx, IPv4AddrLen) == I2C_OK)                   // write the data to the eeprom
            {
            memset(pbx,0,IPv4AddrLen);                                                  // clear the buffer
            if (EEPROM_Read(0xA0, epromaddr, pbx, IPv4AddrLen) == I2C_OK)                 // read data back for eeprom
              {
              printd("%s : %ip",Title,x);                                                 // let the user know what setting is being displayed
              }
            }
          else
            {   printd("Write Error");   }                                                // show user there was an error in the eeprom write
          }
        else
          {
          printd("Incomplete IPv4 address?\n\r");                                         // show user there was an error in the eeprom write
          printd("Usage : host_ip <octet1>.<octet2>.<octet3>.<octet4>");                  // show user there was an error in the eeprom write
          }
        }
        printd("\n\r");
}


//----------------------------------------------------------------------------------------
// name      : DeviceSIP
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DeviceSIP(void *cmdLine)
{
      IPv4Address(rtDevServerIpAddr, "Device Server IP", cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : DeviceCIP
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DeviceCIP(void *cmdLine)
{
      IPv4Address(rtDevClientIPAddr, "Device Client IP", cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : HostCIP
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              HostCIP(void *cmdLine)
{
      IPv4Address(rtHostClientIPAddr, "Host Client IP", cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : HostSIP
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              HostSIP(void *cmdLine)
{
      IPv4Address(rtHostServerIPAddr, "Host Server IP", cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : SubnetMask
// purpose   : overwrite the IP mask in the eeprom
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              SubnetMask(void *cmdLine)
{
      IPv4Address(rtSubNetmask, "Subnet Mask", cmdLine);
}

//----------------------------------------------------------------------------------------
// name      : PortNumber
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              PortNumber(byte portlen,byte portaddr,char *title, void *cmdLine)
{
      byte *cmd;
      char *command;
      byte *x;
      byte  i = 0;
      word  n = 0;

      i       = UTIL_StrLen(cmdLine);                                                     // get the position of the end or 1st ' '
      command = malloc(i+1);                                                              // allocate enough ram to cover length of caommand + a null
      strncpy(command, cmdLine, i);                                                       // copy the command part of the command line

      x    = malloc(portlen);                                                             //
      memset(x,0,portlen);                                                                // clear the buffer
      cmd  = UTIL_NextArg(cmdLine);                                                       // get the address of the first character in the name

      if((char *)cmd == NULL)                                                             // check for no arguments
        {
        if (EEPROM_Read(0xA0, portaddr, x, portlen) == I2C_OK)                            // retrieve the setting from the eeprom
          {
          printd("%s : ", title);                                                         // let the user know what setting is being displayed
          n = 0;                                                                          // initailize variable
          for(i = 0; i != portlen; i++)                                                   // loop until all byte are added to number
            {
            n  = n << 8;                                                                  // shift the lowest byte into the next highest byte
            n |= *(x+i);                                                                  // add the byte to the value
            }
          printd("%du",n);                                                                // display the state of the setting for the user
          }
        else
          {   printd("Read Error.");   }                                                  // show user there was an error in the eeprom write
        }
      else if ((char *)cmd[0] == '?')                                                            // check for the help character
        {
        printd("Usage          :\n\r");                                                   // tell the user what the message is meant for
        printd("read from prom : %s\n\r",command);                                        // display the read command usage
        printd("write to prom  : %s <word>",command);                                     // display the write command usage
        free(command);
        }
      else
        {
        if((UTIL_StrToPortNum(cmdLine, x)) == TRUE)                                       // convert the string into a number
          {
          if(EEPROM_Write(0xA0, portaddr, x, portlen) == I2C_OK)                          // write the data to the eeprom
            {
            memset(x,0,portlen);                                                          // clear the buffer
            if (EEPROM_Read(0xA0, portaddr, x, portlen) == I2C_OK)                        // read data back for eeprom
              {
              printd("%s : ", title);                                                     // let the user know what setting is being displayed
              n = 0;                                                                      // initailize variable
              for(i = 0; i != portlen; i++)                                               // loop until all byte are added to number
                {
                n  = n << 8;                                                              // shift the lowest byte into the next highest byte
                n |= *(x+i);                                                              // add the byte to the value
                }
              printd("%du",n);                                                            // display the state of the setting for the user
              }

            }
          else
            {   printd("Write Error");   }                                                // show user there was an error in the eeprom write
          }
        else
          {
          printd("Invalid character in port number?\n\r");                                // show user there was an error in the eeprom write
          printd("Usage : %s <word>", command);                                           // show user there was an error in the eeprom write
          }
        }
      printd("\n\r");
      free(x);
}

//----------------------------------------------------------------------------------------
// name      : DevServerPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DevServerPort(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtDevServerPort, "Device Server Port", cmdLine);                           //
}

//----------------------------------------------------------------------------------------
// name      : DevClientPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DevClientPort(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtDevClientPort, "Device Client Port", cmdLine);                         //
}

//----------------------------------------------------------------------------------------
// name      : DevUnicastPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DevUnicastPort(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtDevUnicastPort, "Device Unicast Port", cmdLine);                        //
}



//----------------------------------------------------------------------------------------
// name      : UTIL_pwExit
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_pwExit(void)
{
//      enterpw = FALSE;
//      pwcnt   = 0;
}

//----------------------------------------------------------------------------------------
// name      : DevBroadcastPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              DevBroadcastPort(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtDevBroadcastPort, "Device Broadcast Port", cmdLine);                      //
}

//----------------------------------------------------------------------------------------
// name      : HostServerPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              HostServerPort(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtHostServerPort, "Host Server Port", cmdLine);                        //
}

//----------------------------------------------------------------------------------------
// name      : HostClientPort
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              HostClientPort(void *cmdLine)
{
      PortNumber(IPv4PortLen, rtHostClientPort, "Host Client Port", cmdLine);                        //
}

//----------------------------------------------------------------------------------------
// name      : UTIL_Set
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_Set(void)
{
      byte  x[len_dcbx];
      byte *px;
      word  i;
      
      px = &x[0];

      if(EEPROM_Read(0xA0, runtimesettings, px, len_dcbx) == I2C_OK)                      // write the data to the eeprom
        {

        for (i = 0 ; i < len_dcbx ; i ++)
          {
          printd("%02bx ", *(px+i));                                                      // display the contents of the memory cell
          if (((i+1) % 16) == 0)                  printd("\n\r");                         // wrap around after 16th cell
          }

        if (EEPROM_Write(0xA0, defaultsettings, px, len_dcbx) == I2C_OK)                  // read data back for eeprom
          {
          printd("defaults updated sucessfully");                                         // let the user know what setting is being displayed
          }
        else
          {   printd("Default DCB Write Error");   }                                      // show user there was an error in the eeprom write
        }
      else
        {   printd("DCB Read Error");   }                                                 // show user there was an error in the eeprom write
      printd("\n\r");                                                                      // let the user know what setting is being displayed
      while(UART0_TxChk());                                                               // make sure all messages have finished transmitting
//      AX11000_SoftReboot();                                                               // reconfigure device
}

//----------------------------------------------------------------------------------------
// name      : UTIL_Reset
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_Reset(void)
{
      printd("Reset the device to the default settings\n\r");
      while(UART0_TxChk());
      AX11000_SoftReboot();               //SoftReset
}

//----------------------------------------------------------------------------------------
// name      : UTIL_Reboot
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_Reboot(void)
{
      printd("Reboot the device without saving changes\n\r");
      while(UART0_TxChk());
      AX11000_SoftReboot();
}

//----------------------------------------------------------------------------------------
// name      : Show_DCB
// purpose   : show the device configuration block
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              Show_DCB(void)                                              // show the device configuration block
{
      GCONFIG_CFG_DATA block;                                                             //
      GCONFIG_CFG_DATA *dcb;                                                              // 
      char devname[17];                                                                   // 
      byte  *pByte;                                                                       // 
//      ulong tt = 0;

//    gconfig_ExDisplayConfigData();                                                        // 
//  printd("**************************************************************\n\r");           //
      dcb = &block;
      pByte = (byte *)dcb;
      memcpy(&block, pByte, sizeof(GCONFIG_CFG_DATA));
      if (EEPROM_Read(0xA0, runtimesettings, pByte, sizeof(block)) == I2C_OK)             // read data back for eeprom
      printd("%-24s: %dx\n\r","CheckSum", dcb->CheckSum);                                 // 
      printd("%-24s: %du\n\r","CfgLength",dcb->CfgLength);                                // 
      memcpy(&devname[0], (byte *)&dcb->DevName.DevName, 16);                             // 
      devname[17] = '\0';                                                                 // 
      printd("%-24s: %s\n\r","Device Name", &devname);                                    // 
//      UTIL_ByteAryToStr("MAC", (byte *)&dcb->mac.MacAddr, 6);                             // 
//
//      printd("%-24s: %ip\n\r","DevServerIpAddr",dcb->DevServerIpAddr);                    //
//      printd("%-24s: %du\n\r","  Port - Server",dcb->DevServerPort);                      //
//      UTIL_PortNumToStr("  Port - Server", &dcb->DevServerPort);                        // 
//      printd("%-24s: %du\n\r","  Port - Unicast",dcb->DevUnicastPort);                    //
//      printd("%-24s: %du\n\r","  Port - Broadcast",dcb->DevBroadcastPort);                //
//
//      printd("%-24s: %ip\n\r","DevClientIpAddr",dcb->DevClientIPAddr);                    //
//      printd("%-24s: %du\n\r","  Port - Client",dcb->DevClientPort);                      //
//
//      printd("%-24s: %ip\n\r","HostServerIPAddr",dcb->HostServerIPAddr);                  //
//      printd("%-24s: %du\n\r","  Port - Server",dcb->HostServerPort);                     //
// 
//      printd("%-24s: %ip\n\r","HostClientIPAddr",dcb->HostClientIPAddr);                  //
//      printd("%-24s: %du\n\r","  Port - Client",dcb->HostClientPort);                     //
//
//      printd("%-24s: %ip\n\r","Subnet Mask",dcb->SubNetmask);                             //
//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT

//      tt = 32767;
//      printd("\n\r\n\r%4lu %lu\n\r",32767,(long)tt);
//      printd("%-24s: %ip\n\r","test",(long)32767);


//TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT
//
//      UTIL_ByteAryToStr("RuntimeFlags", (byte *)&dcb->BinaryFlags, 1);                    // 
      UTIL_ByteAryToStr("Network", (byte *)&dcb->Network, 2);                             // 
      UTIL_ByteAryToStr("Option", &dcb->Option, 1);                                       // 
      printd("%-24s: %du\n\r","EthernetTxTimer",dcb->EthernetTxTimer);                    //
}

//----------------------------------------------------------------------------------------
// name      : UTIL_PWPrompt
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_PWPrompt(void)
{
      printd("%s%s",Prompt,pwPrompt);
//      enterpw = TRUE;
}

//----------------------------------------------------------------------------------------
// name      : UTIL_Password
// purpose   : 
// params    : 
// returns   : 
// notes     :
//----------------------------------------------------------------------------------------
static      void              UTIL_Password(void *cmd)
{
      char x[16];
      int  err;
  
      if (EEPROM_Read(0xA0, 0xF0, &x, 16) == I2C_OK)
        {
//        printd("cmd=%s\n\r",cmd);
//        printd("xxx=%s\n\r",&x);
        err = UTIL_StrCmp((char *)&x, cmd);
//        printd("err=%du\n\r",err);
        if(err == 0)
          {
          printd("match\n\r");
//          pwcnt = pwTimeout;
          }
        else
          {
          printd("no match\n\r");
//          pwcnt = 0;
          return;
          }          
        }
      else
        {        printd("Read Error.\n");        }
//      enterpw = FALSE;
}

// End of util_menus.c //