#include <Arduino.h>
#include "Timer.h"
#include "TafelBeheer.h"


// ------------------------------------------------------------------------------------
void setup()
{
  pinMode(13, OUTPUT); //Set the pin to be OUTPUT
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);

//   cli();  //stop interrupts for till we make the settings
//   /*1. First we reset the control register to amke sure we start with everything disabled.*/
//   TCCR1A = 0; // Reset entire TCCR1A to 0
//   TCCR1B = 0; // Reset entire TCCR1B to 0
//   /*2. We set the prescalar to the desired value by changing the CS10 CS12 and CS12 bits. */
//   TCCR1B |= B00000100; //Set CS12 to 1 so we get prescalar 256
//   /*3. We enable compare match mode on register A*/
//   TIMSK1 |= B00000010; //Set OCIE1A to 1 so we enable compare match A
//   /*4. Set the value of register A to 31250*/
//   OCR1A = 31250; //Finally we set compare register A to this value 500ms @256
// //  OCR1A = 62499; //Finally we set compare register A to this value 1s @256
//   sei();         //Enable back the interrupts
  Serial.begin(115200);

  TimerInitialise();
  TafelBeheerInit();
}

// ------------------------------------------------------------------------------------
void loop()
{
  TafelBeheerSM();
}

// ///////////////////////////////////////////////////////////////////////////////////////
// //// Cyclic timer
// ///////////////////////////////////////////////////////////////////////////////////////
// // ------------------------------------------------------------------------------------
// //With the settings above, this IRS will trigger each 500ms.
// ISR(TIMER1_COMPA_vect)
// {
//   TCNT1 = 0; //First, set the timer back to 0 so it resets for next interrupt
//   LED_STATE = !LED_STATE;      //Invert LED state
//   digitalWrite(13, LED_STATE);  //Write new state to the LED on pin D5
// //  ReadSensorsToShakTemperature = true; // Indicate the loop to capture
// }
