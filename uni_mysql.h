 #ifndef __UNI_MYSQL_H__ 
#define	__UNI_MYSQL_H__  

/*-------------------------------------------------------------------------------------------------------------------------
;	File: 		uni_mysql.h
;	Author:		KDM
;
;	Customer: 	
;
;	Purpose:	MySQL database connector
;	
;
-------------------------------------------------------------------------------------------------------------------------*/


/* INCLUDES */
#include <bur/plctypes.h>
#include "uni_mysql_pub.h"
#include "uni_tcp.h"
#include "uni_byte_functions.h"
#include "sha1.h"


/* MACROS */	
#define UNI_MYSQL_PACKET_HEADER_SIZE	4
#define UNI_MYSQL_HASHED_PSWD_SIZE		20
#define UNI_MYSQL_SCRAMBLE_BUFF_SIZE	20


/* --MySQL Packet Classes-- */
typedef enum
{
	UNI_MYSQL_PACKET_CLS_OK=	0,
	UNI_MYSQL_PACKET_CLS_PRCSS,	/* 1 - 254 */
	UNI_MYSQL_PACKET_CLS_ERR=	255,
	
}UNI_MYSQL_PACKET_CLS_typ;

/* --MySQL Packet Types-- */
typedef enum
{
	UNI_MYSQL_PACKET_TYP_RESULTS_HDR=	0,
	UNI_MYSQL_PACKET_TYP_FIELD,
	UNI_MYSQL_PACKET_TYP_ROW,
}UNI_MYSQL_PACKET_TYP_typ;

/* --MySQL Return Packet Type-- */
typedef enum
{
	UNI_MYSQL_RTRN_PCKT_TYP_OK=	0,
	UNI_MYSQL_RTRN_PCKT_TYP_RHDR,
	UNI_MYSQL_RTRN_PCKT_TYP_ERR,
	UNI_MYSQL_RTRN_PCKT_TYP_FLD,
	UNI_MYSQL_RTRN_PCKT_TYP_RW,
	UNI_MYSQL_RTRN_PCKT_TYP_EOF,
}UNI_MYSQL_RTRN_PCKT_TYP_typ;

/* --MySQL Packet First Element Values-- */
typedef enum
{
	UNI_MYSQL_PACKET_VAL_DEF=			0,	
	UNI_MYSQL_PACKET_VAL_EOF=			254,	/* 254 reserved for EOF */
}UNI_MYSQL_PACKET_VAL_typ;


/* --CLIENT SEND DATA-- */
typedef struct UNI_MYSQL_CLIENT_SND_typ
{
	UNI_SM_typ				SM;

	USINT					Done;
	UINT					ErrorID;

	UDINT					TCPdatasize;

	USINT					Header[UNI_MYSQL_PACKET_HEADER_SIZE];
	USINT					TCPdata[UNI_MYSQL_SEND_BUFFER_SIZE-UNI_MYSQL_PACKET_HEADER_SIZE];
			
	STRING					sendBuffer[UNI_MYSQL_SEND_BUFFER_SIZE];
}UNI_MYSQL_CLIENT_SND_typ;


/* --CLIENT RECIEVE DATA-- */
typedef struct UNI_MYSQL_CLIENT_REC_typ
{
	UNI_SM_typ				SM;

	USINT					Done;
	UINT					ErrorID;

	USINT					Header[UNI_MYSQL_PACKET_HEADER_SIZE];
	USINT					packetNumber;
	DINT					readSize;

	STRING					recBuffer[UNI_MYSQL_RECV_BUFFER_SIZE];
	USINT					dataOut[UNI_MYSQL_RECV_BUFFER_SIZE];
				
}UNI_MYSQL_CLIENT_REC_typ;


/* --CLIENT SEND COMMAND-- */
typedef struct UNI_MYSQL_CLIENT_SNDCMD_typ
{
	UNI_SM_typ				SM;

	USINT					Done;
	UINT					ErrorID;
	
}UNI_MYSQL_CLIENT_SNDCMD_typ;



/* --CLIENT PARSE RETURN PACKET-- */
typedef struct UNI_MYSQL_CLIENT_RTRN_PCKT_typ
{
	UNI_SM_typ					SM;

	
	UNI_MYSQL_RETURN_DATA_typ	ReturnData;

	USINT						Done;
	UINT						ErrorID;	
}UNI_MYSQL_CLIENT_RTRN_PCKT_typ;


/* --CLIENT CHECK PACKET TYPE-- */
typedef struct UNI_MYSQL_CLIENT_CHK_PKT_typ
{
	UNI_SM_typ				SM;

	BOOL					firstCall;
	USINT					packetClass;
	USINT					packetType;
	USINT					returnPacketType;


	USINT					Done;
	UINT					ErrorID;
	
}UNI_MYSQL_CLIENT_CHK_PKT_typ;


/* --CLIENT QUERY-- */
typedef struct UNI_MYSQL_CLIENT_QUERY_typ
{
	USINT					Done;
	UINT					ErrorID;
	UNI_SM_typ				SM;	

	UNI_MYSQL_RETURN_DATA_typ	ReturnData;
}UNI_MYSQL_CLIENT_QUERY_typ;


/* --MYSQL CLIENT DATA AND FUBS-- */
typedef struct UNI_MYSQL_CLIENT_typ
{
	/* Client Auth Data */
	STRING			username[UNI_MYSQL_DB_USERNAME_SIZE];	/* Database Login - Username */
	STRING			password[UNI_MYSQL_DB_PSWD_SIZE];		/* Database Login - Password */
	STRING			database[UNI_MYSQL_DB_DB_NAME_SIZE];	/* Database Login - Database Name */

	/* Data to send to server during init */
	UINT			clientCapabilities;						/* UNI_MYSQL_CLIENT_CAP 	defined in .c file */
	UINT			extendedCapabilities;					/* UNI_MYSQL_EXT_CAP 		defined in .c file */
	UDINT			maxPacketSize;							/* UNI_MYSQL_MAX_PAC_SIZE 	defined in .c file */
	USINT			charsetNumber;							/* UNI_MYSQL_CHARSET_NUM 	defined in .c file */
	USINT			passwordSize;							/*  Always 20 */

	/* Query Database */
	UNI_MYSQL_CLIENT_QUERY_typ		Query;					/* Send Query Command to Database and Parse Incoming Data*/
	/* Parse Packet */
	UNI_MYSQL_CLIENT_RTRN_PCKT_typ	ParseReturnPacket;		/* Parse MySQL Packets */
	/* Client Send Command */
	UNI_MYSQL_CLIENT_SNDCMD_typ		SendCmd;				/* Send MySQL Command */
	/* Client Recieve Data */
	UNI_MYSQL_CLIENT_REC_typ		RecieveData;			/* Recieve raw data */
	/* Cliebt Send Data */
	UNI_MYSQL_CLIENT_SND_typ		SendData;				/* Send raw data */
	/* Check Packet */
	UNI_MYSQL_CLIENT_CHK_PKT_typ	CheckPacket;			/* Check packet type and class */
}UNI_MYSQL_CLIENT_typ;


/* --MYSQL SERVER-- */
typedef struct UNI_MYSQL_SERVER_typ
{
	UINT			port;									/* port of MySQL server (default= 3306) */
	STRING			IPAddr[UNI_MYSQL_IP_STR_SIZE];			/* MySQL Server's IP */

	/* Data returned from the server */
	USINT			initialServerDataBuffer[UNI_MYSQL_RECV_BUFFER_SIZE];	/* First Packet Recieved From Server */
	USINT			packetNumber;											/* packet number */
	USINT			protocolVersion;										/* Protocol version returned by server */
	STRING			serverVersion[15];										/* Server version returned by server */
	UDINT			threadID;												/* Thread ID returned by server */
	USINT			scrambleBuffer[UNI_MYSQL_SCRAMBLE_BUFF_SIZE];			/* Scramble buffer returned by server - used to hash password */
	UINT			serverCapabilities;										/* Server capabilities returned by server */
	UINT			serverLanguage;											/* Server language returned by server */
	UINT			serverStatus;											/* Server Status returned by server */

}UNI_MYSQL_SERVER_typ;



/* --MYSQL CLIENT TYPE-- */
typedef struct UNI_MYSQL_typ
{
	UNI_SM_typ				SM;												/* SM */

	/* TCP Wrapper */
	UNI_TCP_typ				TcpWrapper;										/* AsTCP Wrapper */

	UNI_MYSQL_CLIENT_typ	Client;											/* Client data and fubs */
	UNI_MYSQL_SERVER_typ	Server;											/* Server data */

	/* Inputs */
	STRING			myIP[UNI_MYSQL_IP_STR_SIZE];							/* IP Address */
	STRING			*pDevice;												/* Device Name */
	UINT			Port;													/* port */

	UDINT			connTimeOut;											/* Connection timeout */

	

	/* Outputs */
	STRING			lastMySQLError[100];									/* If an error packet was returned from  mySQL Server, packet message stored here*/
	UINT			lastMySQLErrorNum;

	UDINT			Ident;													/* Connection Ident- returned from AsTCP */
	BOOL			isConnected;											/* Flag to represent whether we are connected to the MySQL Server or not */
	UINT			status;													/* last fub error */

}UNI_MYSQL_typ;



void UniMySql_Init(UNI_MYSQL_PUB_typ *H, UNI_MYSQL_typ *t);
void UniMySql_Operate(UNI_MYSQL_PUB_typ *H, UNI_MYSQL_typ *t);
void UniMySql_Exit(UNI_MYSQL_typ *t);


#endif
