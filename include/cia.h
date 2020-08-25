#ifndef _CIA_H_
#define _CIA_H_

#include <cia_regdef.h>

typedef volatile struct CIA *const CIAPtrT;

extern struct CIA volatile _ciaa;
extern struct CIA volatile _ciab;

#define ciaa (&_ciaa)
#define ciab (&_ciab)

/* CIA timers resolution is E_CLOCK (in ticks per seconds). */
#define E_CLOCK 709379

/* Maximum delay is around 92.38ms */
#define TIMER_MS(ms) ((ms) * E_CLOCK / 1000)
#define TIMER_US(us) ((us) * E_CLOCK / (1000 * 1000))

/* Both CIAA & CIAB have Timer A & Timer B */
#define TIMER_ANY -1U
#define TIMER_CIAA_A 0
#define TIMER_CIAA_B 1
#define TIMER_CIAB_A 2
#define TIMER_CIAB_B 3

typedef struct CIATimer CIATimerT;

/* Procedures for handling one-shot delays with high resolution timers. */
CIATimerT *AcquireTimer(u_int num);
void ReleaseTimer(CIATimerT *timer);

/* Consider using wrapper macros below instead of this procedure. */
void WaitTimerGeneric(CIATimerT *timer, u_short ticks, bool spin);

/* Busy wait while waiting for timer to underflow.
 * Should wait no more than couple handred microseconds.
 * Interrupts are not disabled while spinning.
 * Use TIMER_MS/TIMER_US to convert time unit to timer ticks. */
#define WaitTimerSpin(TIMER, TICKS) WaitTimerGeneric(TIMER, TICKS, true)

/* Sleep while waiting for timer to underflow.
 * Use it if you want to wait for couple miliseconds or more.
 * Use TIMER_MS/TIMER_US to convert time unit to timer ticks. */
#define WaitTimerSleep(TIMER, TICKS) WaitTimerGeneric(TIMER, TICKS, false)

/* 24-bit frame counter offered by CIA A */
u_int ReadFrameCounter(void);
void SetFrameCounter(u_int frame);

/* 24-bit line counter offered by CIA B */
u_int ReadLineCounter(void);
void WriteLineCounter(u_int line);

/* You MUST use following procedures to access CIA Interrupt Control Register!
 * On read ICR provides pending interrupts bitmask clearing them as well.
 * On write ICR masks or unmasks interrupts. If writing 1 with CIAIRCF_SETCLR to
 * a bit that is enabled causes an interrupt. */

/* ICR: Enable, disable or cause interrupts. */
u_char WriteICR(CIAPtrT cia, u_char mask);

/* ICR: sample and clear pending interrupts. */
u_char SampleICR(CIAPtrT cia, u_char mask);

static inline bool LeftMouseButton(void) {
  return !(ciaa->ciapra & CIAF_GAMEPORT0);
}

#endif
