#ifndef _TIMER_H_
#define _TIMER_H_

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
typedef void (*CIATimeoutT)(CIATimerT *timer);

/* Procedures for handling one-shot delays with high resolution timers. */
CIATimerT *AcquireTimer(u_int num);
void ReleaseTimer(CIATimerT *timer);

void SetupTimer(CIATimerT *timer, CIATimeoutT timeout,
                u_short delay, u_short flags);

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

#endif
