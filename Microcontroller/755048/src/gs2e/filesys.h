/*
 ******************************************************************************
 *     Copyright (c) 2006 	ASIX Electronic Corporation      All rights reserved.
 *
 *     This is unpublished proprietary source code of ASIX Electronic Corporation
 *
 *     The copyright notice above does not evidence any actual or intended
 *     publication of such source code.
 ******************************************************************************
 */
/*=============================================================================
 * Module Name: filesys.h
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: filesys.h,v $
 *=============================================================================
 */

#ifndef __FILE_SYSTEM_H__
#define __FILE_SYSTEM_H__


/* INCLUDE FILE DECLARATIONS */
#include "types.h"

/* NAMING CONSTANT DECLARATIONS */
#define FILE_SETTING			0
#define FILE_LOGO				1
#define FILE_ASIX				2
#define FILE_MESSAGE			3

#define RECORD_SELECT_DBR		0
#define RECORD_SELECT_DATA		1
#define RECORD_SELECT_PARITY	2
#define RECORD_SELECT_STOP		3
#define RECORD_SELECT_FLOW		4
#define RECORD_CHECKBOX_DHCP	5
#define RECORD_TEXT_IP			6
#define RECORD_TEXT_MASK		7
#define RECORD_TEXT_GATEWAY		8
#define RECORD_TEXT_DNS			9
#define RECORD_SELECT_CONNTYPE	10
#define RECORD_TEXT_SLP			11
#define RECORD_TEXT_TXTIMER		12
#define RECORD_TEXT_DIP			13
#define RECORD_TEXT_DPORT		14
#define RECORD_CHECKBOX_REBOOT	15
#define RECORD_SUBMIT_APPLY		16
#define RECORD_SUBMIT_YES		17

#define MAX_STORE_FILE_NUM		4
#define MAX_FILE_NAME_LEN		16

#define FILE_TYPE_BIN			BIT0
#define FILE_TYPE_CAMERA		BIT1

#define MAX_POST_RECORDS		17
#define MAX_POST_BUF_SUBMIT		1
#define MAX_POST_BUF_TEXT		8
#define MAX_POST_BUF_RADIO		0
#define MAX_POST_BUF_CHECKBOX	2
#define MAX_POST_BUF_SELECT		6
#define MAX_POST_NAME_LEN		15
#define MAX_POST_VALUE_LEN		16

#define POST_TYPE_PASSWORD		1
#define POST_TYPE_SUBMIT		2
#define POST_TYPE_TEXT			3
#define POST_TYPE_RADIO			4
#define POST_TYPE_CHECKBOX		5
#define POST_TYPE_SELECT		6

/* TYPE DECLARATIONS */
/*-------------------------------------------------------------*/
typedef struct _FILE_MANAGEMEMT
{
	U8_T			Occupy;
	U8_T			FType;
	U8_T			FName[MAX_FILE_NAME_LEN];
	U16_T			FileSize;
	U8_T			*PBuf;

} FILE_MANAGEMEMT;

/*-------------------------------------------------------------*/
typedef struct _POST_RECORD
{
	U8_T			Occupy;
	U8_T			PostType;
	U8_T			Name[MAX_POST_NAME_LEN];
	U8_T			NameLen;
	U8_T			FileCount;
	U8_T			FileIndex;
	void XDATA*		PValue;

} POST_RECORD;

/*-------------------------------------------------------------*/
typedef struct _BUF_SUBMIT
{
	U8_T			Value[MAX_POST_VALUE_LEN];
	U8_T			DefaultVlaueLen;
	U8_T			IsApply;

} BUF_SUBMIT;

/*-------------------------------------------------------------*/
typedef struct _BUF_RADIO
{
	U16_T			FirstOffset;
	U16_T			SecondOffset;
	U16_T			ThirdOffset;
	U8_T			FirstValue[MAX_POST_VALUE_LEN];
	U8_T			SecondValue[MAX_POST_VALUE_LEN];
	U8_T			ThirdValue[MAX_POST_VALUE_LEN];
	U8_T			FirstLength;
	U8_T			SecondLength;
	U8_T			ThirdLength;
	U8_T			DefaultSet;
	U8_T			CurrentSet;
	U8_T			UserSet;

} BUF_RADIO;

/*-------------------------------------------------------------*/
typedef struct _BUF_TEXT
{
	U16_T			Offset;
	U8_T			CurrValue[MAX_POST_VALUE_LEN];
	U8_T			UserValue[MAX_POST_VALUE_LEN];
	U8_T			DefaultLength;
	U8_T			CurrLength;
	U8_T			UserLength;

} BUF_TEXT;

/*-------------------------------------------------------------*/
typedef struct _BUF_CHECKBOX
{
	U16_T			Offset;
	U8_T			DefaultSet;
	U8_T			CurrentSet;
	U8_T			UserSet;

} BUF_CHECKBOX;

/*-------------------------------------------------------------*/
typedef struct _BUF_SELECT
{
	U16_T			Offset[9];
	U8_T			Count;
	U8_T			DefaultSet;
	U8_T			CurrentSet;
	U8_T			UserSet;

} BUF_SELECT;

/*-------------------------------------------------------------*/
/* EXPORTED SUBPROGRAM SPECIFICATIONS */
extern FILE_MANAGEMEMT XDATA FSYS_Manage[MAX_STORE_FILE_NUM];
extern POST_RECORD XDATA POST_Record[MAX_POST_RECORDS];

void FSYS_Init(void);
U8_T FSYS_FindFile(U8_T XDATA*);
U16_T FSYS_HtmlExpansion(U8_T);


#endif /* End of __FILE_SYSTEM_H__ */


/* End of filesys.h */