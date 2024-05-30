/*  uni_byte_functions.h  */
/*
; 	File:			byte_functions.h
; 	Description:	Byte Swapping Utilities functions 
; 					
;	Author:			KDM

;
; 	Project:		
; 	Date:			12/21/2023
; 	Modified:
; 	Version:		1.1
;   Major:          1
;   Minor:          1

		 
;****************************************************************************
;Notes: Rev 1.0 Split off from cls_wn_mn class.
;		Rev 1.1 Added #if for Intel / Motorola switching.
;****************************************************************************
;
;****************************************************************************
;Dependencies: 
;****************************************************************************
;
*/
#ifndef __UNI_BYTE_FUNCTIONS_H__
#define __UNI_BYTE_FUNCTIONS_H__

/*--- #define the processor type ---*/
#define _i386_	/*Intel, comment out for Motorola*/


/**************** File Includes ********************************************/
#include	<bur/plctypes.h>
#include	<string.h>

/***************************************************************************/



/**************** Function Prototyes *************************/
USINT HighByte(UINT word);
USINT LowByte(UINT word);
UINT HighWord(UDINT dword);
UINT LowWord(UDINT dword);
UINT  BuildWord(USINT highByte, USINT lowByte);
UDINT  BuildDWord(UINT highWord, UINT lowWord);

BOOL TestBit(USINT* addr, USINT bit);
void PackBit(USINT* addr, USINT bit, BOOL val);
void SetBit(USINT* addr, USINT bit);
void ResetBit(USINT* addr, USINT bit);

#endif
