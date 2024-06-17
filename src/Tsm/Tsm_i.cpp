/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include "Tsm_i.h"

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Type Definitions ]---------------------------------------------------------------------------------------------------------*/

/*--[ Constants ]----------------------------------------------------------------------------------------------------------------*/

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/

/*--[ Function Prototypes ]------------------------------------------------------------------------------------------------------*/

/*==[ PUBLIC FUNCTIONS ]=========================================================================================================*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void Tsm_Create(Tsm_SM* const me, const Tsm_States* const pStates, void * const owner, const uint32_t initState, const uint32_t stateCount)
{
  /* List of states */
  me->pStateFunc = pStates;
  /* Owner class */
  me->pI = owner;
  /* Current state - default */
  me->Current = initState;
  /* Previous state - set to maximum so that an initial state transition is forced. */
  me->History = stateCount;

  return;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
uint32_t Tsm_Run(Tsm_SM* const me)
{
  pTPrule  *pprule_list;

  /* Check if state open functionality needs to be xecuted */
  if (me->Current != me->History)
  {
    if (me->pStateFunc[me->Current].pOpen != NULL)
    {
      me->pStateFunc[me->Current].pOpen(me->pI);
    }
    me->History = me->Current;
  }

  /* Functionality to perform in state */
  if (me->pStateFunc[me->Current].pState != NULL)
  {
    me->pStateFunc[me->Current].pState(me->pI);
  }

  /* Go through the list of rules */
  pprule_list = me->pStateFunc[me->Current].pPrule_list;
  while ((*pprule_list != NULL) && (me->History == me->Current))
  {
    (*pprule_list)(me->pI, &me->Current);
    pprule_list++;
  }

  /* Stuff todo when leaving for another state */
  if (me->Current != me->History)
  {
    if (me->pStateFunc[me->History].pClose != NULL)
    {
      me->pStateFunc[me->History].pClose(me->pI, &me->Current);
    }
  }
  return(me->Current);
}

/*==[ PRIVATE FUNCTIONS ]========================================================================================================*/
/*===============================================================================================================================*/

