#include <Arduino.h>
#include "Timer.h"

//#define TIMER_RELOAD        (0xF424)
// #define TIMER_RELOAD          (0x9c4)
// #define TIME_MS               (10)

/*--[ Constants ]----------------------------------------------------------------------------------------------------------------*/
#define TIMER_RELOAD          (16000)
#define TIMER_100UsRELOAD     (24)

#define TIME_MS               (1)
#define TIME_US               (50)

#define TIMER2_PRELOAD_MAX    (199) /*prescale 64, 800us*/
#define TIMER2_PRELOAD_MIN    (49)  /*prescale 64, 200us*/
//#define TIMER2_PRELOAD_MIN    (34)  /*prescale 64, 200us*/ elke dan en wan stall hier
#define TIMER2_PRELOAD_REDUCE (15)

typedef struct Timing
{
  bool configured;
  unsigned long configuredRate;
  unsigned long currentcount;
  unsigned long configuredCount;
} TimingCounters;

/*--[ Data ]---------------------------------------------------------------------------------------------------------------------*/
static volatile TimingCounters TimerCounters[MAX_TIMERS];
static volatile pTimer2 Timer2CallBack;
static volatile int32_t StepWidth;

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void setupTimer1();
void setupTimer2();

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void TimerInitialise()
{
  for (int i = 0; i < MAX_TIMERS; i++)
  {
    TimerCounters[i].configured = false;
  }
  Timer2CallBack = nullptr;
  StepWidth = TIMER2_PRELOAD_MAX;
  setupTimer1();
  setupTimer2();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
// https://www.arduinoslovakia.eu/application/timer-calculator
void setupTimer1()
{
  cli();
  // Clear registers
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;

  // 20000 Hz (16000000/((99+1)*8))
  OCR1A = 99;
  // CTC
  TCCR1B |= (1 << WGM12);
  // Prescaler 8
  TCCR1B |= (1 << CS11);
  // Output Compare Match A Interrupt Enable
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
// https://www.arduinoslovakia.eu/application/timer-calculator
void setupTimer2()
{
  cli();
  // Clear registers
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2  = 0;

  // CTC
  TCCR2A |= (1 << WGM21);
  // Prescaler 64
  TCCR2B |= (1 << CS22);
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void ConfigureTimer(enum Timers bt, float seconds)
{
  cli();
  TimerCounters[bt].configuredRate = (seconds * 1000000) / TIME_US;
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

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void Timer2Attach(pTimer2 callBack)
{
  Timer2CallBack = callBack;
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void Timer2Start()
{
  cli();
  TCNT2 = 0;
  StepWidth = TIMER2_PRELOAD_MAX;
  OCR2A     = TIMER2_PRELOAD_MAX;
  // Output Compare Match A Interrupt Enable
  TIMSK2 |= (1 << OCIE2A);
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void Timer2Stop()
{
  cli();
  // Output Compare Match A Interrupt Disable
  TIMSK2 &= (~(1 << OCIE2A));
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
void Timer2Ramp()
{
  cli();
  StepWidth = (StepWidth <= TIMER2_PRELOAD_MIN) ? TIMER2_PRELOAD_MIN : (StepWidth - TIMER2_PRELOAD_REDUCE);
  sei();
}

/*--[ Function ]-----------------------------------------------------------------------------------------------------------------*/
ISR(TIMER2_COMPA_vect)
{
  // Change the compare to speed up the interrupt
  OCR2A = StepWidth;
  if (Timer2CallBack != nullptr)
  {
    Timer2CallBack();
  }
}
