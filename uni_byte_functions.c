/*  cls_byte_functions.c  */
/*
; 	File:			byte_functions 
; 	Description:	-
; 					
;	Author:			KDM

;
; 	Project:		
; 	Date:			12/20/2023
; 	Modified:
; 	Version:		1.1
;   Major:          1
;   Minor:          1

		 
;****************************************************************************
;Notes:	 
;	Rev 1.0 See header file for changes.
;****************************************************************************
; 
;****************************************************************************
;Dependencies: 
;****************************************************************************
*/


/**************** File Includes ******************************/
#include	"uni_byte_functions.h"

/**************** Local #Defines *****************************/

/**************** Local Function Prototyes ********************/

/**************** External Function implementation *******************************/
/*********************************************************************************/
USINT HighByte(UINT word){
	USINT temp[2];
#ifdef _i386_	
	/*Intel*/
	memcpy(temp, &word, sizeof(UINT));
	return(temp[1]);
#else
	/*Motorola*/	
	memcpy(temp, &word, sizeof(UINT));
	return(temp[0]);
#endif
}

/*-------------------------------------------------------------------------------*/
USINT LowByte(UINT word){
	USINT temp[2];
#ifdef _i386_	
	/*Intel*/
	memcpy(temp, &word, sizeof(UINT));
	return(temp[0]);
#else
	/*Motorola*/	
	memcpy(temp, &word, sizeof(UINT));
	return(temp[1]);
#endif
}

/*-------------------------------------------------------------------------------*/
UINT HighWord(UDINT dword){
	UINT temp[2];	
	/*Intel*/
	memcpy(temp, &dword, sizeof(UDINT));
	return(temp[1]);
}

/*-------------------------------------------------------------------------------*/
UINT LowWord(UDINT dword){
	UINT temp[2];	
	/*Intel*/
	memcpy(temp, &dword, sizeof(UDINT));
	return(temp[0]);
}
/*-------------------------------------------------------------------------------*/
UINT  BuildWord(USINT highByte, USINT lowByte){
	UINT temp;
#ifdef _i386_	
	/*Intel*/
	temp= highByte;
	temp= temp << 8;
	temp= temp | lowByte;
	return(temp);
#else
	/*Motorola*/	
	temp= highByte;
	temp= temp << 8;
	temp= temp | lowByte;
	return(temp);
#endif

}

/*-------------------------------------------------------------------------------*/
UDINT  BuildDWord(UINT highWord, UINT lowWord){
	UDINT temp;	
	/*Intel*/
	temp= highWord;
	temp= temp << 16;
	temp= temp | lowWord;
	return(temp);
}

/*-------------------------------------------------------------------------------*/
BOOL TestBit(USINT* src, USINT bit){
	return((BOOL)((*src >> (bit-1)) & 0x01)); 
}

/*-------------------------------------------------------------------------------*/
void PackBit(USINT* addr, USINT bit, BOOL val){
	if(val == 1){
		SetBit(addr, bit);
	}
	else if (val == 0){
		ResetBit(addr, bit);
	}
}

/*-------------------------------------------------------------------------------*/
void SetBit(USINT* addr, USINT bit){
	*addr |= (0x01 << (bit-1)); 
}
	
/*-------------------------------------------------------------------------------*/
void ResetBit(USINT* addr, USINT bit){
	*addr &= ~(0x01 << (bit-1));	
}
