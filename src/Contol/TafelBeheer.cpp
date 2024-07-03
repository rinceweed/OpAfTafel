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
  TableToPosisie,
  TableUpDown,
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
#define SELECT_TIMEOUT           (5) /*5s*/
#define MOTOR_STEP_TIMEOUT       (1) /*10ms*/

#define HEARTBEAT_LED            (60) /*10ms*/
#define SLOW_SELECT_LED          (40) /*10ms*/
#define FAST_MOTOR_STEP_LED      (20) /*10ms*/
#define FAST_PROGRAM_STEP_LED    (10) /*10ms*/

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
bool LED_STATE = true;
static uint32_t HuidigeTafelPosisie;
static uint8_t HuidigePosisieIndex;
static uint32_t GeStoordePosisie;
static int32_t StepsToTake;
static bool Motor_Step;
static TAFEL_RIGTTING StepRigting;

static Tsm_SM Tafel_SM;
static DebounceNavigate DebounceHandle;

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Idle);
SM_MACRO_PROTO_STATE(Idle);
SM_MACRO_PROTO_RULE(Idle, 0);
SM_MACRO_PROTO_RULE(Idle, 1);
SM_MACRO_PROTO_RULE(Idle, 2);
SM_MACRO_PROTO_RULE(Idle, 3);
SM_MACRO_PROTO_RULE(Idle, 4);
SM_MACRO_PROTO_RULE(Idle, 5);
SM_MACRO_PROTO_RULE(Idle, 6);
SM_MACRO_RULE_LIST(Idle) =
{
  SM_MACRO_NAME_RULE(Idle, 0), SM_MACRO_NAME_RULE(Idle, 1), SM_MACRO_NAME_RULE(Idle, 2), SM_MACRO_NAME_RULE(Idle, 3), SM_MACRO_NAME_RULE(Idle, 4), SM_MACRO_NAME_RULE(Idle, 5), SM_MACRO_NAME_RULE(Idle, 6), NULL
};
 
/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(SelectPosisie);
SM_MACRO_PROTO_STATE(SelectPosisie);
SM_MACRO_PROTO_RULE(SelectPosisie, 0);
SM_MACRO_PROTO_RULE(SelectPosisie, 1);
SM_MACRO_PROTO_RULE(SelectPosisie, 2);
SM_MACRO_PROTO_RULE(SelectPosisie, 3);
SM_MACRO_RULE_LIST(SelectPosisie) =
{
  SM_MACRO_NAME_RULE(SelectPosisie, 0), SM_MACRO_NAME_RULE(SelectPosisie, 1), SM_MACRO_NAME_RULE(SelectPosisie, 2), SM_MACRO_NAME_RULE(SelectPosisie, 3), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(TableToPosisie);
SM_MACRO_PROTO_STATE(TableToPosisie);
SM_MACRO_PROTO_CLOSE(TableToPosisie);
SM_MACRO_PROTO_RULE(TableToPosisie, 0);
SM_MACRO_RULE_LIST(TableToPosisie) =
{
  SM_MACRO_NAME_RULE(TableToPosisie, 0), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(TableUpDown);
SM_MACRO_PROTO_STATE(TableUpDown);
SM_MACRO_PROTO_CLOSE(TableUpDown);
SM_MACRO_PROTO_RULE(TableUpDown, 0);
SM_MACRO_RULE_LIST(TableUpDown) =
{
  SM_MACRO_NAME_RULE(TableUpDown, 0), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Debounce);
SM_MACRO_PROTO_RULE(Debounce, 0);
SM_MACRO_RULE_LIST(Debounce) =
{
  SM_MACRO_NAME_RULE(Debounce, 0), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Done);
SM_MACRO_PROTO_STATE(Done);
SM_MACRO_PROTO_CLOSE(Done);
SM_MACRO_PROTO_RULE(Done, 0);
SM_MACRO_RULE_LIST(Done) =
{
  SM_MACRO_NAME_RULE(Done, 0), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Program);
SM_MACRO_PROTO_STATE(Program);
SM_MACRO_PROTO_RULE(Program, 0);
SM_MACRO_PROTO_RULE(Program, 1);
SM_MACRO_PROTO_RULE(Program, 2);
SM_MACRO_PROTO_RULE(Program, 3);
SM_MACRO_RULE_LIST(Program) =
{
  SM_MACRO_NAME_RULE(Program, 0), SM_MACRO_NAME_RULE(Program, 1), SM_MACRO_NAME_RULE(Program, 2), SM_MACRO_NAME_RULE(Program, 3), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(ProgramUpDown);
SM_MACRO_PROTO_STATE(ProgramUpDown);
SM_MACRO_PROTO_CLOSE(ProgramUpDown);
SM_MACRO_PROTO_RULE(ProgramUpDown, 0);
SM_MACRO_RULE_LIST(ProgramUpDown) =
{
  SM_MACRO_NAME_RULE(ProgramUpDown, 0), NULL
};

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(HomeTafel);
SM_MACRO_PROTO_STATE(HomeTafel);
SM_MACRO_PROTO_RULE(HomeTafel, 0);
SM_MACRO_RULE_LIST(HomeTafel) =
{
  SM_MACRO_NAME_RULE(HomeTafel, 0), NULL
};

/*==[ SM Data ]==================================================================================================================*/
static Tsm_States Tafel_States[MAX_TAFEL_STATES] =
{
  {SM_MACRO_NAME_OPEN(Idle), SM_MACRO_NAME_STATE(Idle), SM_MACRO_RULES(Idle), NULL},
  {SM_MACRO_NAME_OPEN(SelectPosisie), SM_MACRO_NAME_STATE(SelectPosisie), SM_MACRO_RULES(SelectPosisie), NULL},
  {SM_MACRO_NAME_OPEN(TableToPosisie), SM_MACRO_NAME_STATE(TableToPosisie), SM_MACRO_RULES(TableToPosisie), SM_MACRO_NAME_CLOSE(TableToPosisie)},
  {SM_MACRO_NAME_OPEN(TableUpDown), SM_MACRO_NAME_STATE(TableUpDown), SM_MACRO_RULES(TableUpDown), SM_MACRO_NAME_CLOSE(TableUpDown)},
  {SM_MACRO_NAME_OPEN(Debounce), NULL, SM_MACRO_RULES(Debounce), NULL},
  {SM_MACRO_NAME_OPEN(Done), SM_MACRO_NAME_STATE(Done), SM_MACRO_RULES(Done), SM_MACRO_NAME_CLOSE(Done)},
  {SM_MACRO_NAME_OPEN(Program), SM_MACRO_NAME_STATE(Program), SM_MACRO_RULES(Program), NULL},
  {SM_MACRO_NAME_OPEN(ProgramUpDown), SM_MACRO_NAME_STATE(ProgramUpDown), SM_MACRO_RULES(ProgramUpDown), SM_MACRO_NAME_CLOSE(ProgramUpDown)},
  {SM_MACRO_NAME_OPEN(HomeTafel), SM_MACRO_NAME_STATE(HomeTafel), SM_MACRO_RULES(HomeTafel), NULL}
};

/* ==============================================================================================================================*/
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TafelBeheerInit()
{
  ButtonInitialise();
  ConfigureTimer(TIME_LED_SLOW, .01);
  ConfigureTimer(TIME_LED_FAST, .01);
  ConfigureTimer(TIME_MOTOR_STEP, .01); //10ms
  ConfigureTimer(TIME_SELECT, 1); //1s

  pinMode(MOTOR_PULSE, OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_ENABLE, OUTPUT);

  for (uint8_t i = 0; i < MAX_POSISIES; i++)
  {
    pinMode((i + LED_OFFSET), OUTPUT);
  }

  uint32_t start = IsTafelHomed() == HOMED ? Idle: HomeTafel;
  
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
  Serial.println(F("Idle"));
  HuidigeTafelPosisie = KryTafelPosisie();
  HuidigePosisieIndex = KryGekosePosisieIndex();
  digitalWrite((HuidigePosisieIndex + LED_OFFSET), true);
  ShowMem();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(Idle)
{
  unsigned long current_time = WhatIsCount(TIME_LED_SLOW);

  // Should increment every BOTTLE_FLOW_TMER_MS
  if (current_time > HEARTBEAT_LED)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite(13, LED_STATE);
    StartCount(TIME_LED_SLOW);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 0)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_IN);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> TableToPosisie"));
    ((DebounceNavigate*)pI)->goOn = TableToPosisie;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 1)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_OP);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> Up -> TableUpDown"));
    StepRigting = Opwaarts;
    ((DebounceNavigate*)pI)->goOn = TableUpDown;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = TableUpDown;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 2)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_AF);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> Af -> TableUpDown"));
    StepRigting = Afwaarts;
    ((DebounceNavigate*)pI)->goOn = TableUpDown;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = TableUpDown;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 3)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_ADJUST);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> Program"));
    ((DebounceNavigate*)pI)->goOn = Program;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 4)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_LINKS);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> L -> SelectPosisie"));
    ((DebounceNavigate*)pI)->goOn = SelectPosisie;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    HuidigePosisieIndex--;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 5)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_REGS);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> R -> SelectPosisie"));
    ((DebounceNavigate*)pI)->goOn = SelectPosisie;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    HuidigePosisieIndex++;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Idle, 6)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_RESET);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Idle -> HomeTafel"));
    ((DebounceNavigate*)pI)->goOn = HomeTafel;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = Debounce;
  }
}

// SelectPosisie ================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(SelectPosisie)
{
  Serial.println(F("SelectPosisie"));
  // If very big then it was 0--
  HuidigePosisieIndex = (HuidigePosisieIndex > (2 * MAX_POSISIES)) ? 0 : (HuidigePosisieIndex >= MAX_POSISIES) ? MAX_POSISIES - 1 : HuidigePosisieIndex;
  for (uint8_t i = 0; i < MAX_POSISIES; i++)
  {
    digitalWrite((i + LED_OFFSET), false);
  }

  StartCount(TIME_SELECT);
  StartCount(TIME_LED_FAST);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(SelectPosisie)
{
  unsigned long current_time = WhatIsCount(TIME_LED_FAST);

  // Should flash the current Index LED
  if (current_time > SLOW_SELECT_LED)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite((HuidigePosisieIndex + LED_OFFSET), LED_STATE);
    StartCount(TIME_LED_FAST);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(SelectPosisie, 0)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_IN);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("SelectPosisie -> Idle"));
    ((DebounceNavigate*)pI)->goOn = Idle;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    StoorGekosePosisieIndex(HuidigePosisieIndex);
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(SelectPosisie, 1)
{
  unsigned long current_time = WhatIsCount(TIME_SELECT);

  if (current_time > SELECT_TIMEOUT)
  {
    Serial.println(F("SelectPosisie -> T -> Idle"));
    *pstate = Idle;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(SelectPosisie, 2)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_LINKS);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("SelectPosisie -> L"));
    ((DebounceNavigate*)pI)->goOn = SelectPosisie;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    HuidigePosisieIndex--;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(SelectPosisie, 3)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_REGS);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("SelectPosisie -> R"));
    ((DebounceNavigate*)pI)->goOn = SelectPosisie;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    HuidigePosisieIndex++;
    *pstate = Debounce;
  }
}

// TableToPosisie ===============================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(TableToPosisie)
{
  Serial.println(F("TableToPosisie"));
  HuidigeTafelPosisie = KryTafelPosisie();
  GeStoordePosisie = KryGeStoordePosisie(HuidigePosisieIndex);

  int32_t steppies = GeStoordePosisie - HuidigeTafelPosisie;
  //TODO: check if not missing one step
  StepsToTake = abs(steppies);
  Motor_Step = false;
  StepRigting = (steppies < 0) ? Afwaarts : Opwaarts;
  Serial.print(F("HuidigeTafelPosisie"));
  Serial.print(F(" : "));
  Serial.print(HuidigeTafelPosisie);
  Serial.print(F(" : "));
  Serial.print(F("GeStoordePosisie"));
  Serial.print(F(" : "));
  Serial.print(GeStoordePosisie);
  Serial.print(F(" : "));
  Serial.print(F("steppies"));
  Serial.print(F(" : "));
  Serial.print(steppies);
  Serial.print(F("\nStepsToTake"));
  Serial.print(F(" : "));
  Serial.print(StepsToTake);
  Serial.print(F(" : "));
  Serial.print(F("StepRigting"));
  Serial.print(F(" : "));
  Serial.println(StepRigting);
  digitalWrite(MOTOR_DIR, StepRigting);
  digitalWrite(MOTOR_PULSE, Motor_Step);
  digitalWrite(MOTOR_ENABLE, true);
  StartCount(TIME_LED_FAST);
  StartCount(TIME_MOTOR_STEP);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(TableToPosisie)
{
  unsigned long current_time = WhatIsCount(TIME_MOTOR_STEP);

  if (current_time > MOTOR_STEP_TIMEOUT)
  {
    StepsToTake--;
    HuidigeTafelPosisie += ((StepRigting == Opwaarts) ? (1) : (-1));
    Motor_Step = !Motor_Step;
    digitalWrite(MOTOR_PULSE, Motor_Step);
    StartCount(TIME_MOTOR_STEP);
  }
  
  current_time = WhatIsCount(TIME_LED_FAST);

  if (current_time > FAST_MOTOR_STEP_LED)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite((HuidigePosisieIndex + LED_OFFSET), LED_STATE);
    StartCount(TIME_LED_FAST);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(TableToPosisie, 0)
{
  if (StepsToTake < 0)
  {
    Serial.println(F("TableToPosisie -> Idle"));
    *pstate = Idle;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_CLOSE(TableToPosisie)
{
  digitalWrite(MOTOR_ENABLE, false);
  StoorTafelPosisie(HuidigeTafelPosisie);
}

// TableUpDown ==================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(TableUpDown)
{
  Serial.println(F("TableUpDown"));
  HuidigeTafelPosisie = KryTafelPosisie();
  StepRigting = (((DebounceNavigate*)pI)->pressedButton->button == KEY_OP) ? Opwaarts: Afwaarts;
  Motor_Step = false;
  digitalWrite(MOTOR_DIR, StepRigting);
  digitalWrite(MOTOR_PULSE, Motor_Step);
  digitalWrite(MOTOR_ENABLE, true);
  StartCount(TIME_MOTOR_STEP);
  StartCount(TIME_LED_FAST);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(TableUpDown)
{
  unsigned long current_time = WhatIsCount(TIME_MOTOR_STEP);

  if (current_time > MOTOR_STEP_TIMEOUT)
  {
    HuidigeTafelPosisie += ((StepRigting == Opwaarts) ? (1) : (-1));
    Motor_Step = !Motor_Step;
    digitalWrite(MOTOR_PULSE, Motor_Step);
    StartCount(TIME_MOTOR_STEP);
  }

  current_time = WhatIsCount(TIME_LED_FAST);

  if (current_time > FAST_MOTOR_STEP_LED)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite((HuidigePosisieIndex + LED_OFFSET), LED_STATE);
    StartCount(TIME_LED_FAST);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(TableUpDown, 0)
{
  if (((DebounceNavigate*)pI)->pressedButton->buttonState == false)
  {
    Serial.println(F("TableUpDown - > Idle"));
    *pstate = Idle;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_CLOSE(TableUpDown)
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

// Done =========================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Done)
{
  Serial.println(F("Done"));
  StepRigting = Opwaarts;
  Motor_Step = false;

  digitalWrite(MOTOR_DIR, StepRigting);
  digitalWrite(MOTOR_PULSE, Motor_Step);
  digitalWrite(MOTOR_ENABLE, true);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(Done)
{
  unsigned long current_time = WhatIsCount(TIME_MOTOR_STEP);

  if (current_time > MOTOR_STEP_TIMEOUT)
  {
    Motor_Step = !Motor_Step;
    digitalWrite(MOTOR_PULSE, Motor_Step);
    StartCount(TIME_MOTOR_STEP);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Done, 0)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_HOME);

  //Go up until switch is disabled
  if (current_button_ptr->buttonState == true)
  {
    *pstate = Idle;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_CLOSE(Done)
{
  digitalWrite(MOTOR_PULSE, false);
  StoorTafelPosisie(0);
  TafelIsHomed();
}

// Program ======================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(Program)
{
  Serial.println(F("Program"));
  StartCount(TIME_SELECT);
  StartCount(TIME_LED_FAST);
  HuidigePosisieIndex = KryGekosePosisieIndex();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(Program)
{
  unsigned long current_time = WhatIsCount(TIME_LED_FAST);

  if (current_time > FAST_PROGRAM_STEP_LED)
  {
    LED_STATE = !LED_STATE;      //Invert LED state
    digitalWrite((HuidigePosisieIndex + LED_OFFSET), LED_STATE);  //Write new state to the LED on pin D5
    StartCount(TIME_LED_FAST);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Program, 0)
{
  // Do Nothing go back
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_ADJUST);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Program -> Idle"));
    ((DebounceNavigate*)pI)->goOn = Idle;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Program, 1)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_OP);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Program -> Up -> ProgramUpDown"));
    StepRigting = Opwaarts;
    ((DebounceNavigate*)pI)->goOn = ProgramUpDown;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = ProgramUpDown;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Program, 2)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_AF);

  if (current_button_ptr->buttonState == true)
  {
    Serial.println(F("Program -> Af -> ProgramUpDown"));
    StepRigting = Afwaarts;
    ((DebounceNavigate*)pI)->goOn = ProgramUpDown;
    ((DebounceNavigate*)pI)->pressedButton = current_button_ptr;
    *pstate = ProgramUpDown;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(Program, 3)
{
  unsigned long current_time = WhatIsCount(TIME_SELECT);

  if (current_time > SELECT_TIMEOUT)
  {
    Serial.println(F("Program -> T -> Idle"));
    *pstate = Idle;
  }
}

// ProgramUpDown ================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(ProgramUpDown)
{
  Serial.println(F("ProgramUpDown"));
  StartCount(TIME_SELECT);
  HuidigeTafelPosisie = KryTafelPosisie();

  Motor_Step = false;
  digitalWrite(MOTOR_DIR, StepRigting);
  digitalWrite(MOTOR_PULSE, Motor_Step);
  digitalWrite(MOTOR_ENABLE, true);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(ProgramUpDown)
{
  unsigned long current_time = WhatIsCount(TIME_MOTOR_STEP);

  if (current_time > MOTOR_STEP_TIMEOUT)
  {
    HuidigeTafelPosisie += ((StepRigting == Opwaarts) ? (1) : (-1));
    Motor_Step = !Motor_Step;
    digitalWrite(MOTOR_PULSE, Motor_Step);
    StartCount(TIME_MOTOR_STEP);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(ProgramUpDown, 0)
{
  // Key lifted go back
  if (((DebounceNavigate*)pI)->pressedButton->buttonState == false)
  {
    Serial.println(F("ProgramUpDown -> Program"));
    ((DebounceNavigate*)pI)->goOn = Program;
    *pstate = Debounce;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_CLOSE(ProgramUpDown)
{
  digitalWrite(MOTOR_ENABLE, false);
  StoorTafelPosisie(HuidigeTafelPosisie);
  StoorPosisie(HuidigePosisieIndex, HuidigeTafelPosisie);
}

// HomeTafel ====================================================================================================================
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_OPEN(HomeTafel)
{
  Serial.println(F("HomeTafel"));
  StepRigting = Afwaarts;
  Motor_Step = false;

  digitalWrite(MOTOR_DIR, StepRigting);
  digitalWrite(MOTOR_PULSE, Motor_Step);
  digitalWrite(MOTOR_ENABLE, true);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_STATE(HomeTafel)
{
  unsigned long current_time = WhatIsCount(TIME_MOTOR_STEP);

  if (current_time > MOTOR_STEP_TIMEOUT)
  {
    Motor_Step = !Motor_Step;
    digitalWrite(MOTOR_PULSE, Motor_Step);
    StartCount(TIME_MOTOR_STEP);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
SM_MACRO_PROTO_RULE(HomeTafel, 0)
{
  ButtonPinDebounce *current_button_ptr = ButtonOnKey(KEY_HOME);

  if (current_button_ptr->buttonState == false)
  {
    Serial.println(F("HomeTafel -> Done"));
    *pstate = Done;
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*==[ PRIVATE FUNCTIONS ]========================================================================================================*/
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/

/*===============================================================================================================================*/
