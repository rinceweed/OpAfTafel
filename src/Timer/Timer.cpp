#include <Arduino.h>
#include "Timer.h"

//#define TIMER_RELOAD        (0xF424)
#define TIMER_RELOAD        (0x9c4)

#define TIME_MS             (10)

typedef struct Timing
{
  bool configured;
  unsigned long configuredRate;
  unsigned long currentcount;
  unsigned long configuredCount;
} TimingCounters;

static volatile TimingCounters TimerCounters[MAX_TIMERS];

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TimerInitialise()
{
  for (int i = 0; i < MAX_TIMERS; i++)
  {
    TimerCounters[i].configured = false;
  }

  /* Timer1 consists of two major registers TCCR1A and TCCR1B which control the timers where
      TCCR1A is responsible for PWM and
      TCCR1B is used to set the prescalar value.
      Set all the bits in the TCCR1A register to 0 as we will not be using it. */
  
  cli();  //stop interrupts for till we make the settings
  /*1. First we reset the control register to amke sure we start with everything disabled.*/
  TCCR1A = 0; // Reset entire TCCR1A to 0
  TCCR1B = 0; // Reset entire TCCR1B to 0
  /*2. We set the prescalar to the desired value by changing the CS10 CS12 and CS12 bits. */
  //Set CS10 and CS11 1 so we get prescalar 64
  //Set CS12 to 1 so we get prescalar 256
  //Set OCIE1A/WGM12 to 1 so we enable compare match A
  TCCR1B = (1<<WGM12) | (1<<CS11) | (1<<CS10);
  /*3. We enable compare match mode on register A*/
  TIMSK1 = (1<<OCIE1A); 
  /*4. Compare register A to this value to get 10ms*/
  OCR1A = TIMER_RELOAD;
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ConfigureTimer(enum Timers bt, float seconds)
{
  cli();
  TimerCounters[bt].configuredRate = (seconds * 1000) / TIME_MS;
  TimerCounters[bt].configuredCount = 0;
  TimerCounters[bt].currentcount = 0;
  TimerCounters[bt].configured = true;
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void StartCount(enum Timers bt)
{
  cli();
  TimerCounters[bt].configuredCount = 0;
  TimerCounters[bt].currentcount = 0;
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
unsigned long WhatIsCount(enum Timers bt)
{
  return TimerCounters[bt].currentcount;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
ISR(TIMER1_COMPA_vect)
{
  TCNT1 = 0; //First, set the timer back to 0 so it resets for next interrupt

  for (int i = 0; i < MAX_TIMERS; i++)
  {
    if (TimerCounters[i].configured)
    {
      TimerCounters[i].configuredCount++;
      if (TimerCounters[i].configuredCount > TimerCounters[i].configuredRate)
      {
        TimerCounters[i].configuredCount = 0;
        TimerCounters[i].currentcount++;
      }
    }
  }
}
