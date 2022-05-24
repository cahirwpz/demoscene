#ifndef __SYSTEM_INTERRUPT_H__
#define __SYSTEM_INTERRUPT_H__

#include <cdefs.h>

#ifndef __ASSEMBLER__
#include <custom.h>
#include <types.h>

/* All macros below take or'ed INTF_* flags. */
static inline void EnableINT(u_short x) { custom->intena_ = INTF_SETCLR | x; }
static inline void DisableINT(u_short x) { custom->intena_ = x; }
static inline void CauseIRQ(u_short x) { custom->intreq_ = INTF_SETCLR | x; }
static inline void ClearIRQ(u_short x) { custom->intreq_ = x; }

/* Interrupt Handler Routine */
typedef void (*IntHandlerT)(void *);

#define ResetIntVector(irq) SetIntVector((irq), NULL, NULL);

/* Interrupt Server Handler Routine. */
typedef int (*IntFuncT)(void *);

typedef struct IntServer {
  struct IntServer *next;
  IntFuncT code;
  void *data;
  short prio;
} IntServerT;

/* Define Interrupt Server to be used with (Add|Rem)IntServer.
 * Priority is between -128 (lowest) to 127 (highest).
 * Because servers are sorted by ascending number of priority,
 * IntServer definition recalculates priority number accordingly.
 */
#define _INTSERVER(PRI, CODE, DATA)                                            \
  {.next = NULL, .code = CODE, .data = (DATA), .prio = (PRI)} 
#define INTSERVER(NAME, PRI, CODE, DATA)                                       \
  static IntServerT *NAME = &(IntServerT)_INTSERVER(PRI, CODE, DATA)

#ifdef _SYSTEM
void SetupInterruptVector(void);
#endif

#include <system/syscall.h>

/* Set up ISR for given interrupt number. */
SYSCALL3NR(SetIntVector, u_int, irq, d0, IntHandlerT, code, a0, void *, data, a1);

/* Register Interrupt Server for given Interrupt Chain. */
SYSCALL2NR(AddIntServer, u_int, irq, d0, IntServerT *, is, a0);

/* Unregister Interrupt Server for given Interrupt Chain. */
SYSCALL2NR(RemIntServer, u_int, irq, d0, IntServerT *, is, a0);
#endif

#endif /* !__SYSTEM_INTERRUPT_H__ */
