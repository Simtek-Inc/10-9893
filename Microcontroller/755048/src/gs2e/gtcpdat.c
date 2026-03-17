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
 /*============================================================================
 * Module Name: gtcpdat.c
 * Purpose:
 * Author:
 * Date:
 * Notes:
 * $Log: gtcpdat.c,v $
 *
 *=============================================================================
 */

/* INCLUDE FILE DECLARATIONS */
#include "adapter.h"
#include "gtcpdat.h"
#include "gs2e.h"
#include "gconfig.h"
#include "tcpip.h"
#include "mstimer.h"
#include "stoe.h"
#include "uart.h"
#include "hsuart.h"
#include "string.h"

/* NAMING CONSTANT DECLARATIONS */
#define GTCPDAT_MAX_TCP_DATA_LEN  		1460

#define GTCPDAT_MAX_CONNS				1
#define GTCPDAT_NO_NEW_CONN				0xFF

#define GTCPDAT_STATE_FREE				0
#define	GTCPDAT_STATE_WAIT				1
#define	GTCPDAT_STATE_CONNECTED			2

#define GTCPDAT_DATA_INDICATOR   		0x32
#define GTCPDAT_FLOW_CONTROL_INDICATOR	0x33

/* GLOBAL VARIABLES DECLARATIONS */

/* LOCAL VARIABLES DECLARATIONS */
static GTCPDAT_CONN gtcpdat_Conns[GTCPDAT_MAX_CONNS];
static U8_T gtcpdat_InterAppId;
static U8_T gtcpdat_ConnId;
static U8_T gtcpdat_LastTxPktAck; // 1: complete 0: not complete
static U16_T gtcpdat_TxDatLen;
static U32_T gtcpdat_TxWaitTime;
static U8_T gtcpdat_TxBuf[GTCPDAT_TX_BUF_SIZE];
static U16_T gtcpdat_EthernetTxTimer;
static U8_T gtcpdat_TcpClient;

#if GS2E_ENABLE_FLOW_CONTROL			
static U8_T gtcpdat_FlowControlXonRx;
static U8_T gtcpdat_FlowControlModemStatus; 
#endif

/* LOCAL SUBPROGRAM DECLARATIONS */
#if GS2E_ENABLE_FLOW_CONTROL			
static void gtcpdat_HandleFlowControlPacket(U8_T XDATA* pData, U16_T length, U8_T id);
#endif

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Task
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Task(void)
{
	if (gtcpdat_TcpClient == 1)
	{
		U8_T	i;
		U32_T   dip = GCONFIG_GetClientDestIP();
		U16_T   dport = GCONFIG_GetClientDestPort(); 
		
		for (i = 0; i < GTCPDAT_MAX_CONNS; i++)
		{
			if (gtcpdat_Conns[i].State == GTCPDAT_STATE_FREE)
			{
				gtcpdat_ConnId = i;
				gtcpdat_Conns[i].State = GTCPDAT_STATE_CONNECTED;
				gtcpdat_Conns[i].Timer = (U16_T)SWTIMER_Tick();
				gtcpdat_Conns[i].TcpSocket = TCPIP_TcpNew(gtcpdat_InterAppId, gtcpdat_ConnId,	
				                                          GCONFIG_GetClientDestIP(),
											GCONFIG_GetServerDataPktListenPort(),
											GCONFIG_GetClientDestPort());
				TCPIP_TcpConnect(gtcpdat_Conns[i].TcpSocket);
				GS2E_SetTaskState(GS2E_STATE_TCP_DATA_PROCESS);
				gtcpdat_TcpClient++; // To indicate the connection request is done
				printd("Make a TCP connection with host ip %bu.%bu.%bu.%bu at port %du\n\r",
				        (U8_T) ((dip >> 24) & 0x000000FF), (U8_T) ((dip >> 16) & 0x000000FF), 
						(U8_T) ((dip >> 8) & 0x000000FF), (U8_T) (dip & 0x000000FF), (U16_T) dport);
				return;
			}
		}
	}
	else
	{
		U8_T id = gtcpdat_ConnId;
		U32_T time;
		
		if (gtcpdat_Conns[id].State == GTCPDAT_STATE_CONNECTED)
		{		S8_T dat;

				while (readEthTxBufCount() > 0) 
				{
					if (gtcpdat_LastTxPktAck == 1)
					{
						if (gtcpdat_TxDatLen >= GTCPDAT_MAX_TCP_DATA_LEN)
							break;						
					}
					else
					{
						if (gtcpdat_TxDatLen >= GTCPDAT_TX_BUF_SIZE)
							break;
					}
					dat = GetEthChar();
					if (dat >= 0)
						gtcpdat_TxBuf[gtcpdat_TxDatLen++] = (U8_T) dat;					
				}		
				if (gtcpdat_LastTxPktAck == 1 && gtcpdat_TxDatLen > 0)
				{
					if (gtcpdat_TxDatLen >= GTCPDAT_MAX_TCP_DATA_LEN)
					{
						gtcpdat_LastTxPktAck = 0;	
						TCPIP_TcpSend(gtcpdat_Conns[id].TcpSocket, &gtcpdat_TxBuf[0], gtcpdat_TxDatLen, TCPIP_SEND_NOT_FINAL);   	
						gtcpdat_TxWaitTime = 0;
						gtcpdat_TxDatLen -= GTCPDAT_MAX_TCP_DATA_LEN;
						if (gtcpdat_TxDatLen > 0)
							memcpy(&gtcpdat_TxBuf[0], &gtcpdat_TxBuf[GTCPDAT_MAX_TCP_DATA_LEN], gtcpdat_TxDatLen);
					}
					else
					{
						time = SWTIMER_Tick();
		
						if (gtcpdat_TxWaitTime == 0)
						{
							gtcpdat_TxWaitTime = time;
						}
						else
						{
							U32_T elapse;
		 
							if (time >= gtcpdat_TxWaitTime)
								elapse = time - gtcpdat_TxWaitTime;
							else
								elapse = (0xFFFFFFFF - gtcpdat_TxWaitTime) + time;
							if (((elapse * SWTIMER_INTERVAL) >= gtcpdat_EthernetTxTimer))
							{
								gtcpdat_LastTxPktAck = 0;
								TCPIP_TcpSend(gtcpdat_Conns[id].TcpSocket, &gtcpdat_TxBuf[0], gtcpdat_TxDatLen, TCPIP_SEND_NOT_FINAL);	
								gtcpdat_TxDatLen = 0;
								gtcpdat_TxWaitTime = 0;	
							}
						}
					}
			   	}
//				else
//				{
//					if (gtcpdat_TxDatLen > 0)
//				}
		} // GTCPDAT_STATE_CONNECTED)
	} // gtcpdat_TcpClient
} /* End of GTCPDAT_Task() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Init()
 * Purpose: Initialization
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Init(U16_T localPort)
{
	U8_T	i;

	gtcpdat_TcpClient = 0;
	for (i = 0; i < GTCPDAT_MAX_CONNS; i++)
		gtcpdat_Conns[i].State = GTCPDAT_STATE_FREE;
	gtcpdat_InterAppId = TCPIP_Bind(GTCPDAT_NewConn, GTCPDAT_Event, GTCPDAT_Receive);
	if ((GCONFIG_GetNetwork() & GCONFIG_NETWORK_CLIENT) != GCONFIG_NETWORK_CLIENT)
	{	
		/* unicast packet */
		TCPIP_TcpListen(localPort, gtcpdat_InterAppId);
		printd("TCP data server init ok.\n\r");
	}
	else
	{
		gtcpdat_TcpClient = 1;
		printd("TCP data client init ok.\n\r");	
	}
	gtcpdat_LastTxPktAck = 1;
	gtcpdat_TxDatLen = 0;
	gtcpdat_TxWaitTime = 0;
	gtcpdat_EthernetTxTimer = GCONFIG_GetEthernetTxTimer();
} /* End of GTCPDAT_Init() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_NewConn
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
U8_T GTCPDAT_NewConn(U32_T XDATA* pip, U16_T remotePort, U8_T socket)
{
	U8_T	i;

	pip = pip;
	remotePort = remotePort;
	for (i = 0; i < GTCPDAT_MAX_CONNS; i++)
	{
		if (gtcpdat_Conns[i].State == GTCPDAT_STATE_FREE)
		{
			gtcpdat_Conns[i].State = GTCPDAT_STATE_WAIT;
			gtcpdat_Conns[i].Timer = (U16_T)SWTIMER_Tick();
			gtcpdat_Conns[i].Ip = *pip;
			gtcpdat_Conns[i].Port = remotePort;
			gtcpdat_Conns[i].TcpSocket = socket;
			gtcpdat_ConnId = i;
			printd("New TCP connection: id=%bu, remotePort=%du, socket=%bu\n\r", i, remotePort, socket);
			return i;
		}
	}
	return GTCPDAT_NO_NEW_CONN;
} /* End of GTCPDAT_NewConn() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Event
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Event(U8_T id, U8_T event)
{ 
	if (event == TCPIP_CONNECT_ACTIVE)
	{
		if (gtcpdat_Conns[id].State == GTCPDAT_STATE_WAIT)
		{
			gtcpdat_Conns[id].State = GTCPDAT_STATE_CONNECTED;
			GS2E_SetTaskState(GS2E_STATE_TCP_DATA_PROCESS);
			gtcpdat_LastTxPktAck = 1;
			gtcpdat_TxDatLen = 0;
			gtcpdat_TxWaitTime = 0;
		}
	}
	else if (event == TCPIP_CONNECT_CANCEL)
	{
		if (gtcpdat_Conns[id].State == GTCPDAT_STATE_CONNECTED)
		{
			gtcpdat_Conns[id].State = GTCPDAT_STATE_FREE;
			GS2E_SetTaskState(GS2E_STATE_IDLE);			
		}
	}
	else if (event == TCPIP_CONNECT_XMIT_COMPLETE)
	{
		gtcpdat_LastTxPktAck = 1;
	}
} /* End of GTCPDAT_Event() */

/*
 * ----------------------------------------------------------------------------
 * Function Name: GTCPDAT_Receive
 * Purpose: 
 * Params:
 * Returns:
 * Note:
 * ----------------------------------------------------------------------------
 */
void GTCPDAT_Receive(U8_T XDATA* pData, U16_T length, U8_T id)
{
	U16_T i;

	if (gtcpdat_Conns[id].State == GTCPDAT_STATE_CONNECTED)
	{
		if (length <= 1460) // Is valid length
		{
			for (i = 0; i < length; i++)
			{						
				pData = pData;		// used to replace following line to avoid warning
//				HSUR_PutChar((S8_T) *(pData + i));
			}
		}
	}	
} /* End of GTCPDAT_Receive() */

/* End of gtcpdat.c */