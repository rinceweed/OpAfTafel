/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/
#include <Arduino.h>
#include "Tsm_i.h"
#include "Button.h"
#include "Timer.h"
#include "EepromMap.h"
#include "TafelBeheer.h"

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/
typedef enum TafelStates
{
  Idle,
  SelectPosisie,
  MoveUpDown,
  Debounce,
  Done,
  Program,
  ProgramUpDown,
  HomeTafel,
  MAX_TAFEL_STATES
}TAFEL_STATES;

typedef enum TafelRigtings
{
  Afwaarts = 0,
  Opwaarts = 1,
  MAX_TAFEL_RIGTING
}TAFEL_RIGTTING;

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
typedef struct StateDebounceNavigate
{
  TAFEL_STATES goOn;
  ButtonPinDebounce  *pressedButton;
} DebounceNavigate;

/*--[ Constants ]----------------------------------------------------------------------------------------------------------------*/

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
bool LED_STATE = true;
static uint32_t HuidigeTafelPosisie;
static uint8_t HuidigePosisieIndex;
static uint32_t HuidigePosisie;
static uint32_t StepsToTake;
static bool Motor_Step;
static TAFEL_RIGTTING StepRigting;

static Tsm_SM Tafel_SM;
static DebounceNavigate DebounceHandle;

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Idle);
SM_MACRO_PROTO_STATE(Idle);
SM_MACRO_PROTO_RULE(Idle, 0);
SM_MACRO_RULE_LIST(Idle) =
{
  SM_MACRO_NAME_RULE(Idle, 0), NULL
};
 
/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_RULE_LIST(SelectPosisie) =
{
  NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(MoveUpDown);
SM_MACRO_PROTO_STATE(MoveUpDown);
SM_MACRO_PROTO_CLOSE(MoveUpDown);
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

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_RULE_LIST(Done) =
{
  NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_RULE_LIST(Program) =
{
  NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_RULE_LIST(ProgramUpDown) =
{
  NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_RULE_LIST(HomeTafel) =
{
  NULL
};

/*==[ SM Data ]==================================================================================================================*/
static Tsm_States Tafel_States[MAX_TAFEL_STATES] =
{
  {SM_MACRO_NAME_OPEN(Idle), SM_MACRO_NAME_STATE(Idle), SM_MACRO_RULES(Idle), NULL},
  {NULL, NULL, SM_MACRO_RULES(SelectPosisie), NULL},
  {SM_MACRO_NAME_OPEN(MoveUpDown), SM_MACRO_NAME_STATE(MoveUpDown), SM_MACRO_RULES(MoveUpDown), SM_MACRO_NAME_CLOSE(MoveUpDown)},
  {SM_MACRO_NAME_OPEN(Debounce), NULL, SM_MACRO_RULES(Debounce), NULL},
  {NULL, NULL, SM_MACRO_RULES(Done), NULL},
  {NULL, NULL, SM_MACRO_RULES(Program), NULL},
  {NULL, NULL, SM_MACRO_RULES(ProgramUpDown), NULL},
  {NULL, NULL, SM_MACRO_RULES(HomeTafel), NULL}
};

/* ==============================================================================================================================*/
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TafelBeheerInit()
{
  ButtonInitialise();
  ConfigureTimer(TIME_LED_SLOW, .25);
  ConfigureTimer(TIME_LED_FAST, .125);
  ConfigureTimer(TIME_MOTOR_STEP, .01); //10ms

  pinMode(MOTOR_PULSE, OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);

  uint32_t start = IsTafelHomed() == HOMED ? Idle: HomeTafel;
  HuidigeTafelPosisie = KryTafelPosisie();
  HuidigePosisieIndex = KryGekosePosisieIndex();
  
  Tsm_Create(&Tafel_SM, Tafel_States, &DebounceHandle, start, MAX_TAFEL_STATES);
  return;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TafelBeheerSM()
{
  CheckButtonPress();
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
  if (current_time > 2)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite(13, LED_STATE);  //Write new state to the LED on pin D5
    StartCount(TIME_LED_SLOW);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 0)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_IN);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> MoveUpDown"));
    *pstate = Debounce;
    ((DebounceNavigate*)pI)->goOn = MoveUpDown;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
  }
}
// MoveUpDown ====================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(MoveUpDown)
{
  Serial.println(F("MoveUpDown "));
  HuidigeTafelPosisie = KryTafelPosisie();
  HuidigePosisie = KryGeStoordePosisie(HuidigePosisieIndex);

  int steppies = HuidigeTafelPosisie - HuidigePosisie;
  //TODO: check if not missing one step
  StepsToTake = abs(steppies);
  Motor_Step = false;
  StepRigting = (steppies < 0) ? Opwaarts: Afwaarts;
  digitalWrite(MOTOR_DIR, StepRigting);
  digitalWrite(MOTOR_PULSE, Motor_Step);
  digitalWrite(MOTOR_ENABLE, true);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(MoveUpDown)
{
  unsigned long current_time = WhatIsCount(TIME_MOTOR_STEP);

  if (current_time > 1)
  {
    StepsToTake--;
    HuidigeTafelPosisie +=  (StepRigting == Opwaarts) ? (1) : (-1);
    Motor_Step = !Motor_Step;
    digitalWrite(MOTOR_PULSE, Motor_Step);
    StartCount(TIME_MOTOR_STEP);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(MoveUpDown, 0)
{
  if (StepsToTake < 1)
  {
    Serial.println(F("MoveUpDown -> Idle"));
    *pstate = Idle;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_CLOSE(MoveUpDown)
{
  digitalWrite(MOTOR_ENABLE, false);
  StoorTafelPosisie(HuidigeTafelPosisie);
}

// Debounce =====================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Debounce)
{
  Serial.println(F("Debounce"));
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Debounce, 0)
{
  if (((DebounceNavigate*)pI)->pressedButton->buttonState == false)
  {
    *pstate = ((DebounceNavigate*)pI)->goOn;
  }
}
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*==[ PRIVATE FUNCTIONS ]========================================================================================================*/
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*===============================================================================================================================*/
