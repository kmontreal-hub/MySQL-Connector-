#ifndef __UNI_SM_H__ 
#define	__UNI_SM_H__


#include <bur/plc.h>
#include <bur/plctypes.h>
#include "brsystem.h"
#include "uni_tmr.h"


#define UNI_SM_STATE_DEC_MAX	50





typedef struct UNI_SM_typ
{
	BOOL			FirstScan;
	INT				PrevState;
	USINT			State;
	STRING			StateDesc[UNI_SM_STATE_DEC_MAX];
	UNI_TIMER_typ	Tmr;
}UNI_SM_typ;



void UniSM_InitFirstScan(UNI_SM_typ *SM);
void UniSM_CycleFirstScan(UNI_SM_typ *SM);
void UniSM_UpdateStateDesc(UNI_SM_typ *SM, STRING	 Text[UNI_SM_STATE_DEC_MAX]);






  
#endif
