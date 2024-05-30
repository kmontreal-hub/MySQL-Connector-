#ifdef _DEFAULT_INCLUDES
	#include <AsDefault.h>
#endif
#include <bur/plc.h>
#include <bur/plctypes.h>
#include <stdarg.h>
#include "uni_tmr.h"


/*
	FILE VERSION: 1.11
	FILE DATE: 09/29/2023
*/



_LOCAL  RTInfo_typ		rt_info;



void UniTimer_Init(UNI_TIMER_typ	*Tmr)
{
	/* PREPARES THE TIMER FOR USE (clears setpoint, elapsed, remaining, etc..) */
	Tmr->ET = 0;			/* Elapsed Time */
	Tmr->RT = 0;			/* Remaining Time */
	Tmr->PT = 0;			/* Setpoint */
	Tmr->Pause = 0;			/* Reset pause flag */
}



void UniTimer_Set(UNI_TIMER_typ	*Tmr, UDINT	Setpoint)
{
	/* SETS / CHANGES THE TIMER SETPOINT (updates remaining time in case setpoint was changed) */

	/* Update setpoint */
	Tmr->PT = Setpoint;								/* Setpoint Time */
	/*************************/

	/* Update remaining time */
	if(Tmr->PT >= Tmr->ET){							/* Make sure don't go negative */
		Tmr->RT = (UDINT)(Tmr->PT - Tmr->ET);		/* Decrement remaining time */
	}else{
		Tmr->RT = 0;								/* If timer has expired remaining time is zero */
	}
	/*************************/
}



void UniTimer_Restart(UNI_TIMER_typ	*Tmr)
{
	/* CLEARS ELAPSED AND REMAINING TIME WHILE PRESERVING THE SETPOINT (resets / restarts the timer) */
	Tmr->ET = 0;			/* Elapsed time */
	Tmr->RT = Tmr->PT;		/* Set remaining time equal to setpoint */
}





void UniTimer_Increment(UNI_TIMER_typ	*Tmr)
{
	/* INCREMENTS ELAPSED TIME AND UPDATES REMAINING TIME */

	/* Make sure taskclass is known */
	if(Tmr->Internal.TaskClass == 0){
		rt_info.enable = 1; 												/* Enables the function */
		RTInfo(&rt_info); 													/* Get task class info*/
		Tmr->Internal.TaskClass = rt_info.cycle_time/1000;					/* Used to provide timers that are independent of the task class time*/
		if(Tmr->Internal.TaskClass == 0)	Tmr->Internal.TaskClass = 1;	/* Would only happen if running sub 1mS - just a hack so it at least runs */
	}
	/********************************/

	/* Update elapsed time */
	if(Tmr->Pause == 0){
		if((Tmr->ET + Tmr->Internal.TaskClass) >= Tmr->ET){					/* If incrementing elapsed time won't result in rollover */
			Tmr->ET = Tmr->ET + Tmr->Internal.TaskClass;					/* Increment elapsed time */
		}
	}
	/***********************/

	/* Update remaining time */
	if(Tmr->PT >= Tmr->ET){							/* Make sure don't go negative */
		Tmr->RT = (UDINT)(Tmr->PT - Tmr->ET);		/* Decrement remaining time */
	}else{
		Tmr->RT = 0;								/* If timer has expired remaining time is zero */
	}
	/*************************/
}






void UniTimer_100uS_Increment(UNI_TIMER_typ	*Tmr)
{
	/********************************************************************************/
	/* THIS FUNCTION TREATS THE TIMER VARS AS A 100uS  TIMER 						*/
	/* SETPOINT REMAINING, ELAPSED,etc ARE ALL IN 100uS UNITS 						*/
	/* THIS TIMER IS NEEDED FOR CODE RUNNING IN TASK CLASSES BELOW 1mS (ex 400uS) 	*/ 
    /* -or- TASK CLASSES NOT IN EVEN 1Ms INCREMENTS (EX. 1.6mS) 					*/
	/* NOTE:																		*/
	/*		THIS TIME HAS A MAX LIFESPAN OF JUST UNDER 5 DAYS 						*/
	/********************************************************************************/
	
	/* INCREMENTS ELAPSED TIME AND UPDATES REMAINING TIME */

	/* Make sure taskclass is known */
	if(Tmr->Internal.TaskClass == 0){
		rt_info.enable = 1; 												/* Enables the function */
		RTInfo(&rt_info); 													/* Get task class info*/
		Tmr->Internal.TaskClass = rt_info.cycle_time/100;					/* Used to provide timers that are independent of the task class time*/
		if(Tmr->Internal.TaskClass == 0)	Tmr->Internal.TaskClass = 1;	/* Shouldn't happen, but hack so it at least runs */
	}
	/********************************/

	/* Update elapsed time */
	if(Tmr->Pause == 0){
		if((Tmr->ET + Tmr->Internal.TaskClass) >= Tmr->ET){					/* If incrementing elapsed time won't result in rollover */
			Tmr->ET = Tmr->ET + Tmr->Internal.TaskClass;		/* Increment elapsed time */
		}
	}
	/***********************/

	/* Update remaining time */
	if(Tmr->PT >= Tmr->ET){							/* Make sure don't go negative */
		Tmr->RT = (UDINT)(Tmr->PT - Tmr->ET);		/* Decrement remaining time */
	}else{
		Tmr->RT = 0;								/* If timer has expired remaining time is zero */
	}
	/*************************/
}







void UniTimer_Pause(UNI_TIMER_typ	*Tmr)
{
	/* STOPS THE TIMER FROM BEING INCREMENTED ANY FURTHER */
	Tmr->Pause = 1;
}


void UniTimer_Release(UNI_TIMER_typ	*Tmr)
{
	/* ALLOWS THE TIMER TO BE INCREMENTED AGAIN */
	Tmr->Pause = 0;
}




USINT UniTimer_IsExpired(UNI_TIMER_typ	*Tmr)
{
	/* DETERMINES AND RETURNS IF THE TIMER IS EXPIRED BY EVALUATING THE REMAINING TIME */

	if(Tmr->RT <= 0){
		return 1;			/* Timer has expired */
	}else{
		return 0;			/* Timer has NOT expired */
	}
}

