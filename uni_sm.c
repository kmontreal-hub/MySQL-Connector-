#include <bur/plc.h>
#include <bur/plctypes.h>
#include <string.h>
#include "uni_sm.h"

/****************************************/
/* Version: 1.0							*/
/* Date: 09/29/2023						*/
/****************************************/



void UniSM_InitFirstScan(UNI_SM_typ *SM)
{
	SM->PrevState = -1;
}



void UniSM_CycleFirstScan(UNI_SM_typ *SM)
{
	
	if(SM->PrevState != SM->State){
		SM->FirstScan = 1;						/* Set first scan indicator */
	}else{
		SM->FirstScan = 0;						/* Clear first scan indicator */
	}
	
	SM->PrevState = SM->State;
}




void UniSM_UpdateStateDesc(UNI_SM_typ *SM, STRING	 Text[UNI_SM_STATE_DEC_MAX])
{
	memset(SM->StateDesc, 0, sizeof(SM->StateDesc));
	memcpy(SM->StateDesc, Text, UNI_SM_STATE_DEC_MAX);
}




void UniSM_InitFirstScan(UNI_SM_typ *SM);
void UniSM_CycleFirstScan(UNI_SM_typ *SM);
void UniSM_UpdateStateDesc(UNI_SM_typ *SM, STRING	 Text[UNI_SM_STATE_DEC_MAX]);



