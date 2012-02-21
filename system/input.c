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

static __saveds APTR EventHandler(InputEventT *event asm("a0"),
                                  EventQueueT *queue asm("a1"))
{
  ObtainSemaphore(&queue->eventListLock);

  for (; event; event = event->ie_NextEvent) {
    InputEventT *copy = AtomNew(queue->eventPool);

    memcpy(copy, event, sizeof(InputEventT));

    ListPushBack(queue->eventList, copy);
  }

  ReleaseSemaphore(&queue->eventListLock);

  return NULL;
}

static EventQueueT *EventQueue = NULL;

void StartEventQueue() {
  if (!EventQueue) {
    EventQueueT *queue = NEW_S(EventQueueT);

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

  EventQueue = NULL;

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
    DELETE(queue);
  }
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
  bool result;

  ObtainSemaphore(&queue->eventListLock);

  {
    InputEventT *head = ListPopFront(queue->eventList);

    result = head ? TRUE : FALSE;

    if (result)
      memcpy(event, head, sizeof(InputEventT));

    AtomFree(queue->eventPool, head);
  }

  ReleaseSemaphore(&queue->eventListLock);

  return result;
}
