#include <cpu.h>
#include <task.h>
#include <debug.h>
#include <event.h>

#define QUEUELEN 32

static EventT queue[QUEUELEN];
static u_short head, tail, used;

static void _PushEvent(EventT *event) {
  if (used < QUEUELEN) {
    queue[tail] = *event;
    tail = (tail + 1) & (QUEUELEN - 1);
    used++;
  } else {
    Log("[Event] Queue full, dropping event!\n");
  }
}

void PushEventISR(EventT *event) {
  u_short ipl = SetIPL(SR_IM);
  _PushEvent(event);
  (void)SetIPL(ipl);
}

void PushEvent(EventT *event) {
  IntrDisable();
  _PushEvent(event);
  IntrEnable();
}

bool PopEvent(EventT *event) {
  bool present = false;

  IntrDisable();

  if (used > 0) {
    present = true;
    *event = queue[head];
    head = (head + 1) & (QUEUELEN - 1);
    used--;
  }

  IntrEnable();

  return present;
}
