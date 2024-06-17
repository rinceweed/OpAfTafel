/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "Tsm_i.h"
#include "Button.h"
#include "Timer.h"

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/
typedef enum TafelStates
{
  Idle,
  MoveUpDown,
  Debounce,
  Done,
  Program,
  ProgramUpDown,
  MAX_TAFEL_STATES
}TAFEL_STATES;

typedef enum TafelRigtings
{
  Op,
  Af,
  MAX_TAFEL_RIGTING
}TAFEL_RIGTTING;

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/

/*--[ Constants ]----------------------------------------------------------------------------------------------------------------*/

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
bool LED_STATE = true;
PinDebounce ButtonDebounce[KEY_MAX] =
{
  { button : KEY_ADJUST, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_LINKS, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_REGS, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_OP, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_AF, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_IN, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_RESET, buttonState : false,  currentButtonState : false, debounceTime : 20}
};

typedef struct StateDebounceNavigate
{
  TAFEL_STATES goOn;
  PinDebounce  *pressedButton;
} DebounceNavigate;

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Idle);
SM_MACRO_PROTO_STATE(Idle);
SM_MACRO_PROTO_RULE(Idle, 0);
SM_MACRO_RULE_LIST(Idle) =
{
  SM_MACRO_NAME_RULE(Idle, 0), NULL
};
 
/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(MoveUpDown);
SM_MACRO_PROTO_STATE(MoveUpDown);
SM_MACRO_PROTO_RULE(MoveUpDown, 0);
SM_MACRO_RULE_LIST(MoveUpDown) =
{
  SM_MACRO_NAME_RULE(MoveUpDown, 0), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Debounce);
SM_MACRO_PROTO_RULE(Debounce, 0);
SM_MACRO_RULE_LIST(Debounce) =
{
  SM_MACRO_NAME_RULE(Debounce, 0), NULL
};
/*==[ PUBLIC FUNCTIONS ]=========================================================================================================*/

static Tsm_SM Tafel_SM;
static uint8_t TafelRigting[MAX_TAFEL_RIGTING];
static DebounceNavigate DebounceHandle;

/* =========================================================================
  Data */
static Tsm_States Tafel_States[MAX_TAFEL_STATES] =
{
  {SM_MACRO_NAME_OPEN(Idle), SM_MACRO_NAME_STATE(Idle), SM_MACRO_RULES(Idle), NULL},
  {SM_MACRO_NAME_OPEN(MoveUpDown), SM_MACRO_NAME_STATE(MoveUpDown), SM_MACRO_RULES(MoveUpDown), NULL},
  {SM_MACRO_NAME_OPEN(Debounce), NULL, SM_MACRO_RULES(Debounce), NULL},
};

/* =========================================================================
  Public Function */

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TafelBeheerInit()
{
  ButtonInitialise();
  ConfigureTimer(TIME_LED_SLOW, .25);
  ConfigureTimer(TIME_LED_FAST, .125);

  for (uint8_t i = 0; i < KEY_MAX; i++)
  {
    pinMode(ButtonDebounce[i].button, INPUT);
  }
  Tsm_Create(&Tafel_SM, Tafel_States, &DebounceHandle, Idle, MAX_TAFEL_STATES);
  return;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TafelBeheerSM()
{
  for (uint8_t i = 0; i < KEY_MAX; i++)
  {
    CheckButtonPress(&(ButtonDebounce[i]));
  }
  Tsm_Run(&Tafel_SM);
}

// Idle =========================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Idle)
{
  Serial.println(F("Idle "));
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(Idle)
{
  unsigned long current_time = WhatIsCount(TIME_LED_SLOW);

  // Should increment every BOTTLE_FLOW_TMER_MS
  if (current_time > 1)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite(13, LED_STATE);  //Write new state to the LED on pin D5
    StartCount(TIME_LED_SLOW);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 0)
{
  if (ButtonDebounce[KEY_ADJUST].buttonState == true)
  {
    Serial.println(F("Idle -> MoveUpDown"));
    Tafel_SM.Current =  Debounce;
    ((DebounceNavigate*)pI)->goOn = MoveUpDown;
    ((DebounceNavigate*)pI)->pressedButton = &(ButtonDebounce[KEY_ADJUST]);
  }
}
// MoveUpDown ====================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(MoveUpDown)
{
  Serial.println(F("MoveUpDown "));
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(MoveUpDown)
{
  unsigned long current_time = WhatIsCount(TIME_LED_FAST);

  // Should increment every BOTTLE_FLOW_TMER_MS
  if (current_time > 1)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite(13, LED_STATE);  //Write new state to the LED on pin D5
    StartCount(TIME_LED_FAST);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(MoveUpDown, 0)
{
  if (ButtonDebounce[KEY_ADJUST].buttonState)
  {
    Serial.println(F("MoveUpDown -> debounce"));
    Tafel_SM.Current =  Debounce;
    ((DebounceNavigate*)pI)->goOn = Idle;
    ((DebounceNavigate*)pI)->pressedButton = &(ButtonDebounce[KEY_ADJUST]);
  }
}

// Debounce =====================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Debounce)
{
  Serial.println(F("Debounce "));
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Debounce, 0)
{
  if (((DebounceNavigate*)pI)->pressedButton->buttonState == false)
  {
    Tafel_SM.Current = ((DebounceNavigate*)pI)->goOn;
  }
}
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*==[ PRIVATE FUNCTIONS ]========================================================================================================*/
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*===============================================================================================================================*/
