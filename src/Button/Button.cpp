#include <Arduino.h>
#include "Timer.h"
#include "Button.h"

#define PIN_DEBOUNCE    3

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
ButtonPinDebounce ButtonDebounce[] = //wont work, keys need to be in order
{
  { button : KEY_ADJUST, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_LINKS, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_REGS, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_OP, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_AF, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_IN, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_RESET, buttonState : false,  currentButtonState : false, debounceTime : 20},
  { button : KEY_HOME, buttonState : false,  currentButtonState : false, debounceTime : 20},
};

uint32_t ButtonDebounceLookupSize = sizeof(ButtonDebounce)/sizeof(ButtonPinDebounce);

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress(ButtonPinDebounce *fPinIn);

/* ==============================================================================================================================*/
/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ButtonInitialise()
{
  ConfigureTimer(TIME_KEYPRESS, 0.01);
  for (uint8_t i = 0; i < ButtonDebounceLookupSize; i++)
  {
    pinMode(ButtonDebounce[i].button, INPUT);
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
ButtonPinDebounce * ButtonOnKey(enum KEYS key)
{
  for (uint8_t index = 0; (index < ButtonDebounceLookupSize); index++)
  {
    if (ButtonDebounce[index].button == key)
    {
      return &ButtonDebounce[index];
    }
  }
  //not nice but ok
  return nullptr;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress()
{
  for (uint8_t i = 0; i < ButtonDebounceLookupSize; i++)
  {
    CheckButtonPress(&(ButtonDebounce[i]));
  }
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress(ButtonPinDebounce *fPinIn)
{
  // read the state of the switch into a local variable:
  int pinDValue = digitalRead(fPinIn->button);
  bool value = pinDValue > 0 ? true : false;

  // check to see if you just pressed the button
  // (i.e. the input went from LOW to HIGH), and you've waited long enough
  // since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (value != fPinIn->currentButtonState)
  {
    // reset the debouncing timer
    fPinIn->debounceTime = WhatIsCount(TIME_KEYPRESS);
    fPinIn->currentButtonState = value;
  }

  unsigned long current_time = WhatIsCount(TIME_KEYPRESS);
  unsigned long high_time = current_time;
  unsigned long low_time = fPinIn->debounceTime;
  if (current_time < fPinIn->debounceTime)
  {
    low_time = current_time;
    high_time = fPinIn->debounceTime + (fPinIn->debounceTime - current_time);
  }

  unsigned long debounce_time = high_time - low_time;

  if (debounce_time > PIN_DEBOUNCE)
  {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:
    fPinIn->buttonState = value;
    //  Serial.print(F("Button "));
    //  Serial.print(fPinIn->button);
    //  Serial.print(" : ");
    //  Serial.println(fPinIn->buttonState);
    
    // snprintf(_snbuffer, sizeof(_snbuffer), "Raw: x: %i, y: %i, z: %i", *x, *y, *z);
    // Serial.println(_snbuffer);

  }
}