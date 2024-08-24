#include <debug.h>
#include <system/cpu.h>
#include <system/exception.h>
#include <system/interrupt.h>
#include <system/task.h>

/* Interrupt Vector Entry */
typedef struct IntVecEntry {
  IntHandlerT code;
  void *data;
} IntVecEntryT;

typedef IntVecEntryT IntVecT[INTB_INTEN];

/* Amiga autovector interrupts table. */
IntVecT IntVec;

/* Only returns from interrupt, without clearing pending flags. */
extern void DummyInterruptHandler(void *);

/* Set up ISR for given interrupt number. */
void SetIntVector(u_int irq, IntHandlerT code, void *data) {
  IntVecEntryT *iv = &IntVec[irq];
  iv->code = code ? code : DummyInterruptHandler;
  iv->data = data;
}

#if MULTITASK
#define IntrNest CurrentTask->intrNest
#else
static __code short IntrNest = 0;
#endif

void IntrEnable(void) {
  Assume(IntrNest > 0);
  if (--IntrNest == 0)
    CpuIntrEnable();
}

void IntrDisable(void) {
  CpuIntrDisable();
  IntrNest++;
}

/* List of interrupt servers. */
typedef struct IntChain {
  IntServerT *head;
  u_short flag; /* interrupt enable/disable flag (INTF_*) */
} IntChainT;

/* Defines Interrupt Chain of given name. */
#define INTCHAIN(NAME, NUM)                                                    \
  IntChainT *NAME = &(IntChainT) { .head = NULL, .flag = INTF(NUM) }

/* Predefined interrupt chains. */
static INTCHAIN(PortsChain, PORTS);
static INTCHAIN(VertBlankChain, VERTB);
static INTCHAIN(ExterChain, EXTER);

static IntChainT *GetIntChain(u_int irq) {
  if (irq == INTB_VERTB)
    return VertBlankChain;
  if (irq == INTB_PORTS)
    return PortsChain;
  if (irq == INTB_EXTER)
    return ExterChain;
  PANIC();
  return NULL;
}

void AddIntServer(u_int irq, IntServerT *is) {
  IntChainT *ic = GetIntChain(irq);
  IntServerT **is_p;
  IntrDisable();
  /* If the list is empty before insertion then enable the interrupt. */
  if (!ic->head) {
    ClearIRQ(ic->flag);
    EnableINT(ic->flag);
  }
  is_p = &ic->head;
  while (*is_p && (*is_p)->prio < is->prio)
    is_p = &(*is_p)->next;
  is->next = *is_p;
  *is_p = is;
  IntrEnable();
}

void RemIntServer(u_int irq, IntServerT *is) {
  IntChainT *ic = GetIntChain(irq);
  IntServerT **is_p;
  IntrDisable();
  is_p = &ic->head;
  while (*is_p != is)
    is_p = &(*is_p)->next;
  Assume(is_p != NULL);
  *is_p = is->next;
  /* If the list is empty after removal then disable the interrupt. */
  if (!ic->head) {
    DisableINT(ic->flag);
    ClearIRQ(ic->flag);
  }
  IntrEnable();
}

/* Run Interrupt Servers for given Interrupt Chain.
 * Use only inside IntVec handler rountine. */
static void RunIntChain(IntChainT *ic) {
  /* Call each server in turn. */
  IntServerT *is = ic->head;
  ClearIRQ(ic->flag);
  do {
    is->code(is->data);
    is = is->next;
  } while (is);
}

/* Amiga Interrupt Autovector handlers */
extern void AmigaLvl1Handler(void);
extern void AmigaLvl2Handler(void);
extern void AmigaLvl3Handler(void);
extern void AmigaLvl4Handler(void);
extern void AmigaLvl5Handler(void);
extern void AmigaLvl6Handler(void);

void SetupInterruptVector(void) {
  int i;

  /* Initialize level 1-7 interrupt autovector in Amiga specific way. */
  ExcVec[EXC_INTLVL(1)] = AmigaLvl1Handler;
  ExcVec[EXC_INTLVL(2)] = AmigaLvl2Handler;
  ExcVec[EXC_INTLVL(3)] = AmigaLvl3Handler;
  ExcVec[EXC_INTLVL(4)] = AmigaLvl4Handler;
  ExcVec[EXC_INTLVL(5)] = AmigaLvl5Handler;
  ExcVec[EXC_INTLVL(6)] = AmigaLvl6Handler;

  for (i = INTB_TBE; i <= INTB_EXTER; i++)
    SetIntVector(i, NULL, NULL);

  /* Initialize PORTS & VERTB & EXTER as interrupt server chain. */
  SetIntVector(INTB_PORTS, (IntHandlerT)RunIntChain, PortsChain);
  SetIntVector(INTB_VERTB, (IntHandlerT)RunIntChain, VertBlankChain);
  SetIntVector(INTB_EXTER, (IntHandlerT)RunIntChain, ExterChain);
}
