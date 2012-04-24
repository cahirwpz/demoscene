#define NewList alib_NewList
#include <clib/alib_protos.h>
#undef NewList
#include <devices/input.h>
#include <exec/interrupts.h>
#include <exec/ports.h>
#include <proto/exec.h>

#include "std/atompool.h"
#include "std/debug.h"
#include "std/list.h"
#include "std/memory.h"

#include "system/input.h"

typedef struct IOStdReq IOStdReqT;
typedef struct IORequest IORequestT;
typedef struct Interrupt InterruptT;
typedef struct SignalSemaphore SemaphoreT;

typedef struct EventQueue {
  struct MsgPort *msgPort;
  struct IORequest *ioReq;
  InterruptT handler;
  SemaphoreT eventListLock;

  AtomPoolT *eventPool;
  ListT *eventList;
} EventQueueT;

/*
 * Strangely enough this mechanism is not reliable under heavy load.  Whereas
 * it seems to be perfectly reasonable, under certain condition (in
 * contradiction to common sense) it generates memory leaks. Maybe looking
 * through AROS sources would help to understand what really happens here.
 */

static __saveds APTR EventHandler(InputEventT *event asm("a0"),
                                  EventQueueT *queue asm("a1"))
{
  ObtainSemaphore(&queue->eventListLock);

  for (; event; event = event->ie_NextEvent) {
    InputEventT *copy;

    switch (event->ie_Class) {
      case IECLASS_RAWKEY:
      case IECLASS_RAWMOUSE:
        copy = AtomNew(queue->eventPool);
        memcpy(copy, event, sizeof(InputEventT));
        ListPushBack(queue->eventList, copy);
        break;

      default:
        break;
    }
  }

  ReleaseSemaphore(&queue->eventListLock);

  return NULL;
}

static EventQueueT *EventQueue = NULL;

void StartEventQueue() {
  if (!EventQueue) {
    EventQueueT *queue = NewRecord(EventQueueT);

    queue->eventPool = NewAtomPool(sizeof(InputEventT), 32);
    queue->eventList = NewList();

    InitSemaphore(&queue->eventListLock);

    queue->msgPort = CreatePort(NULL, 0);
    if (!queue->msgPort)
      PANIC("CreatePort(...) failed.");

    queue->ioReq = CreateExtIO(queue->msgPort, sizeof(IOStdReqT));
    if (!queue->ioReq)
      PANIC("CreateExtIO(...) failed.");

    if (OpenDevice("input.device", 0L, queue->ioReq, 0))
      PANIC("OpenDevice(...) failed.");
   
    {
      InterruptT *handler;
      IOStdReqT *ioReq;

      handler = &queue->handler;
      handler->is_Code = (APTR)EventHandler;
      handler->is_Data = (APTR)queue;
      handler->is_Node.ln_Pri = 100;

      ioReq = (IOStdReqT *)queue->ioReq;
      ioReq->io_Data = (APTR)&queue->handler;
      ioReq->io_Command = IND_ADDHANDLER;
      DoIO((IORequestT *)ioReq);
    }

    EventQueue = queue;
  }
}

void StopEventQueue() {
  EventQueueT *queue = EventQueue;

  if (queue) {
    IOStdReqT *ioReq;

    ioReq = (IOStdReqT *)queue->ioReq;
    ioReq->io_Data = (APTR)&queue->handler;
    ioReq->io_Command = IND_REMHANDLER;
    DoIO((IORequestT *)ioReq);

    /* Ask device to abort request, if pending */
    if (!CheckIO(queue->ioReq))
      AbortIO(queue->ioReq);
    /* Wait for abort, then clean up */
    WaitIO(queue->ioReq);

    CloseDevice(queue->ioReq);
    DeleteExtIO(queue->ioReq);
    DeleteMsgPort(queue->msgPort);

    DeleteAtomPool(queue->eventPool);
    DeleteList(queue->eventList);
    MemUnref(queue);
  }

  EventQueue = NULL;
}

void EventQueueReset() {
  EventQueueT *queue = EventQueue;

  ObtainSemaphore(&queue->eventListLock);

  ResetAtomPool(queue->eventPool);
  ResetList(queue->eventList);

  ReleaseSemaphore(&queue->eventListLock);
}

bool EventQueuePop(InputEventT *event) {
  EventQueueT *queue = EventQueue;
  InputEventT *head;

  ObtainSemaphore(&queue->eventListLock);

  head = ListPopFront(queue->eventList);

  if (head) {
    memcpy(event, head, sizeof(InputEventT));

    AtomFree(queue->eventPool, head);
  }

  ReleaseSemaphore(&queue->eventListLock);

  return BOOL(head);
}
