#include "interrupts.h"
#include "event.h"

#define QUEUELEN 32

static EventT queue[QUEUELEN];
static UWORD head, tail, used;

__regargs void PushEvent(EventT *event) {
  if (used < QUEUELEN) {
    queue[tail] = *event;
    tail = (tail + 1) & (QUEUELEN - 1);
    used++;
  }
}

__regargs BOOL PopEvent(EventT *event) {
  BOOL present = FALSE;

  Disable();

  if (used > 0) {
    present = TRUE;
    *event = queue[head];
    head = (head + 1) & (QUEUELEN - 1);
    used--;
  }

  Enable();

  return present;
}
