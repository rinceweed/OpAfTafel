/*--[ Visibility ]---------------------------------------------------------------------------------------------------------------*/
#ifndef TSM_H
#define TSM_H
#include <Arduino.h>

/*==[ GLOBALLY VISIBLE ]=========================================================================================================*/

  /*--[ Literals ]---------------------------------------------------------------------------------------------------------------*/
  /*! Value to use to indicate the SM is IDLE */
  #define STATE_SM_IDLE                                  (0x11110000)
  /*! Value to use to indicate the SM is BUSY */
  #define STATE_SM_BUSY                                  (0x22220000)
  /*! Value to use to indicate the SM is in a DONE state */
  #define STATE_SM_DONE                                  (0x44440000)

  /*--[ Type Definitions ]-------------------------------------------------------------------------------------------------------*/
  /*! Define the template for the Open function */
  typedef void (*pTOpen)(void *const pI);
  /*! Define the template for the State function */
  typedef void (*pTState)(void *const pI);
  /*! Define the template for the Rule function */
  typedef void (*pTPrule)(void *const pI, uint32_t *const pstate);
  /*! Define the template for the Close function */
  typedef void (*pTClose)(void *const pI, uint32_t *const pnewstate);

  /*! State functions defined for list. Unused state-functionality is indicated by NULL */
  typedef struct Tsm_States
  {
    pTOpen   pOpen;        /*!< open: Each state has first time in functionality */
    pTState  pState;       /*!< state: functionality executed while in the state. */
    pTPrule  *const pPrule_list; /*!< rule_list: Rules for leaving the state MUST be terminated by a NULL. */
    pTClose  pClose;       /*!< close: When leaving the state some stuff can be done.
                                      The new state value is available to perform to-state specific functionality*/
  }Tsm_States;

  /*! These members defines the State Machine */
  typedef struct Tsm_SM
  {
    uint32_t          Current;     /*!< Current State */
    uint32_t          History;     /*!< History of the Current State */
    const Tsm_States  *pStateFunc; /*!< Points to the States Function List */
    void              *pI;         /*!< Owner of this states*/
  }Tsm_SM;

  /*--[ Macro ]------------------------------------------------------------------------------------------------------------------*/
  #define SM_MACRO_NAME_OPEN(EnumState)                    (EnumState##_open)
  #define SM_MACRO_NAME_STATE(EnumState)                   (EnumState##_state)
  #define SM_MACRO_NAME_RULE(EnumState, idx)               (EnumState##_rule##idx)
  #define SM_MACRO_NAME_CLOSE(EnumState)                   (EnumState##_close)
  #define SM_MACRO_RULE_LIST(EnumState)                    static pTPrule EnumState##_Rule_List[]
  #define SM_MACRO_RULES(EnumState)                        (EnumState##_Rule_List)

  #define SM_MACRO_PROTO_OPEN(EnumState)                   static void EnumState##_open(void *const pI)
  #define SM_MACRO_PROTO_STATE(EnumState)                  static void EnumState##_state(void *const pI)
  #define SM_MACRO_PROTO_RULE(EnumState, idx)              static void EnumState##_rule##idx(void *const pI, uint32_t *const pstate)
  #define SM_MACRO_PROTO_CLOSE(EnumState)                  static void EnumState##_close(void *const pI, uint32_t *const pnewstate)


  /*--[ Data ]-------------------------------------------------------------------------------------------------------------------*/

  /*--[ Prototypes ]-------------------------------------------------------------------------------------------------------------*/
  /*! State Machine Driver API */
  void Tsm_Create(Tsm_SM* const me, const Tsm_States* const pStates, void* const owner, const uint32_t initState, const uint32_t stateCount);
  uint32_t Tsm_Run(Tsm_SM* const me);

#endif
/*===============================================================================================================================*/

