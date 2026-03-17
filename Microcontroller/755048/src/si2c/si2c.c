//
//=============================================================================
//     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
//
//     This is unpublished proprietary source code of ASIX Electronic Corporation
//
//     The copyright notice above does not evidence any actual or intended
//     publication of such source code.
//=============================================================================
//
//=============================================================================
// Module Name: si2c.c
// Purpose:
// Author:
// Date:
// Notes:
// $Log: si2c.c,v $
//
//=============================================================================
//

// INCLUDE FILE DECLARATIONS //
#include "si2c.h"
#include "reg80390.h"
// NAMING CONSTANT DECLARATIONS //

// MACRO DECLARATIONS //

// TYPE DECLARATIONS //

// GLOBAL VARIABLES DECLARATIONS //

// LOCAL VARIABLES DECLARATIONS //

// LOCAL SUBPROGRAM DECLARATIONS //
static void I2C_ReadReg(byte RegAddr, byte *pRegData, byte Len);
static void I2C_WriteReg(byte RegAddr, byte *pRegData, byte Len);

// ----------------------------------------------------------------------------
// Function Name : void I2C_ReadReg(byte RegAddr, byte *pRegData, byte Len)
// Purpose       :
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
//
void I2C_ReadReg(byte RegAddr, byte *pRegData, byte Len)
{
	byte ISRtmp;
	
	ISRtmp = EA;
	EA = 0;
	I2CCIR = RegAddr;
	while (Len --)	pRegData[Len] = I2CDR;
	EA = ISRtmp;	
} // End of I2C_ReadReg() //

// ----------------------------------------------------------------------------
// Function Name : void I2C_WriteReg(byte RegAddr, byte *pRegData, byte Len)
// Purpose       :
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
//
void I2C_WriteReg(byte RegAddr, byte *pRegData, byte Len)
{
	byte ISRtmp;
	
	ISRtmp = EA;
	EA = 0;
	while (Len --)	I2CDR = pRegData[Len];
	I2CCIR = RegAddr;	
	EA = ISRtmp;	
} // End of I2C_WriteReg() //

// ----------------------------------------------------------------------------
// Function Name : void I2C_Setup(byte Mode, word PreClk, word SlavDevAddr)
// Purpose       :
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
//
void I2C_Setup(byte Mode, word PreClk, word SlavDevAddr)
{
	//Set Pre-Scale Register
	I2C_WriteReg(I2CCPR, (byte*)&PreClk, 2);
	
	//Set Control Register
	I2C_WriteReg(I2CCTL, (byte*)&Mode, 1);
	
	//Set Slave Device Address Register
	I2C_WriteReg(I2CSDAR, (byte*)&SlavDevAddr, 2);
		
} // End of I2C_Setup() //

// ----------------------------------------------------------------------------
// Function Name : teI2C_STATUS I2C_Read(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
// Purpose       :
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
//
teI2C_STATUS I2C_Read(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
{
      byte MasterStatus, Cmd, Temp;
	
	//Write start mark
      DevAddr &= 0xFE;                                                                    // For write operation	
      Cmd = I2C_START_COND|I2C_CMD_WRITE|I2C_MASTER_GO;                                   //
      I2C_WriteReg(I2CTR, (byte*)&DevAddr, 1);                                            //
      I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);                                              //
      do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);                                         //
      } while (MasterStatus & I2C_TIP);                                                   //
      if (MasterStatus & I2C_NO_ACK) return I2C_NACK_ERR;                                 //
	
      //Write Register address
      Cmd = I2C_CMD_WRITE|I2C_MASTER_GO;                                                  //
      I2C_WriteReg(I2CTR, (byte*)&RegAddr, 1);                                            //
      I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);                                              //
      do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);                                         //
      } while (MasterStatus & I2C_TIP);                                                   //
      if (MasterStatus & I2C_NO_ACK) return I2C_NACK_ERR;                                 //
	
      //Write start mark again
      DevAddr |= 0x01;                                                                    // For read operation
      Cmd = I2C_START_COND|I2C_CMD_WRITE|I2C_MASTER_GO;                                   //
      I2C_WriteReg(I2CTR, (byte*)&DevAddr, 1);                                            //
      I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);                                              //
      do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);                                         //
      } while (MasterStatus & I2C_TIP);                                                   //
      if (MasterStatus & I2C_NO_ACK) return I2C_NACK_ERR;                                 //
	
	//Read data
      while (Len --)                                                                      //
        {
        if (Len)                                                                      //
          Cmd = I2C_CMD_READ|I2C_MASTER_GO;                                       //
        else                                                                          //
          Cmd = I2C_CMD_READ|I2C_MASTER_GO|I2C_STOP_COND;//Send stop mark for last byte read.
			
        I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);
        do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);
           } while (MasterStatus & I2C_TIP);
        I2C_ReadReg(I2CRR, &Temp, 1);
        *pBuf = Temp;
        pBuf ++;
        }
	
	return I2C_OK;	
} // End of I2C_Read() //

// ----------------------------------------------------------------------------
// Function Name : teI2C_STATUS I2C_Write(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
// Purpose       :
// Params        :
// Returns       :
// Note          :
// ----------------------------------------------------------------------------
//
teI2C_STATUS I2C_Write(byte DevAddr, byte RegAddr, byte *pBuf, byte Len)
{
	byte MasterStatus, Cmd, Temp;
	
	//Write start mark
	DevAddr &= 0xFE;                                                        // For write operation	
	Cmd = I2C_START_COND|I2C_CMD_WRITE|I2C_MASTER_GO;                       //
	I2C_WriteReg(I2CTR, (byte*)&DevAddr, 1);                                //
	I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);                                  //
	do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);                             //
	} while (MasterStatus & I2C_TIP);                                       //
	if (MasterStatus & I2C_NO_ACK) return I2C_NACK_ERR;                     //
	
	//Write Register address
	Cmd = I2C_CMD_WRITE|I2C_MASTER_GO;                                      //
	I2C_WriteReg(I2CTR, (byte*)&RegAddr, 1);                                //
	I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);                                  //
	do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);                             //
	} while (MasterStatus & I2C_TIP);                                       //
	if (MasterStatus & I2C_NO_ACK) return I2C_NACK_ERR;                     //
	
	//Write data
	while (Len --)                                                          //
	  {
	  if (Len)                                                              //
	Cmd = I2C_CMD_WRITE|I2C_MASTER_GO;                                      //
		else
			Cmd = I2C_CMD_WRITE|I2C_MASTER_GO|I2C_STOP_COND;            // Send stop mark for last byte read.

		Temp = *pBuf;                                                     //
		I2C_WriteReg(I2CTR, (byte*)&(Temp), 1);                           //
		I2C_WriteReg(I2CCR, (byte*)&(Cmd), 1);                            //
		do { I2C_ReadReg(I2CMSR, &MasterStatus, 1);                       //
		} while (MasterStatus & I2C_TIP);                                 //
		if (MasterStatus & I2C_NO_ACK) return I2C_NACK_ERR;               //
		pBuf ++;
	}
	
	return I2C_OK;	
} // End of I2C_Write() //

// End of si2c.c //