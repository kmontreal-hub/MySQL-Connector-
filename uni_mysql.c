/*-------------------------------------------------------------------------------------------------------------------------
;	File: 		uni_mysql.c
;	Author:		KDM
;
;	Customer: 	
;
;	Purpose:	MySQL database connector
;	
;
-------------------------------------------------------------------------------------------------------------------------*/


/* INCLUDES */
#include "uni_mysql.h"


/* MACROS */
#define UNI_MYSQL_CLIENT_CAP		0xA28D;
#define UNI_MYSQL_EXT_CAP			0x0002;
#define UNI_MYSQL_MAX_PAC_SIZE		1073741824
#define UNI_MYSQL_CHARSET_NUM		0x21

#define UNI_MYSQL_QUERY_CMD			0x03
#define UNI_MYSQL_CLOSE_CONN_CMD	0x01	


/* --OPERATE CLIENT STATES-- */
typedef enum
{
  /*12345678901234567890123456789012*/
	UNI_MYSQL_CL_SM_INIT=	0,
	UNI_MYSQL_CL_SM_OPEN,
	UNI_MYSQL_CL_SM_PREP,
	UNI_MYSQL_CL_SM_START_CLIENT,
	UNI_MYSQL_CL_SM_GET_SERV_DATA,
	UNI_MYSQL_CL_SM_SEND_CLIENT_DATA,
	UNI_MYSQL_CL_SM_RTRN_PCKT,
	UNI_MYSQL_CL_SM_CYC_Q,
	UNI_MYSQL_CL_SM_RETURN_DATA,
	UNI_MYSQL_CL_SM_CLOSE,
}UNI_MYSQL_CL_SM_STATES_typ;

/* --QUERY FUB STATES-- */
typedef enum
{
	UNI_MYSQL_QUERY_SM_INIT=	0,
	UNI_MYSQL_QUERY_SM_SEND_CMD,
	UNI_MYSQL_QUERY_SM_RETRN_PCKT,
}UNI_MYSQL_QUERY_SM_STATES_typ;

/* --SEND COMMAND FUB STATES-- */
typedef enum
{
	UNI_MYSQL_SNDCMD_SM_INIT=	0,
	UNI_MYSQL_SNDCMD_SM_SEND,
}UNI_MYSQL_SNDCMD_SM_STATES;

/* --PARSE RETURN PACKET FUB STATES-- */
typedef enum
{
	UNI_MYSQL_RTRN_SM_INIT=	0,
	UNI_MYSQL_RTRN_SM_REC,
	UNI_MYSQL_RTRN_SM_CHECK,
	UNI_MYSQL_RTRN_SM_ACT,
	
}UNI_MYSQL_RTRN_SM_STATES_typ;

/* --RECIEVE DATA FUB STATES-- */
typedef enum
{
	UNI_MSQL_REC_DATA_SM_INIT=	0,
	UNI_MSQL_REC_DATA_SM_GET_SIZE,
	UNI_MSQL_REC_DATA_SM_RECIEVE,	
}UNI_MSQL_REC_DATA_SM_STATES_typ;

/* --SEND DATA FUB STATES-- */
typedef enum
{
	UNI_MYSQL_SND_DATA_SM_INIT=	0,
	UNI_MYSQL_SND_DATA_SM_SEND,
}UNI_MYSQL_SND_DATA_SM_typ;

/* --CHECK PACKET TYPE FUB STATES-- */
typedef enum
{
	UNI_MYSQL_CHK_TYP_SM_FIRST= 0,
	UNI_MYSQL_CHK_TYP_SM_RTRN_PKT,
}UNI_MYSQL_CHK_TYP_SM_typ;


/* ------------------------------LOCAL FUNCTION PROTOTYPES------------------------------ */
/* Init query function */
UINT UniMySql_Query_Init(UNI_MYSQL_typ *t);
/* Query database using input message */																
UINT UniMySql_Query(UNI_MYSQL_typ *t, STRING *pMessage);
/* Send command init */
UINT UniMySql_SendCommand_Init(UNI_MYSQL_typ *t);
/* Send command to MySQL server */
UINT UniMySql_SendCommand(UNI_MYSQL_typ *t, USINT Command, STRING *pMessage);
/* Send data packet init */
UINT UniMySql_SendDataPacket_Init(UNI_MYSQL_typ *t);
/* Send data packet */
UINT UniMySql_SendDataPacket(UNI_MYSQL_typ *t, USINT packetNumber);
/* Parse return packet init */
UINT UniMySql_ParseReturnPacket_Init(UNI_MYSQL_typ *t);
/* Parse return packet from MySQL sever */
UINT UniMySql_ParseReturnPacket(UNI_MYSQL_typ *t);
/* Init recieve data packet */
UINT UniMySql_RecieveDataPacket_Init(UNI_MYSQL_typ *t);
/* Recieve data packet from mysql server */
UINT UniMySql_ReceiveDataPacket(UNI_MYSQL_typ *t);
/* Get Packet Return Type Init */
UINT UniMySql_GetReturnType_Init(UNI_MYSQL_typ *t);
/* Get Packet Return Type */
UINT UniMySql_GetReturnType(UNI_MYSQL_typ *t);
/* Get packet class */
USINT UniMySql_GetPacketClass(USINT firstElement);
UINT UniMySql_CheckPacketClass(UNI_MYSQL_typ *t);

/* Parse first packet from server */
UINT UniMySql_ParseInitialServerData(UNI_MYSQL_typ *t);
/* Build first client packet from initial server data */
UINT UniMySql_BuildInitialClientData(UNI_MYSQL_typ *t);
/* Encrypt user password */
UINT UniMySql_EncryptPassword(USINT *ScrambleBuffer, STRING *Password, USINT *OutputByteArray);

/* Update error string from MySQL server */
void UniMySql_UpdateMySQLError(UNI_MYSQL_typ *t, STRING *pErrorMsg);

/* ------------------------------LOCAL VARAIBLES------------------------------ */
_LOCAL STRING QueryString[4996];	/* temporary string for holding query string */






/*--------------------------------------*/
/*			INIT MYSQL CLIENT			*/
/*--------------------------------------*/
void UniMySql_Init(UNI_MYSQL_PUB_typ *H, UNI_MYSQL_typ *t)
{
	/* Init SM */
	t->SM.State=	UNI_MYSQL_CL_SM_INIT;
	UniSM_InitFirstScan(&t->SM);

	t->Ident=		ZERO;
	t->isConnected=	FALSE;

	memset(&t->Client.username, ZERO, sizeof(t->Client.username));
	memset(&t->Client.password, ZERO, sizeof(t->Client.password));
	memset(&t->Client.database, ZERO, sizeof(t->Client.database));

	/* Setup the constants */
	t->Client.clientCapabilities=	UNI_MYSQL_CLIENT_CAP;
	t->Client.extendedCapabilities=	UNI_MYSQL_EXT_CAP;
	t->Client.maxPacketSize=		UNI_MYSQL_MAX_PAC_SIZE;
	t->Client.charsetNumber=		UNI_MYSQL_CHARSET_NUM;

}


/*--------------------------------------*/
/*			CYCLE MYSQL CLIENT			*/
/*--------------------------------------*/
void UniMySql_Operate(UNI_MYSQL_PUB_typ *H, UNI_MYSQL_typ *t)
{
	STRING tempString[99];

	/**************************************************/
	UniSM_CycleFirstScan(&t->SM);
	/**************************************************/
	switch(t->SM.State){
		/*--*/
		case UNI_MYSQL_CL_SM_INIT:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_INIT");
				/* Not Connected */
				t->isConnected=	FALSE;
				t->status=		ZERO;
				/* Pass in config from public */
				strcpy(t->Client.username, H->Config.username);
				strcpy(t->Client.password, H->Config.password);
				strcpy(t->Client.database, H->Config.databaseName);
				strcpy(t->myIP, H->Config.clientIP);
				strcpy(t->Server.IPAddr, H->Config.serverIP);
				t->pDevice=		H->Config.Device;
				t->Port= 		H->Config.port;
				t->Server.port=	H->Config.port;
			}

			/* Go open port */
			t->SM.State=	UNI_MYSQL_CL_SM_OPEN;
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_OPEN:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_OPEN");
				/* Not Connected */
				t->isConnected=	FALSE;
				/* Call Open Fub Init */
				UniTcp_Open_Init(&t->TcpWrapper.Open);
				/* Load Fub */
				t->TcpWrapper.Open.pIPAddress=	t->myIP;
				t->TcpWrapper.Open.Port=		t->Port;
			}

			/* Cycle Fub */
			UniTcp_Open(&t->TcpWrapper.Open);


			/* Wait for done */
			if(t->TcpWrapper.Open.Done){
				/* Eval */
				switch(t->TcpWrapper.Open.ErrorID){
					/*--*/
					case ZERO:
						/* Capture Ident */
						t->Ident=		t->TcpWrapper.Open.Ident;
						/* Move to prep port */
						t->SM.State=	UNI_MYSQL_CL_SM_PREP;
					break;
					/*--*/
					default:
						/* Failed to open, try closing and then re-opening */
						t->status=		t->TcpWrapper.Open.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_PREP:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_PREP");
				/* Call init for fub */
				UniTcp_IOCtl_Init(&t->TcpWrapper.IOCtl);
				/* load fub */
				t->TcpWrapper.IOCtl.LingerOptions.lLinger=	ZERO;
				t->TcpWrapper.IOCtl.LingerOptions.lOnOff=	TRUE;
				t->TcpWrapper.IOCtl.Ident=					t->Ident;
				t->TcpWrapper.IOCtl.IOControlCode=			tcpSO_LINGER_SET;		
			}

			/* Cycle Fub */
			UniTcp_IOCtl(&t->TcpWrapper.IOCtl);
			
			/* wait for done */
			if(t->TcpWrapper.IOCtl.Done){
				/* Eval */
				switch(t->TcpWrapper.IOCtl.ErrorID){
					/*--*/
					case ZERO:
						/* Go start client */
						t->SM.State=	UNI_MYSQL_CL_SM_START_CLIENT;
					break;
					/*--*/
					default:
						/* failed, go close the port */
						t->status=		t->TcpWrapper.IOCtl.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_START_CLIENT:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_START_CLIENT");
				/* init the fub */
				UniTcp_Client_Init(&t->TcpWrapper.Client);
				/* load fub */
				t->TcpWrapper.Client.Ident=		t->Ident;
				t->TcpWrapper.Client.Port=		t->Server.port;
				t->TcpWrapper.Client.pServerIP=	&t->Server.IPAddr[ZERO];
			}

			/* Cycle Fub */
			UniTcp_Client(&t->TcpWrapper.Client);

			/* wait for done */
			if(t->TcpWrapper.Client.Done){
				/* Eval */
				switch(t->TcpWrapper.Client.ErrorID){
					/*--*/
					case ZERO:
						/* Go Authenticate */
						t->SM.State=	UNI_MYSQL_CL_SM_GET_SERV_DATA;
					break;
					/*--*/
					case tcpERR_INVALID:
						t->status=		t->TcpWrapper.Client.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;
					break;
					/*--*/
					default:
						/* failed */
						t->status=		t->TcpWrapper.Client.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_GET_SERV_DATA:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_GET_SERV_DATA");
				UniMySql_RecieveDataPacket_Init(t);
			}

			UniMySql_ReceiveDataPacket(t);

			if(t->Client.RecieveData.Done){
				switch(t->Client.RecieveData.ErrorID){
					/*--*/
					case ZERO:
						memcpy(&t->Server.initialServerDataBuffer, &t->Client.RecieveData.dataOut, sizeof(t->Server.initialServerDataBuffer));
						/* if incoming packet is NOT an error packet */
						if(t->Client.RecieveData.dataOut[ZERO] != UNI_MYSQL_PACKET_CLS_ERR){
							/* parse data from server */
							UniMySql_ParseInitialServerData(t);
							/* build client data packet from server data and constants */
							UniMySql_BuildInitialClientData(t);
						}else{
							/* if it is an error packet, store the error and go try again */
							memset(&tempString, ZERO, sizeof(tempString));
							memcpy(&tempString, &t->Client.RecieveData.dataOut[3], (sizeof(tempString)-1));
							t->lastMySQLErrorNum= BuildWord(t->Client.RecieveData.dataOut[2], t->Client.RecieveData.dataOut[1]);
							UniMySql_UpdateMySQLError(t, tempString);
							/* Close the port */
							t->SM.State= UNI_MYSQL_CL_SM_CLOSE;
						}
						/* go send */
						t->SM.State=	UNI_MYSQL_CL_SM_SEND_CLIENT_DATA;
					break;
					/*--*/
					default:
						t->status=		t->Client.RecieveData.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_SEND_CLIENT_DATA:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_SEND_CLIENT_DATA");
				/* init send */
				UniMySql_SendDataPacket_Init(t);
			}

			/* Cycle */
			UniMySql_SendDataPacket(t, 1);

			if(t->Client.SendData.Done){
				switch(t->Client.SendData.ErrorID){
					/*--*/
					case ZERO:
						t->SM.State=	UNI_MYSQL_CL_SM_RTRN_PCKT;
					break;
					/*--*/
					default:
						t->status=		t->Client.SendData.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;						
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_RTRN_PCKT:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_RTRN_PCKT");
				/* Init Recieve function */

				UniMySql_ParseReturnPacket_Init(t);
			}

			/* Cycle the recieve */
			UniMySql_ParseReturnPacket(t);

			if(t->Client.ParseReturnPacket.Done){
				switch(t->Client.ParseReturnPacket.ErrorID){
					/*--*/
					case ZERO:
						t->SM.State=	UNI_MYSQL_CL_SM_CYC_Q;
					break;
					/*--*/
					default:
						t->status=		t->Client.ParseReturnPacket.ErrorID;
						t->SM.State=	UNI_MYSQL_CL_SM_CLOSE;							
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_CYC_Q:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_SEND_Q");
				/* we are connected */
				t->isConnected=	TRUE;
				/* Init fub */
				UniMySql_Query_Init(t);
			}

			/* If there is no query to execute, bail */
			if(H->Query.execute == FALSE){
				break;
			}else{
				/* If we have a query to execute, copy in the query string */
				strcpy(QueryString, H->Query.pQueryString);
			}

			UniMySql_Query(t, QueryString);
			/* Set busy */
			H->Query.status=	UNI_MYSQL_ERR_BUSY;

			if(t->Client.Query.Done){
				switch(t->Client.Query.ErrorID){
					/*--*/
					case ZERO:
						/* copy query return data and set status */
						memset(&QueryString, ZERO, sizeof(QueryString));
						H->Query.execute=	FALSE;
						if(H->Query.pReturnData != ZERO){
							memset(H->Query.pReturnData, ZERO, sizeof(*H->Query.pReturnData));
							memcpy(H->Query.pReturnData, &t->Client.Query.ReturnData, sizeof(*H->Query.pReturnData));
						}
						H->Query.status=	UNI_MYSQL_ERR_OK;
						t->SM.State=		UNI_MYSQL_CL_SM_RETURN_DATA;
					break;
					/*--*/
					default:
						if(H->Query.pReturnData != ZERO){
							memset(H->Query.pReturnData, ZERO, sizeof(*H->Query.pReturnData));
						}
						H->Query.status=	UNI_MYSQL_ERR_NOT_CONN;
						t->isConnected=		FALSE;
						t->SM.State=		UNI_MYSQL_CL_SM_CLOSE;
					break;
					/*--*/
				}
				/* release the connection */
				UniMySql_ReleaseConnection(H);
			}
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_RETURN_DATA:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_RETURN_DATA");
			}

			t->SM.State=	UNI_MYSQL_CL_SM_CYC_Q;
		break;
		/*--*/
		case UNI_MYSQL_CL_SM_CLOSE:
			if(t->SM.FirstScan){
				UniSM_UpdateStateDesc(&t->SM, "UNI_MYSQL_CL_SM_CLOSE");
				/* Not connected */
				t->isConnected=	FALSE;
				/* Init fub */
				UniTcp_Close_Init(&t->TcpWrapper.Close);
				/* load fub */
				t->TcpWrapper.Close.Ident=				t->Ident;
				t->TcpWrapper.Close.ShutdownBehavior=	0;
			}

			UniTcp_Close(&t->TcpWrapper.Close);

			if(t->TcpWrapper.Close.Done){
				t->Ident=		ZERO;
				t->SM.State=	UNI_MYSQL_CL_SM_INIT;
			}
		break;
		/*--*/
		default:
			/* Invalid state */
			t->SM.State=	UNI_MYSQL_CL_SM_INIT;
		break;
		/*--*/
	}

	/* update is connected flag */
	H->isConnected=	t->isConnected;
}


void UniMySql_Exit(UNI_MYSQL_typ *t)
{
	
}



/*--------------------------------------*/
/*				INIT QUERY				*/
/*--------------------------------------*/
UINT UniMySql_Query_Init(UNI_MYSQL_typ *t)
{
	UniSM_InitFirstScan(&t->Client.Query.SM);
	t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_INIT;

	t->Client.Query.Done=		FALSE;
	t->Client.Query.ErrorID=	ZERO;

	return(ZERO);
}

/*--------------------------------------*/
/*				QUERY					*/
/*--------------------------------------*/
UINT UniMySql_Query(UNI_MYSQL_typ *t, STRING *pMessage)
{

	/*************************************************/
	UniSM_CycleFirstScan(&t->Client.Query.SM);
	/*************************************************/
	switch(t->Client.Query.SM.State){
		/*--*/
		case UNI_MYSQL_QUERY_SM_INIT:
			if(t->Client.Query.SM.FirstScan){
				UniSM_UpdateStateDesc(&t->Client.Query.SM, "UNI_MYSQL_QUERY_SM_INIT");
			}

			/* Go send query command */
			t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_SEND_CMD;
		break;
		/*--*/
		case UNI_MYSQL_QUERY_SM_SEND_CMD:
			if(t->Client.Query.SM.FirstScan){
				UniSM_UpdateStateDesc(&t->Client.Query.SM, "UNI_MYSQL_QUERY_SM_SEND_CMD");
				/* Init */
				UniMySql_SendCommand_Init(t);
			}

			UniMySql_SendCommand(t, UNI_MYSQL_QUERY_CMD, pMessage);

			if(t->Client.SendCmd.Done){
				switch(t->Client.SendCmd.ErrorID){
					/*--*/
					case ZERO:
						t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_RETRN_PCKT;
					break;
					/*--*/
					default:
						t->Client.Query.Done=		TRUE;
						t->Client.Query.ErrorID=	t->Client.SendCmd.ErrorID;
						t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_INIT;
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		case UNI_MYSQL_QUERY_SM_RETRN_PCKT:
			if(t->Client.Query.SM.FirstScan){
				UniSM_UpdateStateDesc(&t->Client.Query.SM, "UNI_MYSQL_QUERY_SM_RETRN_PCKT");
				/* Init function */
				UniMySql_ParseReturnPacket_Init(t);
			}

			/* Cycle the recieve */
			UniMySql_ParseReturnPacket(t);

			if(t->Client.ParseReturnPacket.Done){
				switch(t->Client.ParseReturnPacket.ErrorID){
					/*--*/
					case ZERO:
						/* Query successful, copy data */
						memset(&t->Client.Query.ReturnData, ZERO, sizeof(t->Client.Query.ReturnData));
						memcpy(&t->Client.Query.ReturnData, &t->Client.ParseReturnPacket.ReturnData, sizeof(t->Client.Query.ReturnData));
						t->Client.Query.Done=		TRUE;
						t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_INIT;
					break;
					/*--*/
					default:
						t->Client.Query.Done=		TRUE;
						t->Client.Query.ErrorID=	t->Client.ParseReturnPacket.ErrorID;
						t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_INIT;
					break;
					/*--*/
				}
			}
		break;
		/*--*/
		default:
			/* Invalid state */
			t->Client.Query.SM.State=	UNI_MYSQL_QUERY_SM_INIT;
		break;
		/*--*/
	}

	return(ZERO);
}


/*--------------------------------------*/
/*		INIT PARSE RETURN PACKET		*/
/*--------------------------------------*/
UINT UniMySql_ParseReturnPacket_Init(UNI_MYSQL_typ *t)
{
	UniSM_InitFirstScan(&t->Client.ParseReturnPacket.SM);
	t->Client.ParseReturnPacket.SM.State=	UNI_MYSQL_RTRN_SM_INIT;

	UniMySql_GetReturnType_Init(t);

	t->Client.ParseReturnPacket.Done=		ZERO;
	t->Client.ParseReturnPacket.ErrorID=	ZERO;

	t->Client.ParseReturnPacket.ReturnData.numRecordsReturned=		ZERO;

	memset(&t->Client.ParseReturnPacket.ReturnData.dataOut, ZERO, sizeof(t->Client.ParseReturnPacket.ReturnData.dataOut));

	return(ZERO);
}


/*--------------------------------------*/
/*			PARSE RETURN PACKET			*/
/*--------------------------------------*/
UINT UniMySql_ParseReturnPacket(UNI_MYSQL_typ *t)
{
	static USINT returnPacketType;
	static UINT dataIndex;
	static UINT dataRow;
	STRING tempString[99];

	/************************************************/
	UniSM_CycleFirstScan(&t->Client.ParseReturnPacket.SM);
	/************************************************/
	switch(t->Client.ParseReturnPacket.SM.State){
		/*--*/
		case UNI_MYSQL_RTRN_SM_INIT:
			if(t->Client.ParseReturnPacket.SM.FirstScan){
				
			}
			dataIndex= 0;
			dataRow= 0;

			t->Client.ParseReturnPacket.SM.State=	UNI_MYSQL_RTRN_SM_REC;
		break;
		/*--*/
		case UNI_MYSQL_RTRN_SM_REC:
			if(t->Client.ParseReturnPacket.SM.FirstScan){
				UniMySql_RecieveDataPacket_Init(t);
				UniTimer_Init(&t->Client.ParseReturnPacket.SM.Tmr);
				UniTimer_Set(&t->Client.ParseReturnPacket.SM.Tmr, t->connTimeOut);
			}

			/* Cycle Fub */
			UniMySql_ReceiveDataPacket(t);

			/* Increment Timeout Timer */
			UniTimer_Increment(&t->Client.ParseReturnPacket.SM.Tmr);

			if(t->Client.RecieveData.Done){
				switch(t->Client.RecieveData.ErrorID){
					/*--*/
					case ZERO:
						t->Client.ParseReturnPacket.SM.State=	UNI_MYSQL_RTRN_SM_CHECK;
					break;
					/*--*/
					case tcpERR_NO_DATA:	
						/* wait */
					break;
					/*--*/
					default:
						t->Client.ParseReturnPacket.Done=		TRUE;
						t->Client.ParseReturnPacket.ErrorID=	t->Client.RecieveData.ErrorID;
						t->Client.ParseReturnPacket.SM.State=	UNI_MYSQL_RTRN_SM_INIT;
					break;
					/*--*/
				}
			}

			if(UniTimer_IsExpired(&t->Client.ParseReturnPacket.SM.Tmr) == TRUE){
				t->Client.ParseReturnPacket.Done=		TRUE;
				t->Client.ParseReturnPacket.ErrorID=	UNI_MYSQL_ERR_TIMEOUT;
				t->Client.ParseReturnPacket.SM.State=	UNI_MYSQL_RTRN_SM_INIT;				
			}
		break;
		/*--*/
		case UNI_MYSQL_RTRN_SM_CHECK:
			if(t->Client.ParseReturnPacket.SM.FirstScan){
					
			}

			returnPacketType=	UniMySql_GetReturnType(t);


			t->Client.ParseReturnPacket.SM.State=	UNI_MYSQL_RTRN_SM_ACT;
		break;
		/*--*/
		case UNI_MYSQL_RTRN_SM_ACT:
			if(t->Client.ParseReturnPacket.SM.FirstScan){

			}

			switch(returnPacketType){
				/*--*/
				case UNI_MYSQL_RTRN_PCKT_TYP_OK:
					/*  */
					t->Client.ParseReturnPacket.Done=		TRUE;
					t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_INIT;
				break;
				/*--*/
				case UNI_MYSQL_RTRN_PCKT_TYP_RHDR:
					/* we don't care about results header, throw it out */
					if(t->Client.CheckPacket.Done){
						t->Client.ParseReturnPacket.Done=		TRUE;
						t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_INIT;
					}else{
						t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_REC;
					}
				break;
				/*--*/
				case UNI_MYSQL_RTRN_PCKT_TYP_ERR:
					/* MySQL error returned, copy the message out */
					memset(&tempString, ZERO, sizeof(tempString));
					memcpy(&tempString, &t->Client.RecieveData.dataOut[3], (sizeof(tempString)-1));
					t->lastMySQLErrorNum=	BuildWord(t->Client.RecieveData.dataOut[2], t->Client.RecieveData.dataOut[1]);
					UniMySql_UpdateMySQLError(t,tempString);
					t->Client.ParseReturnPacket.Done=		TRUE;
					t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_INIT;
				break;
				/*--*/
				case UNI_MYSQL_RTRN_PCKT_TYP_FLD:
					/* we don't care about field data, carry on */
					if(t->Client.CheckPacket.Done){
						t->Client.ParseReturnPacket.Done=		TRUE;
						t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_INIT;
					}else{
						t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_REC;
					}
				break;
				/*--*/
				case UNI_MYSQL_RTRN_PCKT_TYP_RW:
					/* Packet is row type, copy data into 2D array */
					if(dataRow < UNI_MYSQL_MAX_NUM_RTRN_ROWS){
						memcpy(&t->Client.ParseReturnPacket.ReturnData.dataOut[dataRow][dataIndex], &t->Client.RecieveData.dataOut[dataIndex], (t->Client.RecieveData.dataOut[dataIndex]+1));
					}
					dataIndex++;
					dataIndex+= t->Client.RecieveData.dataOut[dataIndex-1];

					if(dataIndex >= t->Client.RecieveData.readSize){
						if(t->Client.CheckPacket.Done){
							t->Client.ParseReturnPacket.Done=		TRUE;
							t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_INIT;
						}else{
							t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_REC;
						}
						dataRow++;
						dataIndex= 0;
						/* record the number of records returned */
						t->Client.ParseReturnPacket.ReturnData.numRecordsReturned=	dataRow;	
					}
				break;
				/*--*/
				case UNI_MYSQL_RTRN_PCKT_TYP_EOF:
					/* Packet type is end of file, we are done with this group of packets */
					if(t->Client.CheckPacket.Done){
						t->Client.ParseReturnPacket.Done=		TRUE;
						t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_INIT;
					}else{
						t->Client.ParseReturnPacket.SM.State= 	UNI_MYSQL_RTRN_SM_REC;
					}
				break;
				/*--*/
			}


		break;
		/*--*/
	}

	return(ZERO);
}



/*--------------------------------------*/
/*			INIT MYSQL RECIEVE			*/
/*--------------------------------------*/
UINT UniMySql_RecieveDataPacket_Init(UNI_MYSQL_typ *t)
{
	t->Client.RecieveData.Done=		ZERO;
	t->Client.RecieveData.ErrorID=	ZERO;
	t->Client.RecieveData.SM.State=	UNI_MSQL_REC_DATA_SM_INIT;
	UniSM_InitFirstScan(&t->Client.RecieveData.SM);
	return(ZERO);
}


/*--------------------------------------*/
/*			MYSQL CLIENT RECIEVE		*/
/*--------------------------------------*/
UINT UniMySql_ReceiveDataPacket(UNI_MYSQL_typ *t)
{
	UINT tempLowWord, tempHighWord;
	DINT tempDINT;

	/***************************************************/	
	UniSM_CycleFirstScan(&t->Client.RecieveData.SM);
	/***************************************************/
	switch(t->Client.RecieveData.SM.State){
		/*--*/
		case UNI_MSQL_REC_DATA_SM_INIT:
			if(t->Client.RecieveData.SM.FirstScan){
				UniSM_UpdateStateDesc(&t->Client.RecieveData.SM, "UNI_MSQL_REC_DATA_SM_INIT");
			}

			t->Client.RecieveData.Done=		ZERO;
			t->Client.RecieveData.ErrorID=	ZERO;

			t->Client.RecieveData.SM.State=	UNI_MSQL_REC_DATA_SM_GET_SIZE;
		break;
		/*--*/
		case UNI_MSQL_REC_DATA_SM_GET_SIZE:
			if(t->Client.RecieveData.SM.FirstScan){
				UniSM_UpdateStateDesc(&t->Client.RecieveData.SM, "UNI_MSQL_REC_DATA_SM_GET_SIZE");
				/* We need to read the first 4 bytes to get the packet size and number */
				UniTcp_Recieve_Init(&t->TcpWrapper.Recieve);
				/* Load Fub */
				t->TcpWrapper.Recieve.Ident=			t->Ident;
				t->TcpWrapper.Recieve.Flags=			ZERO;
				t->TcpWrapper.Recieve.MaxSizeOfData=	UNI_MYSQL_PACKET_HEADER_SIZE;
				t->TcpWrapper.Recieve.pRecieveBuffer=	t->Client.RecieveData.recBuffer;
				/* Clear recieve buffer */
				memset(&t->Client.RecieveData.recBuffer, ZERO, sizeof(t->Client.RecieveData.recBuffer));
			}

			/* Cycle Fub */
			UniTcp_Recieve(&t->TcpWrapper.Recieve);

			/* Wait for done */
			if(t->TcpWrapper.Recieve.Done){
				/* Eval */
				switch(t->TcpWrapper.Recieve.ErrorID){
					/*--*/
					case ZERO:
						/* Success! */
						/* clear out header buffer */
						memset(&t->Client.RecieveData.Header, ZERO, sizeof(t->Client.RecieveData.Header));
						/* Copy in the first 4 bytes of the recieve buffer */
						memcpy(&t->Client.RecieveData.Header, t->Client.RecieveData.recBuffer, sizeof(t->Client.RecieveData.Header));
						/* Decifer the header data */
						/* first 2 bytes are the lower word of the total size */
						tempLowWord= 							BuildWord(t->Client.RecieveData.Header[1], t->Client.RecieveData.Header[0]);
						/* 3rd byte is the high word of the total size */
						tempHighWord=							(UINT)t->Client.RecieveData.Header[2];
						/* Assemble total size */
						tempDINT=								BuildDWord(tempHighWord, tempLowWord);
						t->Client.RecieveData.readSize=			tempDINT;
						/* byte 4 of the header is the packet number */	
						t->Client.RecieveData.packetNumber=		t->Client.RecieveData.Header[3];

						/* transition to recieve the rest of the packet */				
						t->Client.RecieveData.SM.State=	UNI_MSQL_REC_DATA_SM_RECIEVE;
					break;
					/*--*/
					default:
						t->Client.RecieveData.Done=		1;
						t->Client.RecieveData.ErrorID=	t->TcpWrapper.Recieve.ErrorID;
						t->Client.RecieveData.SM.State=	UNI_MSQL_REC_DATA_SM_INIT;
					break;
					/*--*/
				}
			}
			
		break;
		/*--*/
		case UNI_MSQL_REC_DATA_SM_RECIEVE:
			if(t->Client.RecieveData.SM.FirstScan){
				UniSM_UpdateStateDesc(&t->Client.RecieveData.SM, "UNI_MSQL_REC_DATA_SM_RECIEVE");
				/* Second read is to get the rest of the packet using the packet size */
				UniTcp_Recieve_Init(&t->TcpWrapper.Recieve);
				/* load fub */
				t->TcpWrapper.Recieve.Ident=				t->Ident;
				/* if the read size is less than or equal to the size of our buffer, use the read size */
				if(t->Client.RecieveData.readSize <= sizeof(t->Client.RecieveData.recBuffer)){
					t->TcpWrapper.Recieve.MaxSizeOfData=	t->Client.RecieveData.readSize;
				}else{
					t->TcpWrapper.Recieve.MaxSizeOfData=	sizeof(t->Client.RecieveData.recBuffer);
				}
				t->TcpWrapper.Recieve.pRecieveBuffer=		t->Client.RecieveData.recBuffer;
				/* clear recieve buffer */
				memset(&t->Client.RecieveData.recBuffer, ZERO, sizeof(t->Client.RecieveData.recBuffer));
			}

			/* Cycle Fub */
			UniTcp_Recieve(&t->TcpWrapper.Recieve);

			/* If done */
			if(t->TcpWrapper.Recieve.Done){
				/* Eval */
				switch(t->TcpWrapper.Recieve.ErrorID){
					/*--*/
					case ZERO:
						/*Success!*/
						/* take the remaining data and move it into a byte array */
						memset(&t->Client.RecieveData.dataOut, ZERO, sizeof(t->Client.RecieveData.dataOut));
						memcpy(&t->Client.RecieveData.dataOut, &t->Client.RecieveData.recBuffer, sizeof(t->Client.RecieveData.dataOut));
						/* Done */
						t->Client.RecieveData.Done=		1;
						t->Client.RecieveData.ErrorID=	ZERO;
						t->Client.RecieveData.SM.State=	UNI_MSQL_REC_DATA_SM_INIT;
					break;
					/*--*/
					default:
						t->Client.RecieveData.Done=		1;
						t->Client.RecieveData.ErrorID=	t->TcpWrapper.Recieve.ErrorID;
						t->Client.RecieveData.SM.State=	UNI_MSQL_REC_DATA_SM_INIT;
					break;
					/*--*/
				}
			}

		break;
		/*--*/
	}




	return(ZERO);
}







UINT UniMySql_GetReturnType_Init(UNI_MYSQL_typ *t)
{
	UniSM_InitFirstScan(&t->Client.CheckPacket.SM);
	t->Client.CheckPacket.SM.State=	UNI_MYSQL_CHK_TYP_SM_FIRST;

	t->Client.CheckPacket.Done=		ZERO;
	t->Client.CheckPacket.ErrorID=	ZERO;

	return(ZERO);
}

UINT UniMySql_GetReturnType(UNI_MYSQL_typ *t)
{
	/***************************************************/	
	UniSM_CycleFirstScan(&t->Client.CheckPacket.SM);
	/***************************************************/	
	switch(t->Client.CheckPacket.SM.State){
		/*--*/
		case UNI_MYSQL_CHK_TYP_SM_FIRST:
			if(t->Client.CheckPacket.SM.FirstScan){
				t->Client.CheckPacket.packetClass=		UniMySql_GetPacketClass(t->Client.RecieveData.dataOut[0]);
			}

			/* if the first element represents a process packet */
			if(t->Client.CheckPacket.packetClass == UNI_MYSQL_PACKET_CLS_PRCSS){
				t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_RESULTS_HDR;
				t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_RHDR;
			/* If the first element is an OK packet */	
			}else if(t->Client.CheckPacket.packetClass == UNI_MYSQL_PACKET_CLS_OK){
				t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_RESULTS_HDR;
				t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_OK;
			/* If the first element is an Error packet */
			}else if(t->Client.CheckPacket.packetClass == UNI_MYSQL_PACKET_CLS_ERR){
				t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_RESULTS_HDR;
				t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_ERR;
			}

			t->Client.CheckPacket.firstCall=	FALSE;
			t->Client.CheckPacket.SM.State=		UNI_MYSQL_CHK_TYP_SM_RTRN_PKT;
		break;
		/*--*/
		case UNI_MYSQL_CHK_TYP_SM_RTRN_PKT:
			if(t->Client.CheckPacket.SM.FirstScan){
				t->Client.CheckPacket.packetClass=		UniMySql_GetPacketClass(t->Client.RecieveData.dataOut[0]);				
			}
			if(t->Client.CheckPacket.packetClass == UNI_MYSQL_PACKET_CLS_PRCSS){
				switch(t->Client.CheckPacket.packetType){
					/*--*/
					case UNI_MYSQL_PACKET_TYP_RESULTS_HDR:
						t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_RHDR;
						t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_FIELD;
					break;
					/*--*/
					case UNI_MYSQL_PACKET_TYP_FIELD:
						if(t->Client.RecieveData.dataOut[0] == UNI_MYSQL_PACKET_VAL_EOF){
							t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_EOF;
							t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_ROW;
						}else{
							t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_FLD;
							t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_FIELD;
						}
					break;
					/*--*/
					case UNI_MYSQL_PACKET_TYP_ROW:
						if(t->Client.RecieveData.dataOut[0] == UNI_MYSQL_PACKET_VAL_EOF){
							t->Client.CheckPacket.Done=				TRUE;
							t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_EOF;
							t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_RESULTS_HDR;
							t->Client.CheckPacket.SM.State=			UNI_MYSQL_CHK_TYP_SM_FIRST;
						}else{
							t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_RW;
							t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_ROW;
						}
					break;
					/*--*/
				}
			}else if(t->Client.CheckPacket.packetClass == UNI_MYSQL_PACKET_CLS_OK){
				t->Client.CheckPacket.Done=				TRUE;
				t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_RESULTS_HDR;
				t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_OK;
				t->Client.CheckPacket.SM.State=			UNI_MYSQL_CHK_TYP_SM_FIRST;
			}else if(t->Client.CheckPacket.packetClass == UNI_MYSQL_PACKET_CLS_ERR){
				t->Client.CheckPacket.Done=				TRUE;
				t->Client.CheckPacket.packetType=		UNI_MYSQL_PACKET_TYP_RESULTS_HDR;
				t->Client.CheckPacket.returnPacketType=	UNI_MYSQL_RTRN_PCKT_TYP_ERR;
				t->Client.CheckPacket.SM.State=			UNI_MYSQL_CHK_TYP_SM_FIRST;
			}
		break;
		/*--*/
	
	}
	return(t->Client.CheckPacket.returnPacketType);
}


USINT UniMySql_GetPacketClass(USINT firstElement)
{
	switch(firstElement){
		/*--*/
		case UNI_MYSQL_PACKET_CLS_OK:
			return(UNI_MYSQL_PACKET_CLS_OK);
		break;
		/*--*/
		case UNI_MYSQL_PACKET_CLS_ERR:
			return(UNI_MYSQL_PACKET_CLS_ERR);
		break;
		/*--*/
		default:
			if((firstElement >= 1) && (firstElement <=254)){
				return(UNI_MYSQL_PACKET_CLS_PRCSS);
			}else{
				return(UNI_MYSQL_PACKET_CLS_OK);
			}
		break;
		/*--*/
	}
}



UINT UniMySql_SendCommand_Init(UNI_MYSQL_typ *t)
{
	/* Init SM */
	UniSM_InitFirstScan(&t->Client.SendCmd.SM);
	t->Client.SendCmd.SM.State=	UNI_MYSQL_SNDCMD_SM_INIT;	

	/* Init fub */
	UniMySql_SendDataPacket_Init(t);

	/* Init flags */
	t->Client.SendCmd.Done=		FALSE;
	t->Client.SendCmd.ErrorID=	ZERO;


	return(ZERO);
}


UINT UniMySql_SendCommand(UNI_MYSQL_typ *t, USINT Command, STRING *pMessage)
{
	USINT tempSendBuffer[UNI_MYSQL_SEND_BUFFER_SIZE - UNI_MYSQL_PACKET_HEADER_SIZE];
	UINT tempIndex;

	/***************************************************/	
	UniSM_CycleFirstScan(&t->Client.SendCmd.SM);
	/***************************************************/
	switch(t->Client.SendCmd.SM.State){
		/*--*/
		case UNI_MYSQL_SNDCMD_SM_INIT:
			if(t->Client.SendCmd.SM.FirstScan){

			}

			t->Client.SendCmd.SM.State=	UNI_MYSQL_SNDCMD_SM_SEND;
		break;
		/*--*/
		case UNI_MYSQL_SNDCMD_SM_SEND:
			if(t->Client.SendCmd.SM.FirstScan){
				/* Clear temp buffer */	
				memset(&tempSendBuffer, ZERO, sizeof(tempSendBuffer));
				/* Clear size */
				t->Client.SendData.TCPdatasize=	ZERO;
				/* Clear index */
				tempIndex=						ZERO;
			
				/* Copy the command in */
				tempSendBuffer[tempIndex]=	Command;
				tempIndex++;
			
				/* copy the message in */
				memcpy(&tempSendBuffer[tempIndex], pMessage, strlen(pMessage));
				tempIndex+= strlen(pMessage);
			
				/* load fub */
				memset(&t->Client.SendData.TCPdata, ZERO, sizeof(t->Client.SendData.TCPdata));
				memcpy(&t->Client.SendData.TCPdata, &tempSendBuffer, sizeof(t->Client.SendData.TCPdata));
				t->Client.SendData.TCPdatasize= tempIndex;
			}

			/* cycle fub */
			UniMySql_SendDataPacket(t, ZERO);
		
		
			if(t->Client.SendData.Done){
				t->Client.SendCmd.Done=	TRUE;
				t->Client.SendCmd.ErrorID=	t->Client.SendData.ErrorID;
			}
		break;
		/*--*/
	}

	return(ZERO);
}



/*--------------------------------------*/
/*			INIT MYSQL SEND				*/
/*--------------------------------------*/
UINT UniMySql_SendDataPacket_Init(UNI_MYSQL_typ *t)
{
	t->Client.SendData.Done=			ZERO;
	t->Client.SendData.ErrorID=			ZERO;
	t->Client.SendData.SM.State=	UNI_MYSQL_SND_DATA_SM_INIT;
	UniSM_InitFirstScan(&t->Client.SendData.SM);
	return(ZERO);
}


/*--------------------------------------*/
/*				MYSQL SEND				*/
/*--------------------------------------*/
UINT UniMySql_SendDataPacket(UNI_MYSQL_typ *t, USINT packetNumber)
{
	UINT tempHighWord, tempLowWord;
	USINT tempHighByte, tempLowByte;


	/***************************************************/	
	UniSM_CycleFirstScan(&t->Client.SendData.SM);
	/***************************************************/
	switch(t->Client.SendData.SM.State){
		/*--*/
		case UNI_MYSQL_SND_DATA_SM_INIT:
			if(t->Client.SendData.SM.FirstScan){

			}
			/* Go send */
			t->Client.SendData.SM.State=	UNI_MYSQL_SND_DATA_SM_SEND;
		break;
		/*--*/
		case UNI_MYSQL_SND_DATA_SM_SEND:
			if(t->Client.SendData.SM.FirstScan){
				/* INIT FUB */
				UniTcp_Send_Init(&t->TcpWrapper.Send);

				/* LOAD FUB */
				t->TcpWrapper.Send.Ident=				t->Ident;
				t->TcpWrapper.Send.pSendBuffer=			t->Client.SendData.sendBuffer;
				t->TcpWrapper.Send.SizeOfSendBuffer=	t->Client.SendData.TCPdatasize + UNI_MYSQL_PACKET_HEADER_SIZE;
	
				/* -PACK HEADER- */
				/* first 3 bytes is size of payload */
				tempHighWord=	HighWord(t->Client.SendData.TCPdatasize);
				tempLowWord=	LowWord(t->Client.SendData.TCPdatasize);
				
				tempLowByte=	LowByte(tempLowWord);
				tempHighByte=	HighByte(tempLowWord);

				t->Client.SendData.Header[0]=	tempLowByte;
				t->Client.SendData.Header[1]=	tempHighByte;
				
				tempLowByte=	LowByte(tempHighWord);
				tempHighByte=	HighByte(tempHighWord);	

				t->Client.SendData.Header[2]=	tempLowByte;

				/* 4th byte is the packet number */
				t->Client.SendData.Header[3]=	packetNumber;	

				/* Clear send buffer */
				memset(&t->Client.SendData.sendBuffer, ZERO, sizeof(t->Client.SendData.sendBuffer));
				/* copy header in */
				memcpy(&t->Client.SendData.sendBuffer[ZERO], &t->Client.SendData.Header, sizeof(t->Client.SendData.Header));
				/* copy in payload */
				memcpy(&t->Client.SendData.sendBuffer[4], &t->Client.SendData.TCPdata, sizeof(t->Client.SendData.TCPdata));				
			}

			/* cycle */
			UniTcp_Send(&t->TcpWrapper.Send);


			/* wait for done sending flag */
			if(t->TcpWrapper.Send.Done){
				t->Client.SendData.Done=	TRUE;
				t->Client.SendData.ErrorID=	t->TcpWrapper.Send.ErrorID;
				return(t->Client.SendData.ErrorID);
			}		
		break;
		/*--*/
	}

	return(ZERO);
}


/*--------------------------------------*/
/*		Parse Initial Server Data		*/
/*--------------------------------------*/
UINT UniMySql_ParseInitialServerData(UNI_MYSQL_typ *t)
{
	UINT i;
	UINT tempZeroSpot;
	UINT WorkingIndex, StartofScrambleBuffer2;
	UINT TempLowWord, TempHighWord;
	USINT tempScrambleBuffer1[8], tempScrambleBuffer2[12];


	/* Take initial server data that was returned by the server and parse it */
	
	/* PROTOCOL VERSION is element 0 */
	t->Server.protocolVersion=			t->Server.initialServerDataBuffer[0];
	
	WorkingIndex=	0;

	/* SERVER VERSION STRING */
	/* Search the array from index 1 and look for the next 0 */
	tempZeroSpot= 0;

	for(i=1; i<sizeof(t->Server.initialServerDataBuffer); i++){
		if(t->Server.initialServerDataBuffer[i] == 0){
			tempZeroSpot= i - 1;	/* end of our server version */
			break;	/* bail */
		}
	}

	/* Now copy out of data buffer and into string */
	memcpy(&t->Server.serverVersion, &t->Server.initialServerDataBuffer[1], tempZeroSpot);

	WorkingIndex=	tempZeroSpot+2;


	/* THREAD ID */
	TempLowWord=		BuildWord(t->Server.initialServerDataBuffer[WorkingIndex+1], t->Server.initialServerDataBuffer[WorkingIndex]);	
	TempHighWord=		BuildWord(t->Server.initialServerDataBuffer[WorkingIndex+3], t->Server.initialServerDataBuffer[WorkingIndex+2]);

	t->Server.threadID=	BuildDWord(TempHighWord, TempLowWord);

	WorkingIndex+= 4;


	/* SCRAMBLE BUFFER */
	/* copy 8 bytes in from the current index */
	memset(&tempScrambleBuffer1, ZERO, sizeof(tempScrambleBuffer1));
	memcpy(&tempScrambleBuffer1, &t->Server.initialServerDataBuffer[WorkingIndex], sizeof(tempScrambleBuffer1));

	/* Add 27 to the working index */
	StartofScrambleBuffer2= WorkingIndex + 27;

	/* Copy 12 bytes from the new index */
	memset(&tempScrambleBuffer2, ZERO, sizeof(tempScrambleBuffer2));
	memcpy(&tempScrambleBuffer2, &t->Server.initialServerDataBuffer[StartofScrambleBuffer2], sizeof(tempScrambleBuffer2));

	/* Then combine the two arrays into 1 for the scramble buffer */
	memset(&t->Server.scrambleBuffer, ZERO, sizeof(t->Server.scrambleBuffer));
	memcpy(&t->Server.scrambleBuffer[ZERO], &tempScrambleBuffer1[ZERO], sizeof(tempScrambleBuffer1));
	memcpy(&t->Server.scrambleBuffer[sizeof(tempScrambleBuffer1)], &tempScrambleBuffer2[ZERO], sizeof(tempScrambleBuffer2));


	WorkingIndex+= 8;


	WorkingIndex++;
	
	t->Server.serverCapabilities=	BuildWord(t->Server.initialServerDataBuffer[WorkingIndex+1], t->Server.initialServerDataBuffer[WorkingIndex]);

	/* SERVER LANGUAGE */
	t->Server.serverLanguage=		t->Server.initialServerDataBuffer[WorkingIndex+2];

	/* SERVER STATUS */
	t->Server.serverStatus=			BuildWord(t->Server.initialServerDataBuffer[WorkingIndex+4], t->Server.initialServerDataBuffer[WorkingIndex+3]);

		

	return(ZERO);
}


/*--------------------------------------*/
/*		BUILD INIT CLIENT DATA			*/
/*--------------------------------------*/
UINT UniMySql_BuildInitialClientData(UNI_MYSQL_typ *t)
{
	USINT tempEncryptedPassword[UNI_MYSQL_HASHED_PSWD_SIZE];
	USINT tempSendBuffer[UNI_MYSQL_SEND_BUFFER_SIZE - UNI_MYSQL_PACKET_HEADER_SIZE];
	UINT tempHighWord, tempLowWord;
	UDINT tempIndex;
	USINT i;

	memset(&tempSendBuffer, ZERO, sizeof(tempSendBuffer));


	/* Run user password through encryption function */
	UniMySql_EncryptPassword(t->Server.scrambleBuffer, t->Client.password, tempEncryptedPassword);


	tempSendBuffer[0]=	LowByte(t->Client.clientCapabilities);
	tempSendBuffer[1]=	HighByte(t->Client.clientCapabilities);
	tempSendBuffer[2]=	LowByte(t->Client.extendedCapabilities);
	tempSendBuffer[3]=	HighByte(t->Client.extendedCapabilities);
	
	tempHighWord=		HighWord(t->Client.maxPacketSize);
	tempLowWord=		LowWord(t->Client.maxPacketSize);
	
	tempSendBuffer[4]=	LowByte(tempLowWord);
	tempSendBuffer[5]=	HighByte(tempLowWord);
	tempSendBuffer[6]=	LowByte(tempHighWord);
	tempSendBuffer[7]=	HighByte(tempHighWord);

	tempSendBuffer[8]=	t->Client.charsetNumber;

	/* 23 0x00's for padding */
	for(i=9; i<33; i++){
		tempSendBuffer[i]=	0x00;
	}


	/* Pack the username */
	memcpy(&tempSendBuffer[32], t->Client.username, strlen(t->Client.username));
	/* pack a terminating zero after the username */
	tempIndex=	(32 + strlen(t->Client.username));

	tempSendBuffer[tempIndex]= 0x00;
	tempIndex++;

	/* Pack password */
	/* sizeof password */
	tempSendBuffer[tempIndex]=	sizeof(tempEncryptedPassword);
	tempIndex++;

	memcpy(&tempSendBuffer[tempIndex], &tempEncryptedPassword, sizeof(tempEncryptedPassword));
	tempIndex+= sizeof(tempEncryptedPassword);


	/* Pack database name */
	memcpy(&tempSendBuffer[tempIndex], &t->Client.database, strlen(t->Client.database));
	tempIndex+= strlen(t->Client.database);

	tempSendBuffer[tempIndex]= 0x00;

	tempIndex++;
	
	/* set data size  */
	t->Client.SendData.TCPdatasize=	tempIndex;
	
	memset(&t->Client.SendData.sendBuffer, ZERO, sizeof(t->Client.SendData.sendBuffer));
	memcpy(&t->Client.SendData.TCPdata, &tempSendBuffer, sizeof(tempSendBuffer));

	return(ZERO);
}


/*--------------------------------------*/
/*		MYSQL PASSWORD ENCRYPTION		*/
/*--------------------------------------*/
UINT UniMySql_EncryptPassword(USINT *ScrambleBuffer, STRING *Password, USINT *OutputByteArray)
{
	USINT i;

	/* initial password hash and byte array */
	unsigned char tempPasswordHashBytes[UNI_MYSQL_HASHED_PSWD_SIZE];

	/* second pass password string and byte array */
	unsigned char tempPasswordHash2Bytes[UNI_MYSQL_HASHED_PSWD_SIZE];

	/* array = scrablebuffer and then second pass of password */
	USINT tempArray[UNI_MYSQL_HASHED_PSWD_SIZE + UNI_MYSQL_SCRAMBLE_BUFF_SIZE];
	USINT tempArrayHashBytes[UNI_MYSQL_HASHED_PSWD_SIZE];



	memset(&tempPasswordHashBytes, ZERO, sizeof(tempPasswordHashBytes));
	memset(&tempPasswordHash2Bytes, ZERO, sizeof(tempPasswordHash2Bytes));
	memset(&tempArray, ZERO, sizeof(tempArray));
	memset(&tempArrayHashBytes, ZERO, sizeof(tempArrayHashBytes));


	/* Create a hash of the password */
	Create_SHA1_Hash((unsigned char*)Password, strlen(Password), tempPasswordHashBytes);

	/* Create a hash of the Password's Hash */
	Create_SHA1_Hash((unsigned char*)tempPasswordHashBytes, sizeof(tempPasswordHashBytes), tempPasswordHash2Bytes);

	/* copy the scramble buffer into the first  20 bytes of the temp array*/
	memcpy(&tempArray[0], ScrambleBuffer, UNI_MYSQL_SCRAMBLE_BUFF_SIZE);
	memcpy(&tempArray[UNI_MYSQL_SCRAMBLE_BUFF_SIZE], &tempPasswordHash2Bytes, sizeof(tempPasswordHash2Bytes));

	Create_SHA1_Hash(tempArray, sizeof(tempArray), tempArrayHashBytes);


	for(i=0; i<UNI_MYSQL_HASHED_PSWD_SIZE; i++){
		OutputByteArray[i]= tempPasswordHashBytes[i] ^ tempArrayHashBytes[i];
	}

	return(ZERO);
}



void UniMySql_UpdateMySQLError(UNI_MYSQL_typ *t, STRING *pErrorMsg)
{
	memset(&t->lastMySQLError, ZERO, sizeof(t->lastMySQLError));
	if(strlen(pErrorMsg) <= (sizeof(t->lastMySQLError)-1)){
		strcpy(t->lastMySQLError, pErrorMsg);
	}
}

