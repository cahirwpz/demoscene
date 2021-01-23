#include <cpu.h>
#include <timer.h>
#include <debug.h>
#include <interrupt.h>
#include <task.h>

/* Bitmask marking TIMER_* timers being in use. */
static uint8_t InUse;

/* Defines timer state after it has been acquired. */
struct CIATimer {
  IntServerT server;
  CIATimeoutT timeout;
  CIAPtrT cia;
  u_int event;
  u_char icr;
  u_char num;
};

/* Interrupt handler for CIA timers. */
static void CIATimerHandler(CIATimerT *timer) {
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

CIATimerT *AcquireTimer(u_int num) {
  CIATimerT *timer = NULL;
  u_int i;

  Assert(num <= TIMER_CIAB_B || num == TIMER_ANY);

  IntrDisable();
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
  IntrEnable();

  if (timer) {
    IntChainT *chain = (num & 2) ? ExterChain : PortsChain;
    AddIntServer(chain, &timer->server);
  }

  return timer;
}

void ReleaseTimer(CIATimerT *timer) {
  IntrDisable();
  {
    IntChainT *chain = (timer->num & 2) ? ExterChain : PortsChain;
    RemIntServer(chain, &timer->server);
    InUse &= ~__BIT(timer->num);
  }
  IntrEnable();
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
    Assert(GetIPL() == 0);
    /* Turn on the interrupt and go to sleep. */
    IntrDisable();
    timer->timeout = NotifyTimeout;
    WriteICR(cia, CIAICRF_SETCLR | icr);
    TaskWait(timer->event);
    IntrEnable();
  }
}
