#include <devices/input.h>
#include <devices/inputevent.h>
#include <exec/interrupts.h>
#include <exec/ports.h>

#include <clib/alib_protos.h>
#include <proto/exec.h>
#include <inline/exec_protos.h>

#include "common.h"
#include "debug.h"
#include "input.h"

struct InputDev {
  struct MsgPort *MsgPort;
  struct IORequest *IoReq;
  struct Interrupt *IntHandler;
  BOOL Open;
};

static struct InputDev InputDev = { NULL, NULL, NULL, FALSE};

static __saveds APTR EventHandler(__reg("a0") struct InputEvent *event,
                                  __reg("a1") APTR data) {

  for (; event; event = event->ie_NextEvent) {
    switch (event->ie_Class) {
      case IECLASS_RAWKEY:
        LOG("Key %ld %s (%04lx)\n",
            (LONG)(event->ie_Code & ~IECODE_UP_PREFIX),
            (event->ie_Code & IECODE_UP_PREFIX) ? "up" : "down",
            (LONG)event->ie_Qualifier);
        break;

      case IECLASS_RAWMOUSE:
        if (event->ie_Code == IECODE_NOBUTTON) {
          LOG("Mouse move: (%ld,%ld)\n", (LONG)event->ie_X, (LONG)event->ie_Y);
        } else {
          const char *name[] = {"left", "right", "middle"};

          LOG("Mouse %s key %s.\n",
              name[(event->ie_Code & ~IECODE_UP_PREFIX) - IECODE_LBUTTON],
              (event->ie_Code & IECODE_UP_PREFIX) ? "up" : "down");
        }
        break;

      case IECLASS_TIMER:
        continue;

      default:
        break;
    }
  }

  return NULL;
}

BOOL InitEventHandler() {
  if (!InputDev.Open) {
    if ((InputDev.IntHandler = NEW_SZ(struct Interrupt))) {
      InputDev.IntHandler->is_Code = (APTR)EventHandler;
      InputDev.IntHandler->is_Data = NULL;
      InputDev.IntHandler->is_Node.ln_Pri = 100;

      if ((InputDev.MsgPort = CreatePort(NULL, 0))) {
        if ((InputDev.IoReq = CreateExtIO(InputDev.MsgPort,
                                          sizeof(struct IOStdReq)))) {
          InputDev.Open = (OpenDevice("input.device", 0L,
                                      InputDev.IoReq, 0) == 0);
        }
      }
    }

    if (InputDev.Open) {
      struct IOStdReq *ioReq = (struct IOStdReq *)InputDev.IoReq;

      ioReq->io_Data = (APTR)InputDev.IntHandler;
      ioReq->io_Command = IND_ADDHANDLER;
      DoIO((struct IORequest *)ioReq);
    } else {
      KillEventHandler();
    }
  }

  return InputDev.Open;
}

void KillEventHandler() {
  if (InputDev.Open) {
    struct IOStdReq *ioReq = (struct IOStdReq *)InputDev.IoReq;

    ioReq->io_Data = (APTR)InputDev.IntHandler;
    ioReq->io_Command = IND_REMHANDLER;
    DoIO((struct IORequest *)ioReq);

    /* Ask device to abort request, if pending */
    if (!CheckIO(InputDev.IoReq))
      AbortIO(InputDev.IoReq);
    /* Wait for abort, then clean up */
    WaitIO(InputDev.IoReq);
  }

  CloseDevice(InputDev.IoReq);
  DeleteExtIO(InputDev.IoReq);
  DeleteMsgPort(InputDev.MsgPort);
  DELETE(InputDev.IntHandler);

  InputDev.IoReq = NULL;
  InputDev.MsgPort = NULL;
  InputDev.IntHandler = NULL;
  InputDev.Open = FALSE;
}
