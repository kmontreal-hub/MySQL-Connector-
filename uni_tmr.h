#ifndef __UNI_TMR_H__ 
#define	__UNI_TMR_H__

#include <bur/plc.h>
#include <bur/plctypes.h>
#include "brsystem.h"



typedef struct
{
	UINT		TaskClass;
}UNI_INTERNAL_typ;


typedef struct
{
	UDINT				ET;				/* Elapsed time (Read Only) */
	UDINT				RT;				/* Remaining time (Read Only) */
	UDINT				PT;				/* Setpoint time (Read Only) */
	USINT				Pause;			/* If 1 the timer will not increment (Read Only) */
	UNI_INTERNAL_typ	Internal;
}UNI_TIMER_typ;							/* Unieric timer data type */




void UniTimer_Init(UNI_TIMER_typ	*Tmr);							/* PREPARES THE TIMER FOR USE (clears setpoint, elapsed, remaining, etc..) */
void UniTimer_Set(UNI_TIMER_typ	*Tmr, UDINT	Setpoint);				/* SETS / CHANGES THE TIMER SETPOINT (updates remaining time in case setpoint was changed) */
void UniTimer_Restart(UNI_TIMER_typ	*Tmr);							/* CLEARS ELAPSED AND REMAINING TIME WHILE PRESERVING THE SETPOINT (resets / restarts the timer) */

void UniTimer_Increment(UNI_TIMER_typ	*Timer);					/* INCREMENTS ELAPSED TIME AND UPDATES REMAINING TIME (in 1mS increments) */
void UniTimer_100uS_Increment(UNI_TIMER_typ	*Tmr);					/* INCREMENTS ELAPSED TIME AND UPDATES REMAINING TIME (in 100uS increments) */

void UniTimer_Pause(UNI_TIMER_typ	*Timer);						/* STOPS THE TIMER FROM BEING INCREMENTED ANY FURTHER */
void UniTimer_Release(UNI_TIMER_typ	*Timer);						/* ALLOWS THE TIMER TO BE INCREMENTED AGAIN */

USINT UniTimer_IsExpired(UNI_TIMER_typ	*Tmr);						/* DETERMINES AND RETURNS IF THE TIMER IS EXPIRED BY EVALUATING THE REMAINING TIME */




  
#endif
