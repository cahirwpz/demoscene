#define NewList alib_NewList
#include <clib/alib_protos.h>
#undef NewList
#include <devices/input.h>
#include <exec/interrupts.h>
#include <exec/ports.h>
#include <proto/exec.h>

#include "std/debug.h"
#include "std/queue.h"
#include "std/memory.h"

#include "system/input.h"

typedef struct IOStdReq IOStdReqT;
typedef struct IORequest IORequestT;
typedef struct Interrupt InterruptT;
typedef struct SignalSemaphore SemaphoreT;

#define MAX_EVENTS 32

typedef struct EventQueue {
  struct MsgPort *msgPort;
  struct IORequest *ioReq;
  InterruptT handler;
  SemaphoreT lock;
  QueueT *events;
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
  ObtainSemaphore(&queue->lock);

  for (; event; event = event->ie_NextEvent) {
    switch (event->ie_Class) {
      case IECLASS_RAWKEY:
      case IECLASS_RAWMOUSE:
        QueuePushBack(queue->events, event);
        break;

      default:
        break;
    }
  }

  ReleaseSemaphore(&queue->lock);

  return NULL;
}

static EventQueueT *EventQueue = NULL;

static void DeleteEventQueue(EventQueueT *queue) {
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

  MemUnref(queue->events);
}

static EventQueueT *NewEventQueue(size_t size) {
  EventQueueT *queue = NewRecordGC(EventQueueT, (FreeFuncT)DeleteEventQueue);

  queue->events = NewQueue(size, sizeof(InputEventT));

  InitSemaphore(&queue->lock);

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

  return queue;
}

void StartEventQueue() {
  if (!EventQueue)
    EventQueue = NewEventQueue(MAX_EVENTS);
}

void StopEventQueue() {
  if (EventQueue) {
    MemUnref(EventQueue);
    EventQueue = NULL;
  }
}

void EventQueueReset() {
  ObtainSemaphore(&EventQueue->lock);
  QueueReset(EventQueue->events);
  ReleaseSemaphore(&EventQueue->lock);
}

bool EventQueuePop(InputEventT *event) {
  bool nonEmpty;

  ObtainSemaphore(&EventQueue->lock);
  nonEmpty = QueuePopFront(EventQueue->events, event);
  ReleaseSemaphore(&EventQueue->lock);

  return nonEmpty;
}
