#include <stdio.h>
#include <strings.h>
#include <serial.h>
#include <interrupt.h>
#include <custom.h>
#include <memory.h>

#define CLOCK 3546895
#define QUEUELEN 512

typedef struct {
  u_short head, tail;
  volatile u_short used;
  u_char data[QUEUELEN];
} CharQueueT;

struct File {
  FileOpsT *ops;
  CharQueueT sendq[1];
  CharQueueT recvq[1];
};

static void PushChar(CharQueueT *queue, u_char data) {
  while (queue->used == QUEUELEN)
    continue;

  DisableINT(INTF_TBE);
  {
    queue->data[queue->tail] = data;
    queue->tail = (queue->tail + 1) & (QUEUELEN - 1);
    queue->used++;
  }
  EnableINT(INTF_TBE);
}

static int PopChar(CharQueueT *queue) {
  int result;

  if (queue->used == 0)
    return -1;

  DisableINT(INTF_RBF);
  {
    result = queue->data[queue->head];
    queue->head = (queue->head + 1) & (QUEUELEN - 1);
    queue->used--;
  }
  EnableINT(INTF_RBF);

  return result;
}

static void SendIntHandler(CharQueueT *sendq) {
  int data;
  ClearIRQ(INTF_TBE);
  data = PopChar(sendq);
  if (data >= 0)
    custom->serdat = data | 0x100;
}

static void RecvIntHandler(CharQueueT *recvq) {
  u_short serdatr = custom->serdatr;
  ClearIRQ(INTF_RBF);
  PushChar(recvq, serdatr);
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

FileT *SerialOpen(u_int baud) {
  static FileT *f = NULL;

  IntrDisable();

  if (f == NULL) {
    f = MemAlloc(sizeof(FileT), MEMF_PUBLIC|MEMF_CLEAR);
    f->ops = &SerialOps;

    custom->serper = CLOCK / baud - 1;

    SetIntVector(TBE, (IntHandlerT)SendIntHandler, f->sendq);
    SetIntVector(RBF, (IntHandlerT)RecvIntHandler, f->recvq);

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
    PushChar(f->sendq, data);
    if (data == '\n')
      PushChar(f->sendq, '\r');
    DisableINT(INTF_TBE);
    if (custom->serdatr & SERDATF_TBE)
      SendIntHandler(f->sendq);
    EnableINT(INTF_TBE);
  }

  return nbyte;
}

static int SerialRead(FileT *f, void *_buf, u_int nbyte) {
  u_char *buf = _buf;
  u_int i = 0;

  while (i < nbyte) {
    buf[i] = PopChar(f->recvq);
    if (buf[i++] == '\n')
      break;
  }

  return i;
}
