#ifndef _INTERRUPT_H_
#define _INTERRUPT_H_

#include <cdefs.h>

#define INTB_SETCLR 15  /* Set/Clear control bit. Determines if bits */
                        /* written with a one get set or cleared. Bits */
                        /* written with a zero are always unchanged */
#define INTB_INTEN 14   /* Master interrupt (enable only) */
#define INTB_EXTER 13   /* External interrupt */
#define INTB_DSKSYNC 12 /* Disk re-SYNChronized */
#define INTB_RBF 11     /* Serial port Receive Buffer Full */
#define INTB_AUD3 10    /* Audio channel 3 block finished */
#define INTB_AUD2 9     /* Audio channel 2 block finished */
#define INTB_AUD1 8     /* Audio channel 1 block finished */
#define INTB_AUD0 7     /* Audio channel 0 block finished */
#define INTB_BLIT 6     /* Blitter finished */
#define INTB_VERTB 5    /* Start of Vertical Blank */
#define INTB_COPER 4    /* Coprocessor */
#define INTB_PORTS 3    /* I/O Ports and timers */
#define INTB_SOFTINT 2  /* Software interrupt request */
#define INTB_DSKBLK 1   /* Disk Block done */
#define INTB_TBE 0      /* Serial port Transmit Buffer Empty */

#define INTF(x) __BIT(INTB_##x)

#define INTF_SETCLR INTF(SETCLR)
#define INTF_INTEN INTF(INTEN)
#define INTF_EXTER INTF(EXTER)
#define INTF_DSKSYNC INTF(DSKSYNC)
#define INTF_RBF INTF(RBF)
#define INTF_AUD3 INTF(AUD3)
#define INTF_AUD2 INTF(AUD2)
#define INTF_AUD1 INTF(AUD1)
#define INTF_AUD0 INTF(AUD0)
#define INTF_BLIT INTF(BLIT)
#define INTF_VERTB INTF(VERTB)
#define INTF_COPER INTF(COPER)
#define INTF_PORTS INTF(PORTS)
#define INTF_SOFTINT INTF(SOFTINT)
#define INTF_DSKBLK INTF(DSKBLK)
#define INTF_TBE INTF(TBE)

#define INTF_ALL 0x3FFF

#ifndef __ASSEMBLER__
#include <custom.h>
#include <types.h>

/* All macros below take or'ed INTF_* flags. */
static inline void EnableINT(u_short x) { custom->intena_ = INTF_SETCLR | x; }
static inline void DisableINT(u_short x) { custom->intena_ = x; }
static inline void CauseIRQ(u_short x) { custom->intreq_ = INTF_SETCLR | x; }
static inline void ClearIRQ(u_short x) { custom->intreq_ = x; }

void WaitIRQ(u_short x);

/* Enable / disable all interrupts. Handle nested calls. */
void IntrEnable(void);
void IntrDisable(void);

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
#ifndef SetIntVector
#define SetIntVector(INTR, CODE, DATA)                                         \
  IntVec[INTB_##INTR] = (IntVecEntryT){.code = (CODE), .data = (DATA)}
#endif
#define ResetIntVector(INTR) SetIntVector(INTR, DummyInterruptHandler, NULL)

/* Amiga Interrupt Autovector handlers */
extern void AmigaLvl1Handler(void);
extern void AmigaLvl2Handler(void);
extern void AmigaLvl3Handler(void);
extern void AmigaLvl4Handler(void);
extern void AmigaLvl5Handler(void);
extern void AmigaLvl6Handler(void);

typedef struct _Node { short prio;} NodeT;
typedef struct _List {} ListT;

/* Interrupt Server Handler Routine. */
typedef int (*IntFuncT)(void *);

typedef struct IntServer {
  NodeT node;
  IntFuncT code;
  void *data;
} IntServerT;

/* List of interrupt servers. */
typedef struct IntChain {
  ListT list;
  u_short flag; /* interrupt enable/disable flag (INTF_*) */
} IntChainT;

/* Define Interrupt Server to be used with (Add|Rem)IntServer.
 * Priority is between -128 (lowest) to 127 (highest).
 * Because servers are sorted by ascending number of priority,
 * IntServer definition recalculates priority number accordingly.
 */
#define _INTSERVER(PRI, CODE, DATA)                                            \
  {.node = { .prio = (PRI)}, .code = CODE, .data = (DATA)} 
#define INTSERVER(NAME, PRI, CODE, DATA)                                       \
  static IntServerT *NAME = &(IntServerT)_INTSERVER(PRI, CODE, DATA)

/* Defines Interrupt Chain of given name. */
#define INTCHAIN(NAME)                                                         \
  IntChainT *NAME = &(IntChainT) { .list = {}, .flag = 0 }

/* Initialize Interrupt Chain structure. */
#define InitIntChain(CHAIN, NUM)                                               \
  {                                                                            \
    /* INIT */                                                                 \
    (CHAIN)->flag = INTF(NUM);                                                 \
  }

/* Register Interrupt Server for given Interrupt Chain. */
#ifndef AddIntServer
void AddIntServer(IntChainT *ic, IntServerT *is);
#endif

/* Unregister Interrupt Server for given Interrupt Chain. */
#ifndef RemIntServer
void RemIntServer(IntChainT *ic, IntServerT *is);
#endif

/* Run Interrupt Servers for given Interrupt Chain.
 * Use only inside IntVec handler rountine. */
void RunIntChain(IntChainT *ic);

/* Predefined interrupt chains defined by Amiga port. */
extern IntChainT *PortsChain;
extern IntChainT *VertBlankChain;
extern IntChainT *ExterChain;

#endif

#endif /* !_INTERRUPT_H_ */
