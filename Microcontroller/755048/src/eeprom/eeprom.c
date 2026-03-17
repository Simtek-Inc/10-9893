//=============================================================================
//     Copyright (c) 2006     ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//=============================================================================
//
//=============================================================================
// Module Name     : eeprom.c
// Purpose       :
// Author        :
// Date                  :
// Note          :
// $Log                  : eeprom.c,v $
//=============================================================================

// INCLUDE FILE DECLARATIONS //
#include "eeprom.h"
#include "uart.h"
#include "reg80390.h"
#include "ax11000.h"
#include "interrupt.h"
#include "delay.h"

#include <string.h>
#include "printd.h"
// NAMING CONSTANT DECLARATIONS //

// MACRO DECLARATIONS //

// TYPE DECLARATIONS //

// GLOBAL VARIABLES DECLARATIONS //

// LOCAL VARIABLES DECLARATIONS //

// LOCAL SUBPROGRAM DECLARATIONS //
static void EEPCFG_ChgByteOrder(byte *pBuf, byte Len);

// ----------------------------------------------------------------------------
// Function Name : EEPROM_Init
// Purpose       :
// Params          :
// Returns         :
// Note                  :
// ----------------------------------------------------------------------------
void EEPROM_Init(void)
{
      // Disable I2C interrupt.

      switch (CSREPR & (BIT6|BIT7))
      {
      case SYS_CLK_100M:
            // I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 100M
            I2C_Setup(I2C_ENB|I2C_FAST|I2C_7BIT|I2C_MASTER_MODE, I2C_FAST_100M, 0x005A);
            break;
      case SYS_CLK_50M:
            // I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 50M
            I2C_Setup(I2C_ENB|I2C_FAST|I2C_7BIT|I2C_MASTER_MODE, I2C_FAST_50M, 0x005A);
            break;
      case SYS_CLK_25M:
            // I2C master mode, interrupt enable, fast mode in slave, 7-bits address, 400KHz at 25M
            I2C_Setup(I2C_ENB|I2C_FAST|I2C_7BIT|I2C_MASTER_MODE, I2C_FAST_25M, 0x005A);
            break;
      }

} // End of EEPROM_Init() //

// ----------------------------------------------------------------------------
// Function Name : teI2C_STATUS EEPROM_Read(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
// Purpose       :
// Params          :
// Returns         :
// Note                  :
// ----------------------------------------------------------------------------
teI2C_STATUS EEPROM_Read(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
{
      return I2C_Read(DevAddr, RegAddr, pBuf, Len);
}

// ----------------------------------------------------------------------------
// Function Name : teI2C_STATUS EEPROM_Write(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
// Purpose       :
// Params          :
// Returns         :
// Note                  :
// ----------------------------------------------------------------------------
teI2C_STATUS EEPROM_Write(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
{
      byte LenTemp, ErrFlag = 0;
      
      while (Len)
      {
            LenTemp = 0x08 - (RegAddr & 0x07);
            if (LenTemp > Len)
                  LenTemp = Len;
                  
            ErrFlag = I2C_Write(DevAddr, RegAddr, pBuf, LenTemp);
            
            Len -= LenTemp;
            RegAddr += LenTemp;
            pBuf += LenTemp;
            
            DELAY_Ms(10);
      }
      return ErrFlag;
}

// End of eeprom.c //