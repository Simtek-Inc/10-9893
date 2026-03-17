/*
 ******************************************************************************
 *     Copyright (c) 2006	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended 
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name:eeprom.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: eeprom.h,v $
 *
 *=============================================================================
 */

#ifndef __EEPROM_H__
#define __EEPROM_H__

/* INCLUDE FILE DECLARATIONS */
#include "types.h"
#include "si2c.h"

/* NAMING CONSTANT DECLARATIONS */
#define	DEVICE_ADDR	(0xA0)

/* TYPE DECLARATIONS */

/* GLOBAL VARIABLES */

/* EXPORTED SUBPROGRAM SPECIFICATIONS */
void EEPROM_Init(void);
teI2C_STATUS EEPROM_Read(byte DevAddr, byte RegAddr, byte *pBuf, byte Len);
teI2C_STATUS EEPROM_Write(byte DevAddr, byte RegAddr, byte *pBuf, byte Len);
#endif /* End of __EEPCONFIG_H__ */

/* End of eepconfig.h */

