#include <debug.h>
#include <system/cpu.h>
#include <system/cia.h>
#include <system/interrupt.h>
#include <system/mutex.h>
#include <system/task.h>
#include <system/timer.h>

/* Bitmask marking TIMER_* timers being in use. */
static uint8_t InUse;

/* Defines timer state after it has been acquired. */
struct CIATimer {
  const char *name;
  IntServerT server;
  CIATimeoutT timeout;
  CIAPtrT cia;
  u_int event;
  u_char icr;
  u_char num;
};

/* Interrupt handler for CIA timers. */
static void CIATimerHandler(CIATimerT *timer) {
  Debug("Timeout on %s.", timer->name);
  if (SampleICR(timer->cia, timer->icr))
    timer->timeout(timer);
}

static void NotifyTimeout(CIATimerT *timer) {
  TaskNotifyISR(timer->event);
}

#define CIAA ciaa
#define CIAB ciab

#define TIMER(CIA, TIMER)                                                      \
  [TIMER_##CIA##_##TIMER] = {                                                  \
      .name = #CIA "-" #TIMER,                                                 \
      .server = _INTSERVER(0, (IntFuncT)CIATimerHandler,                       \
                           &Timers[TIMER_##CIA##_##TIMER]),                    \
      .timeout = NULL,                                                         \
      .cia = CIA,                                                              \
      .event = EVF_##CIAA##(CIAICRF_T##TIMER),                                 \
      .num = TIMER_##CIA##_##TIMER,                                            \
      .icr = CIAICRF_T##TIMER,                                                 \
  }

static CIATimerT Timers[4] = {TIMER(CIAA, A), TIMER(CIAA, B), TIMER(CIAB, A),
                              TIMER(CIAB, B)};
static MUTEX(TimerMtx);

CIATimerT *AcquireTimer(u_int num) {
  CIATimerT *timer = NULL;
  u_int i;

  Assume(num <= TIMER_CIAB_B || num == TIMER_ANY);

  MutexLock(&TimerMtx);
  /* Allocate a timer that is not in use and its' number matches
   * (if specified instead of wildcard). */
  for (i = TIMER_CIAA_A; i <= TIMER_CIAB_B; i++) {
    if (InUse & __BIT(i))
      continue;
    if (num == TIMER_ANY || num == i) {
      InUse |= __BIT(i);
      timer = &Timers[i];
      num = i;
      break;
    }
  }
  MutexUnlock(&TimerMtx);

  if (timer) {
    u_int irq = (num & 2) ? INTB_EXTER : INTB_PORTS;
    AddIntServer(irq, &timer->server);
  }

  return timer;
}

void ReleaseTimer(CIATimerT *timer) {
  MutexLock(&TimerMtx);
  {
    u_int irq = (timer->num & 2) ? INTB_EXTER : INTB_PORTS;
    RemIntServer(irq, &timer->server);
    InUse &= ~__BIT(timer->num);
  }
  MutexUnlock(&TimerMtx);
}

static void LoadTimer(CIAPtrT cia, u_char icr, u_short delay, u_short flags) {
  /* Load counter and start timer in one-shot mode. */
  if (icr == CIAICRF_TB) {
    cia->ciacrb |= CIACRF_LOAD;
    cia->ciatblo = delay;
    cia->ciatbhi = delay >> 8;
    cia->ciacrb |= flags | CIACRF_START;
  } else {
    cia->ciacra |= CIACRF_LOAD;
    cia->ciatalo = delay;
    cia->ciatahi = delay >> 8;
    cia->ciacra |= flags | CIACRF_START;
  }
}

void SetupTimer(CIATimerT *timer, CIATimeoutT timeout,
                u_short delay, u_short flags)
{
  CIAPtrT cia = timer->cia;
  u_char icr = timer->icr;
  timer->timeout = timeout;

  WriteICR(cia, icr);
  LoadTimer(cia, icr, delay, flags);
  WriteICR(cia, CIAICRF_SETCLR | icr);
}

void WaitTimerGeneric(CIATimerT *timer, u_short delay, bool spin) {
  CIAPtrT cia = timer->cia;
  u_char icr = timer->icr;

  Debug("%s for %d ticks on %s.", spin ? "Spin" : "Wait", delay, timer->name);

  /* Turn off interrupt while the timer is being set up. */
  WriteICR(cia, icr);
  LoadTimer(cia, icr, delay, CIACRF_RUNMODE);

  /* Busy wait for interrupt bit. */
  if (spin) {
    /* We were requested to busy wait. */
    while (!SampleICR(cia, icr))
      continue;
  } else {
    /* Must not sleep while in interrupt context! */
    Assume(GetIPL() == 0);
    /* Turn on the interrupt and go to sleep. */
    IntrDisable();
    timer->timeout = NotifyTimeout;
    WriteICR(cia, CIAICRF_SETCLR | icr);
    TaskWait(timer->event);
    IntrEnable();
  }
}
