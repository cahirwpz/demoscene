#include <clib/alib_protos.h>
#include <devices/timer.h>
#include <exec/ports.h>
#include <proto/exec.h>
#include <proto/timer.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/timer.h"

typedef struct {
  struct MsgPort *msgPort;
  struct IORequest *ioReq;
  int32_t freq;
  bool enabled;
} TimerT;

static TimerT TheTimer = { NULL, NULL, 0, FALSE };

bool SetupTimer() {
  if (TheTimer.enabled)
    return FALSE;

  TheTimer.msgPort = CreatePort(NULL, 0);
  if (!TheTimer.msgPort)
    PANIC("CreatePort(...) failed.");

  TheTimer.ioReq = CreateExtIO(TheTimer.msgPort, sizeof(struct timerequest));
  if (!TheTimer.ioReq)
    PANIC("CreateExtIO(...) failed.");

  if (OpenDevice(TIMERNAME, UNIT_ECLOCK, TheTimer.ioReq, 0))
    PANIC("OpenDevice(" TIMERNAME ", ...) failed.");

  {
    struct timerequest *tr = (struct timerequest *)TheTimer.ioReq;

    TimerBase = (struct Device *)tr->tr_node.io_Device;
  }

  {
    struct EClockVal clock;

    TheTimer.freq = ReadEClock(&clock);
  }

  TheTimer.enabled = TRUE;

  return TRUE;
}

void KillTimer() {
  if (!TheTimer.enabled)
    return;

  CloseDevice(TheTimer.ioReq);
  DeleteExtIO(TheTimer.ioReq);
  DeletePort(TheTimer.msgPort);

  TheTimer.enabled = FALSE;
}

uint64_t TimerRead() {
  struct EClockVal clock;

  (void)ReadEClock(&clock);

  return *(uint64_t *)&clock;
}

double TimerDiff(uint64_t start, uint64_t end) {
  int64_t diff = end - start;

  return (double)diff / TheTimer.freq;
}
