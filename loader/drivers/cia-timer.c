#include <cia.h>

void WaitTimerA(CIAPtrT cia, u_short delay) {
  /* Turn off interrupt while the timer is being set up. */
  WriteICR(cia, CIAICRF_TA);

  /* Load counter and start timer in one-shot mode. */
  cia->ciacra |= CIACRAF_LOAD;
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  cia->ciacra |= CIACRAF_RUNMODE | CIACRAF_START;

  /* Busy wait for interrupt bit. */
  while (!SampleICR(cia, CIAICRF_TA))
    continue;
}

void WaitTimerB(CIAPtrT cia, u_short delay) {
  /* Turn off interrupt while the timer is being set up. */
  WriteICR(cia, CIAICRF_TB);

  /* Load counter and start timer in one-shot mode. */
  cia->ciacrb |= CIACRBF_LOAD;
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  cia->ciacrb |= CIACRBF_RUNMODE | CIACRBF_START;

  /* Busy wait for interrupt bit. */
  while (!SampleICR(cia, CIAICRF_TB))
    continue;
}
