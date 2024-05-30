#ifndef __UNI_MYSQL_PUB_H__ 
#define	__UNI_MYSQL_PUB_H__
/*-------------------------------------------------------------------------------------------------------------------------
;	File: 		uni_mysql_pub.h
;	Author:		KDM
;
;	Customer: 	 
;
;	Purpose:	Public interface to MySQL database connector
;	
;
-------------------------------------------------------------------------------------------------------------------------*/

/* --INCLUDES-- */
#include <bur/plctypes.h>
#include <asstring.h>
#include <string.h>
#include <sys_lib.h>
#include "uni_sm.h"


/* --MACROS-- */
#define TRUE	1
#define FALSE	0
#define ZERO	0

#define UNI_MYSQL_OBJ_NAME_SIZE			30				/* MySQL Object name size */
#define UNI_MYSQL_DB_USERNAME_SIZE		32				/* MySQL Max size of username string */
#define UNI_MYSQL_DB_PSWD_SIZE			32				/* MySQL Max size of password string */
#define UNI_MYSQL_DB_DB_NAME_SIZE		32				/* MySQL Max size of databse name string */
#define UNI_MYSQL_IP_STR_SIZE			21				/* Max size of IP address string */
#define UNI_MYSQL_DEVICE_NAME_SIZE		32				/* Interface name */

#define UNI_MYSQL_RECV_BUFFER_SIZE		1000			/* recieve buffer size */
#define UNI_MYSQL_SEND_BUFFER_SIZE		5000			/* Send buffer size */
#define UNI_MYSQL_MAX_NUM_RTRN_ROWS		15				/* Max number of records returned */
#define UNI_MYSQL_MAX_DATA_ROW_SIZE		1000			/* bytes */

#define UNI_MYSQL_HEARTBEAT_Q			"SELECT NULL"	/* If heartbeat is required, user SELECT NULL */

/* --ENUMS-- */
typedef enum
{
	UNI_MYSQL_ERR_OK=		0,							/* No Error */
	UNI_MYSQL_ERR_NOT_CONN=	62101,						/* Not Connected */
	UNI_MYSQL_OBJ_LOCKED=	62102,						/* Connection is locked */
	UNI_MYSQL_ERR_TIMEOUT=	62103,						/* Connection timed out */
	UNI_MYSQL_ERR_BUSY=		65535,						/* Connection busy */
}UNI_MYSQL_PUB_ERR_typ;


/* --OBJECT-- */
typedef struct UNI_MYSQL_OBJ_typ
{
	UINT			id;
	STRING			name[UNI_MYSQL_OBJ_NAME_SIZE];
	BOOL			isLocked;
	BOOL			isConfigured;
}UNI_MYSQL_OBJ_typ;


/* --CONFIGURATION-- */
typedef struct UNI_MYSQL_CONFIG_typ
{
	STRING			username[UNI_MYSQL_DB_USERNAME_SIZE];
	STRING			password[UNI_MYSQL_DB_PSWD_SIZE];
	STRING			databaseName[UNI_MYSQL_DB_DB_NAME_SIZE];

	STRING			Device[UNI_MYSQL_DEVICE_NAME_SIZE];
	STRING			clientIP[UNI_MYSQL_IP_STR_SIZE];
	STRING			serverIP[UNI_MYSQL_IP_STR_SIZE];
	UINT			port;
}UNI_MYSQL_CONFIG_typ;


/* --MYSQL RETURN DATA-- */
typedef struct UNI_MYSQL_RETURN_DATA_typ
{
	UDINT					numRecordsReturned;
	STRING					dataOut[UNI_MYSQL_MAX_NUM_RTRN_ROWS][UNI_MYSQL_MAX_DATA_ROW_SIZE];	
}UNI_MYSQL_RETURN_DATA_typ;


/* --MYSQL QUERY INTERFACE-- */
typedef struct UNI_MYSQL_PUB_QUERY_typ
{
	BOOL						execute;

	STRING						*pQueryString;
	UNI_MYSQL_RETURN_DATA_typ	*pReturnData;

	UINT						status;
}UNI_MYSQL_PUB_QUERY_typ;


/* --PUBLIC MYSQL STRUCT-- */
typedef struct UNI_MYSQL_PUB_typ
{

	UNI_MYSQL_OBJ_typ			Object;						/* MySQL connection object */

	UNI_MYSQL_CONFIG_typ		Config;						/* Connection Configuration */
	UNI_MYSQL_PUB_QUERY_typ		Query;						/* Query data */
	BOOL 						isConnected;				/* is Connected flag for MySQL server */

}UNI_MYSQL_PUB_typ;




/* Called externally to execute Query */
UINT UniMySql_ExecuteQuery(UNI_MYSQL_PUB_typ *H, STRING FunctionCaller[UNI_MYSQL_OBJ_NAME_SIZE], STRING *pQueryString, UNI_MYSQL_RETURN_DATA_typ *pReturnData);

/* Called externally to create MySQL timestamp with single colons around it */
UDINT UniMySql_CreateMySQLDateTimeStamp(STRING *pOutputString);


/* Called to release the connection after a query */
UINT UniMySql_ReleaseConnection(UNI_MYSQL_PUB_typ *H);
/* Called externally to either lock, unlock, or check object (connection) status */
BOOL UniMySql_LockObject(UNI_MYSQL_PUB_typ *H);
BOOL UniMySql_UnlockObject(UNI_MYSQL_PUB_typ *H);
BOOL UniMySql_ObjectIsLocked(UNI_MYSQL_PUB_typ *H);

/* Called to configure database connection in the main _INIT function */
BOOL UniMySql_ConfigureServerLogin(UNI_MYSQL_PUB_typ *H, STRING username[UNI_MYSQL_DB_USERNAME_SIZE], STRING password[UNI_MYSQL_DB_PSWD_SIZE], STRING databaseName[UNI_MYSQL_DB_DB_NAME_SIZE]);
BOOL UniMySql_ConfigureConnection(UNI_MYSQL_PUB_typ *H, STRING Device[UNI_MYSQL_DEVICE_NAME_SIZE], STRING clientIP[UNI_MYSQL_IP_STR_SIZE], STRING serverIP[UNI_MYSQL_IP_STR_SIZE], UINT port);





  
#endif
