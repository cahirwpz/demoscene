#include <interrupt.h>
#include <task.h>
#include <debug.h>

void WaitIRQ(u_short mask) {
  while (!(custom->intreqr & mask));
  ClearIRQ(mask);
}

/* Predefined interrupt chains. */
INTCHAIN(PortsChain, PORTS);
INTCHAIN(VertBlankChain, VERTB);
INTCHAIN(ExterChain, EXTER);

void AddIntServer(IntChainT *ic, IntServerT *is) {
  IntServerT **is_p;
  IntrDisable();
  /* If the list is empty before insertion then enable the interrupt. */
  if (!ic->head) {
    ClearIRQ(ic->flag);
    EnableINT(ic->flag);
  }
  is_p = &ic->head;
  while (*is_p && (*is_p)->prio < is->prio)
    continue;
  is->next = *is_p;
  *is_p = is;
  IntrEnable();
}

void RemIntServer(IntChainT *ic, IntServerT *is) {
  IntServerT **is_p;
  IntrDisable();
  is_p = &ic->head;
  while (*is_p != is)
    is_p = &(*is_p)->next;
  Assert(is_p != NULL);
  *is_p = is->next;
  /* If the list is empty after removal then disable the interrupt. */
  if (!ic->head) {
    DisableINT(ic->flag);
    ClearIRQ(ic->flag);
  }
  IntrEnable();
}

void RunIntChain(IntChainT *ic) {
  /* Call each server in turn. */
  IntServerT *is = ic->head;
  do {
    is->code(is->data);
    is = is->next;
  } while (is);
}
