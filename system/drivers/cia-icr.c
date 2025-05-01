#include <system/cia.h>

static __code u_char _ICREnabled[2]; /* enabled interrupts mask */
static __code u_char _ICRPending[2]; /* pending interrupts mask */

/* If lower CIA address bit is set it's CIA-A, otherwise CIA-B. */
#define ICREnabled(cia) &_ICREnabled[(intptr_t)cia & 1]
#define ICRPending(cia) &_ICRPending[(intptr_t)cia & 1]

u_char WriteICR(CIAPtrT cia, u_char mask) {
  u_char *enabled;
  /* Write real ICR */
  cia->_ciaicr = mask;
  /* Read cached enabled interrupts bitmask. */
  enabled = ICREnabled(cia);
  /* Modify cached value accordingly to the mask. */
  if (mask & CIAICRF_SETCLR)
    *enabled |= mask & ~CIAICRF_SETCLR;
  else
    *enabled &= ~mask;
  /* Return enabled interrupts bitmask. */
  return *enabled;
}

u_char SampleICR(CIAPtrT cia, u_char mask) {
  /* Read cached pending interrupts bitmask. */
  u_char *pending = ICRPending(cia);
  /* Read real ICR and or its value into pending bitmask. */
  u_char icr = *pending | cia->_ciaicr;
  /* Clear bits masked by the user in cached bitmask. */
  *pending = icr & ~mask;
  /* Return only those bits that were requested by the user. */
  return icr & mask;
}
