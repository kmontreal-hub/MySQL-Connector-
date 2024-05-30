

#include "uni_tcp.h"





void UniTcp_SM_InitFirstScan(UNI_TCP_SM_typ *SM);
void UniTcp_SM_CycleFirstScan(UNI_TCP_SM_typ *SM);
void UniTcp_SM_UpdateStateDesc(UNI_TCP_SM_typ *SM, STRING	 Text[TCP_STATE_DESC_TEXT_SIZE]);



UINT UniTcp_GetDeviceIF_IP_Init(UNI_TCP_GET_IP_typ *G)
{
	/* Clear Struct */
	G->Done=		0;
	G->ErrorID=		ERR_OK;
	G->pIFName=		0;
	G->pIPAddress=	0;

	/* Init Fub */
	G->Fub.enable=	0;
	CfgGetIPAddr(&G->Fub);

	return(ERR_OK);
}

UINT UniTcp_GetDeviceIF_IP(UNI_TCP_GET_IP_typ *G)
{
	/* Pointer Checks */
	if((G->pIFName == 0) || (G->pIPAddress == 0)){
		G->Done=	1;
		G->ErrorID=	UNI_TCP_ERR_MISSING_PTR;
		return(UNI_TCP_ERR_MISSING_PTR);
	}


	/* Pass data into fub */
	G->Fub.enable=	1;
	G->Fub.pDevice=	(UDINT)G->pIFName;
	G->Fub.pIPAddr=	(UDINT)G->pIPAddress;
	G->Fub.Len=		G->IPAddressLength;

	/* Cycle Fub */
	CfgGetIPAddr(&G->Fub);

	
	/* eval Fub */
	switch(G->Fub.status){
		/*--*/
		case ERR_OK:
			/* IP Successfully Obtained */
			G->Done=	1;
			G->ErrorID=	ERR_OK;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* wait for completion */
		break;
		/*--*/
		default:
			/* return error */
			G->Done=	1;
			G->ErrorID=	G->Fub.status;
			return(G->ErrorID);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_Open_Init(UNI_TCP_OPEN_typ *O)
{		
	/* Clear Struct */
	O->Done=		0;
	O->ErrorID=		UNI_TCP_ERR_NO_ERR;
	O->pIPAddress=	0;
	O->Port=		0;

	/* Init fub */
	O->Fub.enable=	0;
	TcpOpen(&O->Fub);

	return(ERR_OK);
}


UINT UniTcp_Open(UNI_TCP_OPEN_typ *O)
{
	/* Pass data into Fub */
	O->Fub.enable=		1;
	O->Fub.options=		tcpOPT_REUSEADDR;
	O->Fub.pIfAddr=		(UDINT)O->pIPAddress;
	O->Fub.port=		O->Port;


	/* Cycle the fub */
	TcpOpen(&O->Fub);

	/* Eval Fub */
	switch(O->Fub.status){
		/*--*/
		case ERR_OK:
			/* TCP Socket opened successfully */
			O->Ident=		O->Fub.ident;
			O->ErrorID=		O->Fub.status;
			O->Done=		1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
		break;
		/*--*/
		default:
			/* Fail */
			O->Ident=	0;
			O->ErrorID=	O->Fub.status;
			O->Done=	1;
			return(O->ErrorID);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_IOCtl_Init(UNI_TCP_IOCTL_typ *I)
{
	/* Clear Struct */
	I->Done=	0;
	I->ErrorID=	ERR_OK;
	I->Ident=	0;

	/* Init Fub */
	I->Fub.enable=	0;
	TcpIoctl(&I->Fub);


	return(ERR_OK);
}

UINT UniTcp_IOCtl(UNI_TCP_IOCTL_typ *I)
{

	/* Pass data into fub */
	I->Fub.enable=	1;
	I->Fub.ident=	I->Ident;
	I->Fub.ioctl=	I->IOControlCode;
	I->Fub.pData=	(UDINT)&I->LingerOptions;
	I->Fub.datalen=	sizeof(I->LingerOptions);


	/* Cycle Fub */
	TcpIoctl(&I->Fub);


	/* Eval Fub */
	switch(I->Fub.status){
		/*--*/
		case ERR_OK:
			/* Successful */
			I->ErrorID=	ERR_OK;
			I->Done=	1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
		break;
		/*--*/
		default:
			/* Fail */
			I->ErrorID=	I->Fub.status;
			I->Done=	1;
			return(I->ErrorID);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_Server_Init(UNI_TCP_SERV_typ *S)
{
	/* Clear struct */
	S->Done=				0;
	S->ErrorID=				ERR_OK;
	S->Ident=				0;
	S->pClientIPAddress=	0;

	return(ERR_OK);
}

UINT UniTcp_Server(UNI_TCP_SERV_typ *S)
{
	/* Pointer Checks */
	if(S->pClientIPAddress == 0){
		S->Done=	1;
		S->ErrorID=	UNI_TCP_ERR_MISSING_PTR;
		return(UNI_TCP_ERR_MISSING_PTR);
	}

	/* pass data into fub */
	S->Fub.enable=	1;
	S->Fub.ident=	S->Ident;
	S->Fub.backlog=	1;
	S->Fub.pIpAddr=	(UDINT)S->pClientIPAddress;

	/* Cycle Fub */	
	TcpServer(&S->Fub);

	
	/* Eval Fub */
	switch(S->Fub.status){
		/*--*/
		case ERR_OK:
			/* Success */
			S->ClientIdent=	S->Fub.identclnt;
			S->ErrorID=		ERR_OK;
			S->Done=		1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
		break;
		/*--*/
		default:
			/* Failure */
			S->ErrorID=	S->Fub.status;
			S->Done=	1;
			return(S->Fub.status);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_Client_Init(UNI_TCP_CLNT_typ *C)
{
	/* Clear struct */
	C->Done=		0;
	C->ErrorID=		ERR_OK;
	C->Ident=		0;
	C->Port=		0;
	C->pServerIP=	0;


	/* init fub */
	C->Fub.enable=	0;
	TcpClient(&C->Fub);

	
	return(ERR_OK);
}


UINT UniTcp_Client(UNI_TCP_CLNT_typ *C)
{
	/* Pointer Check */
	if(C->pServerIP == 0){
		C->ErrorID=		UNI_TCP_ERR_MISSING_PTR;
		C->Done=		1;
		return(UNI_TCP_ERR_MISSING_PTR);
	}


	/* Pass data into fub */
	C->Fub.enable=		1;
	C->Fub.ident=		C->Ident;
	C->Fub.pServer=		(UDINT)C->pServerIP;	/* V1.12 */
	C->Fub.portserv=	C->Port;


	/* Cycle Fub */
	TcpClient(&C->Fub);


	/* Eval Fub */
	switch(C->Fub.status){
		/*--*/
		case ERR_OK:
			/* Successful */
			C->ErrorID=		ERR_OK;
			C->Done=		1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
		break;
		/*--*/
		default:
			/* Failure */
			C->ErrorID=		C->Fub.status;
			C->Done=		1;
			return(C->Fub.status);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_Send_Init(UNI_TCP_SEND_typ *S)
{
	/* Clear Struct */
	S->Done= 				0;
	S->Busy=				0;
	S->ErrorID=				ERR_OK;
	S->Flags=				0;
	S->Ident=				0;
	S->pSendBuffer=			0;
	S->SizeOfSendBuffer=	0;

	
	/* Init Fub */
	S->Fub.enable=	0;
	TcpSend(&S->Fub);

	return(ERR_OK);
}


UINT UniTcp_Send(UNI_TCP_SEND_typ *S)
{
	/* Pointer Check */
	if(S->pSendBuffer == 0){
		S->ErrorID=		UNI_TCP_ERR_MISSING_PTR;
		S->Done=		1;
		return(UNI_TCP_ERR_MISSING_PTR);		
	}

	
	/* Pass data into fub */
	S->Fub.enable=	1;
	S->Fub.flags=	S->Flags;
	S->Fub.ident=	S->Ident;
	S->Fub.pData=	(UDINT)S->pSendBuffer;
	S->Fub.datalen=	S->SizeOfSendBuffer;


	/* Cycle Fub */
	TcpSend(&S->Fub);


	/* Eval Fub */
	switch(S->Fub.status){
		/*--*/
		case ERR_OK:
			S->ErrorID=	ERR_OK;
			S->Busy=	0;
			S->Done=	1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
			S->Busy=	1;
		break;
		/*--*/
		default:
			/* Failure */
			S->ErrorID=	S->Fub.status;
			S->Busy=	0;
			S->Done=	1;
			return(S->ErrorID);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_Recieve_Init(UNI_TCP_RECIEVE_typ *R)
{
	/* Clear struct */
	R->Done=			0;
	R->ErrorID=			ERR_OK;
	R->Busy=			0;
	R->Flags=			0;
	R->Ident=			0;
	R->pRecieveBuffer=	0;
	R->MaxSizeOfData=	0;

	/* Init Fub */
	R->Fub.enable=	0;
	TcpRecv(&R->Fub);
	
	return(ERR_OK);
}


UINT UniTcp_Recieve(UNI_TCP_RECIEVE_typ *R)
{
	/* Pointer Check */
	if(R->pRecieveBuffer == 0){
		R->ErrorID=	UNI_TCP_ERR_MISSING_PTR;
		R->Done=	1;
		return(ERR_OK);
	}


	/* Pass data into fub */
	R->Fub.enable=	1;
	R->Fub.ident=	R->Ident;
	R->Fub.flags=	R->Flags;
	R->Fub.pData=	(UDINT)R->pRecieveBuffer;
	R->Fub.datamax=	R->MaxSizeOfData;


	/* Cycle Fub */
	TcpRecv(&R->Fub);


	/* Eval fub */
	switch(R->Fub.status){
		/*--*/
		case ERR_OK:
			R->ErrorID=	ERR_OK;
			R->Done=	1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
			R->Busy=	1;
		break;
		/*--*/
		default:
			R->ErrorID=	R->Fub.status;
			R->Busy=	0;
			R->Done=	1;
			return(R->ErrorID);
		break;
		/*--*/
	}

	return(ERR_OK);
}


UINT UniTcp_Close_Init(UNI_TCP_CLOSE_typ *C)
{
	/* Clear Struct */
	C->Done=				0;
	C->ErrorID=				ERR_OK;
	C->Ident=				0;
	C->ShutdownBehavior=	0;

	/* Init Fub */
	C->Fub.enable=	0;
	TcpClose(&C->Fub);

	return(ERR_OK);
}

UINT UniTcp_Close(UNI_TCP_CLOSE_typ *C)
{
	/* Pass data into fub */
	C->Fub.enable=	1;
	C->Fub.ident=	C->Ident;
	C->Fub.how=		C->ShutdownBehavior;


	/* Cycle Fub */
	TcpClose(&C->Fub);

	/* Eval Fub */
	switch(C->Fub.status){
		/*--*/
		case ERR_OK:
			C->ErrorID=	ERR_OK;
			C->Done=	1;
			return(ERR_OK);
		break;
		/*--*/
		case ERR_FUB_BUSY:
			/* Wait for completion */
		break;
		/*--*/
		default:
			C->ErrorID=	C->Fub.status;
			C->Done=	1;
			return(C->ErrorID);
		break;
		/*--*/
	}

	return(ERR_OK);
}



void UniTcp_SM_InitFirstScan(UNI_TCP_SM_typ *SM)
{
	SM->PrevState = -1;
}


void UniTcp_SM_CycleFirstScan(UNI_TCP_SM_typ *SM)
{

	if(SM->PrevState != SM->State){
		SM->FirstScan = 1;						/* Set first scan indicator */
	}else{
		SM->FirstScan = 0;						/* Clear first scan indicator */
	}

	SM->PrevState = SM->State;
}


void UniTcp_SM_UpdateStateDesc(UNI_TCP_SM_typ *SM, STRING	 Text[TCP_STATE_DESC_TEXT_SIZE])
{
	memset(SM->StateDesc, 0, sizeof(SM->StateDesc));
	memcpy(SM->StateDesc, Text, sizeof(SM->StateDesc));
}
