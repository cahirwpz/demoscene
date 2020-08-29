#ifndef _CIA_H_
#define _CIA_H_

#include <cia_regdef.h>

typedef volatile struct CIA *const CIAPtrT;

extern struct CIA volatile _ciaa;
extern struct CIA volatile _ciab;

#define ciaa (&_ciaa)
#define ciab (&_ciab)

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
