#include <Arduino.h>
#include <EEPROM.h>
#include "Timer.h"
#include "TafelBeheer.h"
#include "EepromMap.h"

#define KANARI_VALUE  (0x18)

// ------------------------------------------------------------------------------------
void setup()
{
  pinMode(13, OUTPUT); //Set the pin to be OUTPUT
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);

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
