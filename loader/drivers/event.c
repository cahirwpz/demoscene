#include <task.h>
#include <event.h>

#define QUEUELEN 32

static EventT queue[QUEUELEN];
static u_short head, tail, used;

void PushEvent(EventT *event) {
  if (used < QUEUELEN) {
    queue[tail] = *event;
    tail = (tail + 1) & (QUEUELEN - 1);
    used++;
  }
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
