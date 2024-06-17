#include <Arduino.h>
#include "Timer.h"
#include "Button.h"

#define PIN_DEBOUNCE    3

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ButtonInitialise()
{
  ConfigureTimer(TIME_KEYPRESS, 0.01);
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void CheckButtonPress(PinDebounce *fPinIn)
{
  // read the state of the switch into a local variable:
  bool value = digitalRead(fPinIn->button) ? true : false;

  // Serial.print(F("Pin "));
  // Serial.print(fPinIn->button);
  // Serial.println(value);

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
    // Serial.print(F("Button "));
    // Serial.print(fPinIn->button);
    // Serial.println(fPinIn->buttonState);
    
    // snprintf(_snbuffer, sizeof(_snbuffer), "Raw: x: %i, y: %i, z: %i", *x, *y, *z);
    // Serial.println(_snbuffer);

  }
}