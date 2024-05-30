/*-------------------------------------------------------------------------------------------------------------------------
;	File: 		uni_mysql_pub.c
;	Author:		KDM
;
;	Customer: 	
;
;	Purpose:	Public interface to MySQL database connector
;	
;
-------------------------------------------------------------------------------------------------------------------------*/

/* --INCLUDES-- */
#include "uni_mysql_pub.h"


/* --EXECUTE QUERY-- */
UINT UniMySql_ExecuteQuery(UNI_MYSQL_PUB_typ *H, STRING FunctionCaller[UNI_MYSQL_OBJ_NAME_SIZE], STRING *pQueryString, UNI_MYSQL_RETURN_DATA_typ *pReturnData)
{
	if(H->isConnected == FALSE){
		return(UNI_MYSQL_ERR_NOT_CONN);
	}

	/* If object is NOT locked */
	if(UniMySql_ObjectIsLocked(H) == FALSE){
		/* Lock the object */
		UniMySql_LockObject(H);
		/* Copy in name of function caller */
		strcpy(H->Object.name, FunctionCaller);
		/* load parameters */
		H->Query.execute=		TRUE;
		H->Query.pQueryString=	pQueryString;
		H->Query.pReturnData=	pReturnData;
		return(UNI_MYSQL_ERR_OK);	
	}else{
		return(UNI_MYSQL_OBJ_LOCKED);
	}	
}


/* --RELEASE DB CONNECTION-- */
UINT UniMySql_ReleaseConnection(UNI_MYSQL_PUB_typ *H)
{
	/* Clear Function Caller Name and ID */
	strcpy(H->Object.name, "");
	/* unlock connection */
	UniMySql_UnlockObject(H);
	return(TRUE);
}


/* --CREATE MYSQL DATE TIME STAMP-- */
UDINT UniMySql_CreateMySQLDateTimeStamp(STRING *pOutputString)
{
	STRING tempString[10];
	RTCtime_typ	RTC;

	/* Clear output string */
	memset(pOutputString, ZERO, sizeof(*pOutputString));
	/* Clear temp string */
	memset(&tempString, ZERO, sizeof(tempString));

	/* Get time */
	RTC_gettime(&RTC);

	/* copy in single quote */
	strcpy(pOutputString, "'");
	/* copy in year */
	itoa(RTC.year, (UDINT)tempString);
	strcat(pOutputString, tempString);
	/* add dash */
	strcat(pOutputString, "-");
	/* copy in month */
	itoa(RTC.month, (UDINT)tempString);
	strcat(pOutputString, tempString);
	/* add dash */
	strcat(pOutputString, "-");
	/* copy in day */
	itoa(RTC.day, (UDINT)tempString);
	strcat(pOutputString, tempString);	

	/* add space */
	strcat(pOutputString, " ");

	/* copy in hour */
	itoa(RTC.hour, (UDINT)tempString);
	strcat(pOutputString, tempString);	
	/* add colon */
	strcat(pOutputString, ":");
	/* copy in minutes */
	itoa(RTC.minute, (UDINT)tempString);
	strcat(pOutputString, tempString);		
	/* add colon */
	strcat(pOutputString, ":");
	/* copy in seconds */
	itoa(RTC.second, (UDINT)tempString);
	strcat(pOutputString, tempString);	
	/* copy in single quote */
	strcat(pOutputString, "'");


	return((UDINT)pOutputString);
}


/* --CONFIGURE DATABASE LOGIN-- */
BOOL UniMySql_ConfigureServerLogin(UNI_MYSQL_PUB_typ *H, STRING username[UNI_MYSQL_DB_USERNAME_SIZE], STRING password[UNI_MYSQL_DB_PSWD_SIZE], STRING databaseName[UNI_MYSQL_DB_DB_NAME_SIZE])
{
	/* clear out variables */
	memset(&H->Config.username, ZERO, sizeof(H->Config.username));
	memset(&H->Config.password, ZERO, sizeof(H->Config.password));
	memset(&H->Config.databaseName, ZERO, sizeof(H->Config.databaseName));	

	/* copy in function parameters */
	strcpy(H->Config.username, username);
	strcpy(H->Config.password, password);
	strcpy(H->Config.databaseName, databaseName);

	return(TRUE);
}


/* --CONFIGURE CONNECTION INTERFACE-- */
BOOL UniMySql_ConfigureConnection(UNI_MYSQL_PUB_typ *H, STRING Device[UNI_MYSQL_DEVICE_NAME_SIZE], STRING clientIP[UNI_MYSQL_IP_STR_SIZE], STRING serverIP[UNI_MYSQL_IP_STR_SIZE], UINT port)
{
	/* Clear out variables */
	memset(&H->Config.Device, ZERO, sizeof(H->Config.Device));
	memset(&H->Config.clientIP, ZERO, sizeof(H->Config.clientIP));
	memset(&H->Config.serverIP, ZERO, sizeof(H->Config.serverIP));

	/* Copy in function parameters */
	strcpy(H->Config.Device, Device);
	strcpy(H->Config.clientIP, clientIP);
	strcpy(H->Config.serverIP, serverIP);
	H->Config.port=	port;

	/* Connection configured */
	H->Object.isConfigured=	TRUE;

	return(TRUE);
}


/*-------------------------------------------------------------------------------------------*/


/* --LOCK OBJECT (CONNECTION)-- */
BOOL UniMySql_LockObject(UNI_MYSQL_PUB_typ *H)
{
	H->Object.isLocked= TRUE;
	return(TRUE);
}

/* --UNLOCK OBJECT (CONNECTION)-- */
BOOL UniMySql_UnlockObject(UNI_MYSQL_PUB_typ *H)
{
	H->Object.isLocked=	FALSE;
	return(FALSE);
}

/* --DETERMINE IF OBJECT IS LOCKED-- */
BOOL UniMySql_ObjectIsLocked(UNI_MYSQL_PUB_typ *H)
{
	if(H->Object.isLocked){
		return(TRUE);
	}else{
		return(FALSE);
	}
}


