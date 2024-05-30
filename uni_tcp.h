#ifndef __UNI_TCP_H__ 
#define	__UNI_TCP_H__



#include <bur/plctypes.h>
#include <string.h>
#include <AsARCfg.h>
#include <AsTcp.h>


#define TCP_STATE_DESC_TEXT_SIZE	40


typedef enum
{
	UNI_TCP_ERR_NO_ERR=	0,
	UNI_TCP_ERR_MISSING_PTR,
}UNI_TCP_ERR_typ;



///////////////////////////////////////////////////////
//	TCP SM
///////////////////////////////////////////////////////
typedef struct UNI_TCP_SM_typ
{
	USINT					State;									/* State var */
	INT						PrevState;								/* Can be used inside a state to eval the last state executed*/
	USINT					FirstScan;								/* Can be used inside a state to handle the first scan of a state */
	STRING					StateDesc[TCP_STATE_DESC_TEXT_SIZE];	/* Text string - can be used for debugging*/
}UNI_TCP_SM_typ;


typedef struct UNI_TCP_INTERNAL_typ
{
	UNI_TCP_SM_typ			SM;	
}UNI_TCP_INTERNAL_typ;



typedef struct UNI_TCP_GET_IP_typ
{
	USINT					Done;
	USINT					IPAddressLength;
	UINT					ErrorID;
	STRING					*pIFName;
	STRING					*pIPAddress;
	CfgGetIPAddr_typ		Fub;
	UNI_TCP_INTERNAL_typ	Internal;
}UNI_TCP_GET_IP_typ;


typedef struct UNI_TCP_OPEN_typ
{
	USINT					Done;
	UINT					ErrorID;
	UINT					Port;
	UDINT					Ident;
	STRING					*pIPAddress;
	TcpOpen_typ				Fub;
	UNI_TCP_INTERNAL_typ	Internal;
}UNI_TCP_OPEN_typ;


typedef struct UNI_TCP_IOCTRL_typ
{
	USINT					Done;
	UINT					ErrorID;
	UDINT					IOControlCode;
	UDINT					Ident;
	tcpLINGER_typ			LingerOptions;
	TcpIoctl_typ			Fub;
	UNI_TCP_INTERNAL_typ	Internal;
}UNI_TCP_IOCTL_typ;


typedef struct UNI_TCP_SERV_typ
{
	USINT					Done;
	UINT					ErrorID;
	UDINT					Ident;
	UDINT					ClientIdent;
	STRING					*pClientIPAddress;
	TcpServer_typ			Fub;
	UNI_TCP_INTERNAL_typ	Internal;
}UNI_TCP_SERV_typ;


typedef struct UNI_TCP_CLNT_typ
{
	USINT					Done;
	UINT					ErrorID;
	UINT					Port;
	UDINT					Ident;
	STRING					*pServerIP;
	TcpClient_typ			Fub;
}UNI_TCP_CLNT_typ;


typedef struct UNI_TCP_SEND_typ
{
	USINT					Done;
	USINT					Busy;
	UINT					ErrorID;
	UDINT					Flags;
	UDINT					Ident;
	UDINT					SizeOfSendBuffer;
	STRING					*pSendBuffer;
	TcpSend_typ				Fub;
}UNI_TCP_SEND_typ;


typedef struct UNI_TCP_RECIEVE_typ
{
	USINT					Done;
	USINT					Busy;
	UINT					ErrorID;
	UDINT					Ident;
	UDINT 					Flags;
	UDINT					MaxSizeOfData;
	STRING					*pRecieveBuffer;
	TcpRecv_typ				Fub;
}UNI_TCP_RECIEVE_typ;


typedef struct UNI_TCP_CLOSE_typ
{
	USINT					Done;
	UINT					ErrorID;
	UDINT					Ident;
	UDINT					ShutdownBehavior;
	TcpClose_typ			Fub;
}UNI_TCP_CLOSE_typ;



typedef struct UNI_TCP_typ
{
	UNI_TCP_GET_IP_typ		GetIp;
	UNI_TCP_OPEN_typ		Open;
	UNI_TCP_IOCTL_typ		IOCtl;
	UNI_TCP_SERV_typ		Server;
	UNI_TCP_CLNT_typ		Client;
	UNI_TCP_SEND_typ		Send;
	UNI_TCP_RECIEVE_typ		Recieve;
	UNI_TCP_CLOSE_typ		Close;
}UNI_TCP_typ;




UINT UniTcp_GetDeviceIF_IP_Init(UNI_TCP_GET_IP_typ *G);
UINT UniTcp_GetDeviceIF_IP(UNI_TCP_GET_IP_typ *G);


UINT UniTcp_Open_Init(UNI_TCP_OPEN_typ *O);
UINT UniTcp_Open(UNI_TCP_OPEN_typ *O);


UINT UniTcp_IOCtl_Init(UNI_TCP_IOCTL_typ *I);
UINT UniTcp_IOCtl(UNI_TCP_IOCTL_typ *I);


UINT UniTcp_Server_Init(UNI_TCP_SERV_typ *S);
UINT UniTcp_Server(UNI_TCP_SERV_typ *S);


UINT UniTcp_Client_Init(UNI_TCP_CLNT_typ *C);
UINT UniTcp_Client(UNI_TCP_CLNT_typ *C);


UINT UniTcp_Send_Init(UNI_TCP_SEND_typ *S);
UINT UniTcp_Send(UNI_TCP_SEND_typ *S);


UINT UniTcp_Recieve_Init(UNI_TCP_RECIEVE_typ *R);
UINT UniTcp_Recieve(UNI_TCP_RECIEVE_typ *R);


UINT UniTcp_Close_Init(UNI_TCP_CLOSE_typ *C);
UINT UniTcp_Close(UNI_TCP_CLOSE_typ *C);






  
#endif
