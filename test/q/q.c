/*=================================================================================================================================

                                             Copyright (c) S A A B   A V I T R O N I C S

  File Name       : $RCSfile: q.c,v $
  Revision        : $Revision: 1.1 $
  Date            : $Date: 2009/08/28 10:24:41 $
  Abstract        : This module contains the base device driver class. All derived device drivers shall implement the interface
                    specified by the base device driver.

  \author         : $Author: za100736 $
  \package        : Protocol
  \subsection     : Q


 ================================================================================================================================*/
#include <avr/interrupt.h>

/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include "../TDefinitions.h"
#include "q.h"

/*--[ Private Literals ]---------------------------------------------------------------------------------------------------------*/
#define  ENTER_CRITICAL()  (cpu_sr = CPUSaveSR())    /* Disable interrupts                        */
#define  EXIT_CRITICAL()   (CPURestoreSR(cpu_sr))    /* Enable  interrupts                        */

/*--[ Type Definitions ]---------------------------------------------------------------------------------------------------------*/

/*--[ Private Constants ]--------------------------------------------------------------------------------------------------------*/

/*--[ Private Data ]-------------------------------------------------------------------------------------------------------------*/
typedef unsigned char CPU_SR;

/*--[ Private Function Prototypes ]----------------------------------------------------------------------------------------------*/
static CPU_SR CPUSaveSR(void);
static void CPURestoreSR(CPU_SR cpu_sr);

/*==[ PUBLIC FUNCTIONS ]=========================================================================================================*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
/*!

  \brief  Constructor

  Construct a new QUEUE for my type. Must be the first function called

  \public \memberof Q

  \trace_to  todo

*/
/*-------------------------------------------------------------------------------------------------------------------------------*/
void Queue_
(
  TQ    *const I,         /*!<[IN|OUT] Object Instance */
  void  *const pQ,        /*!<[IN|OUT] Address of my queue buffer */
  const uint16_t Qsize,     /*!<[IN] How many items to store on Queue*/
  const uint16_t SizeOfType /*!<[IN] Sizeof value of type in Queue */
)
{
  I->Head = 0;
  I->Tail = 0;
  I->pQ   = pQ;
  I->Size = Qsize;
  I->SizeOfType = SizeOfType;
  I->Status = Q_EMPTY;
  return;
}/*Queue_*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
/*!

  \brief  Write an item to the queue

  Check if the queue is not full before writing the item in byte-size to the designated queue buffer. The head is updated to
  include the new item.

  \public \memberof Q

  \return  QUEUE-status type

  \trace_to  todo

*/
/*-------------------------------------------------------------------------------------------------------------------------------*/
enum QUEUE_STATUS  QueueWrite
(
  TQ   *const I,        /*!<[IN|OUT] Object Instance */
  const void *const pData     /*!<[IN|OUT] Item to write into queue */
)
{
  uint16_t head_start;
  uint16_t i;
  CPU_SR cpu_sr;

  ENTER_CRITICAL();

  if ((I->Head != I->Tail) || (I->Status == Q_EMPTY))
  {
    head_start = I->Head * I->SizeOfType;
    for (i = 0; i < I->SizeOfType; i++)
    {
      ((uint8_t*)(I->pQ))[head_start + i] = ((uint8_t*)pData)[i];
    }
    I->Head++;
    if (I->Head >= I->Size)
    {
      I->Head = 0;
    }

    I->Status = (I->Head == I->Tail)?Q_FULL:Q_OK;
  }
  else
  {
    I->Status = Q_FULL;
  }

  EXIT_CRITICAL();

  return(I->Status);
}/*QueueWrite*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
/*!

  \brief  Read an item from the queue

  Check if he Queue is not empty before reading an item in byte-size back to the caller. The tail is updated to reflect that one
  item is removed from the queue.

  \public \memberof Q

  \return  QUEUE-status type

  \trace_to  todo

*/
/*-------------------------------------------------------------------------------------------------------------------------------*/
enum QUEUE_STATUS QueueRead
(
  TQ   *const I,        /*!<[IN|OUT] Object Instance */
  void *const pData     /*!<[OUT] Data from queue*/
)
{
  uint16_t tail_spin;
  uint16_t i;
  CPU_SR cpu_sr;

  ENTER_CRITICAL();

  if ((I->Head != I->Tail) || (I->Status == Q_FULL))
  {
    tail_spin = I->Tail * I->SizeOfType;
    for (i = 0; i < I->SizeOfType; i++)
    {
      ((uint8_t*)pData)[i] = ((uint8_t*)(I->pQ))[tail_spin + i];
    }
    (I->Tail)++;

    if (I->Tail >= I->Size)
    {
      I->Tail = 0;
    }

    I->Status = (I->Head == I->Tail)?Q_EMPTY:Q_OK;
  }
  else
  {
    I->Status = Q_EMPTY;
  }

  EXIT_CRITICAL();

  return(I->Status);
}/*QueueRead*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
/*!

  \brief  Check the emptyness

  When the head and tail of the queue is at the same index, the queue is deemed to be empty, otherwise there is something in the
  queue.

  \public \memberof Q

  \retval  TRUE  queue is empty
  \retval  FALSE  something in queue, queue is not empty

  \trace_to  todo

*/
/*-------------------------------------------------------------------------------------------------------------------------------*/
bool QueueEmpty
(
  const TQ *const I /*!<[IN|OUT] Object instance*/
)
{
  bool empty;

  empty = (I->Status == Q_EMPTY)?true:false;

  return(empty);
}/*QueueEmpty*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
/*!

  Report the internal status of the Queue.

  \public \memberof Q

  \return  QUEUE-status type

  \trace_to  todo

*/
/*-------------------------------------------------------------------------------------------------------------------------------*/
enum QUEUE_STATUS QueueStatus
(
  const TQ *const I /*!<[IN|OUT] Object instance*/
)
{
  return(I->Status);
}/*QueueStatus*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
/*!

  \brief  Flush the data

  Set the Head and tail to the same index and set the EMPTY flag, the Queue is now flushed

  \public \memberof Q

*/
/*-------------------------------------------------------------------------------------------------------------------------------*/
void QueueFlush
(
  TQ *const I /*!<[IN|OUT] Object instance*/
)
{
  CPU_SR cpu_sr;

  ENTER_CRITICAL();
  I->Status = Q_EMPTY;
  I->Head = I->Tail;
  EXIT_CRITICAL();

  return;
}/*QueueFlush*/

/****************************************************************************
 *  CPUSaveSR
 *  Save the __SREG__
 ****************************************************************************/
static CPU_SR CPUSaveSR(void)
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
static void CPURestoreSR(CPU_SR cpu_sr)
{
  SREG = cpu_sr;
}

/*===============================================================================================================================*/
