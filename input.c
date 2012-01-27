#include <devices/input.h>
#include <exec/interrupts.h>
#include <exec/ports.h>

#include <proto/exec.h>
#include <clib/alib_protos.h>
#include <inline/exec_protos.h>

#include "common.h"
#include "input.h"

struct InputDev {
  struct MsgPort *MsgPort;
  struct IORequest *IoReq;
  struct Interrupt *IntHandler;
  BOOL Open;
};

static struct InputDev InputDev = { NULL, NULL, NULL, FALSE};

static APTR EventHandler() {
  return NULL;
}

BOOL InitEventHandler() {
  if (!InputDev.Open) {
    if ((InputDev.IntHandler = NEW_SZ(struct Interrupt))) {
      InputDev.IntHandler->is_Code = (APTR)EventHandler;
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
