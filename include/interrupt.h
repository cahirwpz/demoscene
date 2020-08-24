#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <cdefs.h>

#ifndef __ASSEMBLER__
#include <custom.h>
#include <types.h>

void WaitIRQ(u_short x);

/* Interrupt Handler Routine */
typedef void (*IntHandlerT)(void *);

/* Interrupt Vector Entry */
typedef struct IntVecEntry {
  IntHandlerT code;
  void *data;
} IntVecEntryT;

typedef IntVecEntryT IntVecT[INTB_INTEN];

extern IntVecT IntVec;

/* Only returns from interrupt, without clearing pending flags. */
extern void DummyInterruptHandler(void *);

/* Macros for setting up ISR for given interrupt number. */
#define SetIntVector(INTR, CODE, DATA)                                         \
  IntVec[INTB_##INTR] = (IntVecEntryT){.code = (CODE), .data = (DATA)}
#define ResetIntVector(INTR) SetIntVector(INTR, DummyInterruptHandler, NULL)

/* Amiga Interrupt Autovector handlers */
extern void AmigaLvl1Handler(void);
extern void AmigaLvl2Handler(void);
extern void AmigaLvl3Handler(void);
extern void AmigaLvl4Handler(void);
extern void AmigaLvl5Handler(void);
extern void AmigaLvl6Handler(void);

/* Interrupt Server Handler Routine. */
typedef int (*IntFuncT)(void *);

typedef struct IntServer {
  struct IntServer *next;
  IntFuncT code;
  void *data;
  short prio;
} IntServerT;

/* List of interrupt servers. */
typedef struct IntChain {
  IntServerT *head;
  u_short flag; /* interrupt enable/disable flag (INTF_*) */
} IntChainT;

/* Define Interrupt Server to be used with (Add|Rem)IntServer.
 * Priority is between -128 (lowest) to 127 (highest).
 * Because servers are sorted by ascending number of priority,
 * IntServer definition recalculates priority number accordingly.
 */
#define _INTSERVER(PRI, CODE, DATA)                                            \
  {.next = NULL, .code = CODE, .data = (DATA), .prio = (PRI)} 
#define INTSERVER(NAME, PRI, CODE, DATA)                                       \
  static IntServerT *NAME = &(IntServerT)_INTSERVER(PRI, CODE, DATA)

/* Defines Interrupt Chain of given name. */
#define INTCHAIN(NAME, NUM)                                                    \
  IntChainT *NAME = &(IntChainT) { .head = NULL, .flag = INTF(NUM) }

/* Register Interrupt Server for given Interrupt Chain. */
void AddIntServer(IntChainT *ic, IntServerT *is);

/* Unregister Interrupt Server for given Interrupt Chain. */
void RemIntServer(IntChainT *ic, IntServerT *is);

/* Run Interrupt Servers for given Interrupt Chain.
 * Use only inside IntVec handler rountine. */
void RunIntChain(IntChainT *ic);

/* Predefined interrupt chains defined by Amiga port. */
extern IntChainT *PortsChain;
extern IntChainT *VertBlankChain;
extern IntChainT *ExterChain;

#endif

#endif
