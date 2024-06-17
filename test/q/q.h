/*=================================================================================================================================

  File Name       : $RCSfile: q.h,v $
  Revision        : $Revision: 1.1 $
  Date            : $Date: 2009/08/28 10:24:44 $
  Abstract        : This module contains the Q device class.

  \author         : $Author: za100736 $
  \package        : Q


 ================================================================================================================================*/
#ifndef Q_H_
#define Q_H_

/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
/*! Queue status */
enum QUEUE_STATUS
{
  Q_EMPTY = 0,
  Q_FULL,
  Q_OK
};

/*! Defines a generic QUEUE type that copies using UINT8s */
typedef struct QUEUE_STRUCT
{
  uint16_t  Head;  /*!< Last item in */
  uint16_t  Tail;  /*!< First item out */
  void      *pQ;   /*!< Byte start of Queue */
  uint16_t  Size;  /*!< Number of items in Queue */
  uint16_t  SizeOfType;     /*!< Size in bytes of the type thats queued */
  enum QUEUE_STATUS Status; /*!< Keep track of the queue */
}TQ;

/*--[ Constants ]----------------------------------------------------------------------------------------------------------------*/

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void Queue_(TQ *const I, void *const pQ, const uint16_t Qsize, const uint16_t SizeOfType);
enum QUEUE_STATUS QueueWrite(TQ *const I, const void *const pData);
enum QUEUE_STATUS QueueRead(TQ *const I, void *const pData);
enum QUEUE_STATUS QueueStatus(const TQ *const I);
bool QueueEmpty(const TQ *const I);
void QueueFlush(TQ *const I);

/*===============================================================================================================================*/
#endif /* Q_H_ */
