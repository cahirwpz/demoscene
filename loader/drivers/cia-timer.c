#include <cia.h>

void WaitTimerA(CIAPtrT cia, u_short delay) {
  cia->ciacra |= CIACRAF_RUNMODE;
  WriteICR(cia, CIAICRF_TA);
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  while (!SampleICR(cia, CIAICRF_TA));
}

void WaitTimerB(CIAPtrT cia, u_short delay) {
  cia->ciacra |= CIACRBF_RUNMODE;
  WriteICR(cia, CIAICRF_TB);
  cia->ciatalo = delay;
  cia->ciatahi = delay >> 8;
  while (!SampleICR(cia, CIAICRF_TB));
}
