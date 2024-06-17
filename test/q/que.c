/*********************************************************************
 *   Name          : QUE.C
 *   Description   : Module Consists of the Que Functions
 *   Version       : 1.00c
 *   Date          : 06/09/99
 *   Author        : LEO
 *   Reference     : X
 *********************************************************************/
/*-------------------------------------------------------------------
 - INCLUDE FILES
 -------------------------------------------------------------------*/
#include <avr/interrupt.h>

/*-------------------------------------------------------------------
 - INCLUDE GLOBALS
 -------------------------------------------------------------------*/
#define QUE_GLOBAL
#include "que.h"

/*-------------------------------------------------------------------
 - DEFINITIONS
 -------------------------------------------------------------------*/

#define  ENTER_CRITICAL()  (cpu_sr = CPUSaveSR())    /* Disable interrupts                        */
#define  EXIT_CRITICAL()   (CPURestoreSR(cpu_sr))    /* Enable  interrupts                        */

/*-------------------------------------------------------------------
 - CONTSTANTS
 -------------------------------------------------------------------*/
typedef unsigned char CPU_SR;
/*-------------------------------------------------------------------
 - LOCAL DEFINITIONS
 -------------------------------------------------------------------*/

/*-------------------------------------------------------------------
 - LOCAL PROTOTYPES
 -------------------------------------------------------------------*/
CPU_SR CPUSaveSR(void);
void CPURestoreSR(CPU_SR cpu_sr);

/********************************************************************
 * Function calls
 *********************************************************************
 * void que_clear(struct QUE *, unsigned char);
 * unsigned char que_status(struct QUE *);
 * void que_put(struct QUE *, void *, unsigned char);
 * unsigned int que_get(struct QUE *, unsigned char);
 * unsigned int que_look(struct QUE *, unsigned char);
 * void que_remove(struct QUE *que);
 * unsigned char ique_status(struct QUE *);
 * void ique_put(struct QUE *, unsigned char);
 *********************************************************************/

/*********************************************************************
 * Setup que pointers at Reset
 *********************************************************************/
void Que_Setup(struct QUE * const que, unsigned char *pdata, const unsigned char max)
{
	que->data = pdata;
	que->max = max;
	Que_Clear(que);
	return;
}

/*********************************************************************
 * Clear the que_data and reset the que pointers to the beginning
 *********************************************************************/
void Que_Clear(struct QUE * const que)
{
	unsigned char i, max_in_que;
	CPU_SR cpu_sr;

	ENTER_CRITICAL();
	max_in_que = que->max;
	for (i = 0; i < max_in_que; i++)
	{
		*(que->data + i) = Q_EMPTY;
	}
	que->quein = 0;
	que->queout = 0;
	EXIT_CRITICAL();
	return;
}

/*********************************************************************
 * Return the status ie Q_FULL, Q_EMPTY, Q_OK
 *********************************************************************/
Queue_Status_Type Que_Status(struct QUE * const que)
{
	Queue_Status_Type status;
	unsigned char temp_val;
	unsigned char max_in_que;
	CPU_SR cpu_sr;

	ENTER_CRITICAL();
	max_in_que = que->max;
	if (que->quein == que->queout)
	{
		status = Q_EMPTY;
	}
	else
	{
		temp_val = que->quein + 1;
		if (temp_val >= max_in_que)
		{
			temp_val = 0;
		}
		if (temp_val == que->queout)
		{
			status = Q_FULL;
		}
		else
		{
			status = Q_OK;
		}
	}
	EXIT_CRITICAL();
	return (status);
}

/*********************************************************************
 * Put the data on the specified que
 *********************************************************************/
void Que_Put(struct QUE * const que, const unsigned char data)
{
	unsigned char max_in_que;
	CPU_SR cpu_sr;

	ENTER_CRITICAL();
	max_in_que = que->max;
	*(que->data + que->quein) = data;
	(que->quein)++;
	if (que->quein == max_in_que)
	{
		que->quein = 0;
	}
	EXIT_CRITICAL();
	return;
}

/*********************************************************************
 * Return the data from specified que
 *********************************************************************/
unsigned char Que_Get(struct QUE *que)
{
	unsigned char temp_val;
	unsigned char max_in_que;
	CPU_SR cpu_sr;

	ENTER_CRITICAL();
	max_in_que = que->max;
	temp_val = *(que->data + que->queout);
	(que->queout)++;
	if (que->queout == max_in_que)
	{
		que->queout = 0;
	}
	EXIT_CRITICAL();
	return (temp_val);
}

/*********************************************************************
 * Look at the data from specified que
 *********************************************************************/
unsigned char Que_Look(struct QUE *que)
{
	unsigned char temp_val;
	CPU_SR cpu_sr;

	ENTER_CRITICAL();
	temp_val = *(que->data + que->queout);
	EXIT_CRITICAL();
	return temp_val;
}

/*********************************************************************
 * Remove one element from the top of the que
 *********************************************************************/
void Que_remove(struct QUE *que)
{
	unsigned char max_in_que;
	CPU_SR cpu_sr;

	ENTER_CRITICAL();
	max_in_que = que->max;
	if (que->quein != que->queout)
	{
		if (que->quein > 0)
		{
			que->quein--;
		}
		else
		{
			que->quein = max_in_que - 1;
		}
	}
	EXIT_CRITICAL();
	return;
}
/*********************************************************************
 * Dummy Enable disable all interrupts function
 *********************************************************************/
/****************************************************************************
 *  CPUSaveSR
 *  Save the __SREG__
 ****************************************************************************/
CPU_SR CPUSaveSR(void)
{
	CPU_SR cpu_sr;

	cpu_sr = SREG;
	cli();
	return (cpu_sr);
}
/****************************************************************************
 *  CPURestoreSR
 *  Restore the __SREG__
 ****************************************************************************/
void CPURestoreSR(CPU_SR cpu_sr)
{
	SREG = cpu_sr;
}

