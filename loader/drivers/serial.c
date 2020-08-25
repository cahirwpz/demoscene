#include <stdio.h>
#include <strings.h>
#include <file.h>
#include <debug.h>
#include <interrupt.h>
#include <task.h>
#include <custom.h>
#include <memory.h>

#define CLOCK 3546895
#define QUEUELEN 80

typedef struct {
  u_short head, tail;
  volatile u_short used;
  u_char data[QUEUELEN];
} CharQueueT;

struct File {
  FileOpsT *ops;
  u_int flags;
  CharQueueT sendq[1];
  CharQueueT recvq[1];
};

static void _PushChar(CharQueueT *queue, u_char data) {
  if (queue->used < QUEUELEN) {
    queue->data[queue->tail] = data;
    queue->tail = (queue->tail + 1) & (QUEUELEN - 1);
    queue->used++;
  } else {
    Log("[Serial] Queue full, dropping character!\n");
  }
}

static int _PopChar(CharQueueT *queue) {
  int result = -1;
  if (queue->used > 0) {
    result = queue->data[queue->head];
    queue->head = (queue->head + 1) & (QUEUELEN - 1);
    queue->used--;
  }
  return result;
}

static void PushChar(CharQueueT *queue, u_char data, u_int flags) {
  /* If send queue and serdat register are empty,
   * then push out first character directly. */
  IntrDisable();
  if (queue->used == 0 && custom->serdatr & SERDATF_TBE) {
    custom->serdat = data | 0x100;
  } else {
    while (queue->used == QUEUELEN && !(flags & O_NONBLOCK))
      TaskWait(INTF_TBE);
    _PushChar(queue, data);
  }
  IntrEnable();
}

static int PopChar(CharQueueT *queue, u_int flags) {
  int result;
  IntrDisable();
  for (;;) {
    result = _PopChar(queue);
    if (result >= 0 || flags & O_NONBLOCK)
      break;
    TaskWait(INTF_RBF);
  }
  IntrEnable();
  return result;
}

static void SendIntHandler(FileT *f) {
  CharQueueT *sendq = f->sendq;
  int data;
  ClearIRQ(INTF_TBE);
  data = _PopChar(sendq);
  if (data >= 0) {
    custom->serdat = data | 0x100;
    if (!(f->flags & O_NONBLOCK))
      TaskNotifyISR(INTF_TBE);
  }
}

static void RecvIntHandler(FileT *f) {
  CharQueueT *recvq = f->recvq;
  u_short code = custom->serdatr;
  ClearIRQ(INTF_RBF);
  if (code & SERDATF_RBF) {
    _PushChar(recvq, code);
    if (!(f->flags & O_NONBLOCK))
      TaskNotifyISR(INTF_RBF);
  }
}

static int SerialRead(FileT *f, void *buf, u_int nbyte);
static int SerialWrite(FileT *f, const void *buf, u_int nbyte);
static void SerialClose(FileT *f);

static FileOpsT SerialOps = {
  .read = SerialRead,
  .write = SerialWrite,
  .seek = NULL,
  .close = SerialClose
};

FileT *SerialOpen(u_int baud, u_int flags) {
  static FileT *f = NULL;

  IntrDisable();

  if (f == NULL) {
    f = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);
    f->ops = &SerialOps;
    f->flags = flags;

    custom->serper = CLOCK / baud - 1;

    SetIntVector(TBE, (IntHandlerT)SendIntHandler, f);
    SetIntVector(RBF, (IntHandlerT)RecvIntHandler, f);

    ClearIRQ(INTF_TBE | INTF_RBF);
    EnableINT(INTF_TBE | INTF_RBF);
  }

  IntrEnable();

  return f;
}

static void SerialClose(FileT *f) {
  DisableINT(INTF_TBE | INTF_RBF);
  ClearIRQ(INTF_TBE | INTF_RBF);

  ResetIntVector(RBF);
  ResetIntVector(TBE);

  MemFree(f);
}

static int SerialWrite(FileT *f, const void *_buf, u_int nbyte) {
  const u_char *buf = _buf;
  u_int i;

  for (i = 0; i < nbyte; i++) {
    u_char data = *buf++;
    PushChar(f->sendq, data, f->flags);
    if (data == '\n')
      PushChar(f->sendq, '\r', f->flags);
  }

  return nbyte;
}

static int SerialRead(FileT *f, void *_buf, u_int nbyte) {
  u_char *buf = _buf;
  u_int i = 0;

  while (i < nbyte) {
    buf[i] = PopChar(f->recvq, f->flags);
    if (buf[i++] == '\n')
      break;
  }

  return i;
}
