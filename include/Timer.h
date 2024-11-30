#ifndef TIMER_H_
#define TIMER_H_

/*--[ Include Files ]------------------------------------------------------------------------------------------------------------*/

/*--[ Literals ]-----------------------------------------------------------------------------------------------------------------*/

/*--[ Types ]--------------------------------------------------------------------------------------------------------------------*/
enum Timers
{
  TIME_KEYPRESS = 0,
  TIME_LED_FAST,
  TIME_LED_SLOW,
  TIME_MOTOR_STEP,
  TIME_SELECT,
  MAX_TIMERS
};

/*! Define the template for the Callback function */
typedef void (*pTimer2)(uint8_t);

/*--[ Prototypes ]---------------------------------------------------------------------------------------------------------------*/
void TimerInitialise();
void ConfigureTimer(enum Timers bt, float seconds);
void StartCount(enum Timers bt);
unsigned long WhatIsCount(enum Timers bt);
void Timer2Attach(pTimer2 callBack);
void Timer2Start();
void Timer2Stop();
void Timer2Ramp();

/*EOF============================================================================================================================*/
#endif