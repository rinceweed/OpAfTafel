#include <Arduino.h>
#include "Timer.h"
#include "TafelBeheer.h"
#include "EepromMap.h"

// ------------------------------------------------------------------------------------
void setup()
{
  pinMode(13, OUTPUT); //Set the pin to be OUTPUT

  Serial.begin(115200);

  EepromMapInitialise();
  TimerInitialise();
  TafelBeheerInit();
}

// ------------------------------------------------------------------------------------
void loop()
{
  TafelBeheerSM();
}
