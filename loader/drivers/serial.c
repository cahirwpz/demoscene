#include <stdio.h>
#include <strings.h>
#include <file.h>
#include <debug.h>
#include <cpu.h>
#include <interrupt.h>
#include <task.h>
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

static void _PushChar(CharQueueT *queue, u_char data) {
  if (queue->used == QUEUELEN) {
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

static void PushCharISR(CharQueueT *queue, u_char data) {
  u_short ipl = SetIPL(SR_IM);
  _PushChar(queue, data);
  (void)SetIPL(ipl);
}

static void PushChar(CharQueueT *queue, u_char data) {
  IntrDisable();
  while (queue->used == QUEUELEN)
    TaskWait(INTF_TBE);
  _PushChar(queue, data);
  IntrEnable();
}

static int PopCharISR(CharQueueT *queue) {
  u_short ipl = SetIPL(SR_IM);
  int result = _PopChar(queue);
  (void)SetIPL(ipl);
  return result;
}

static int PopChar(CharQueueT *queue) {
  int result;
  IntrDisable();
  while ((result = _PopChar(queue)) < 0)
    TaskWait(INTF_RBF);
  IntrEnable();
  return result;
}

static void SendIntHandler(CharQueueT *sendq) {
  int data;
  ClearIRQ(INTF_TBE);
  data = PopCharISR(sendq);
  if (data >= 0)
    custom->serdat = data | 0x100;
}

static void RecvIntHandler(CharQueueT *recvq) {
  u_short ipl = SetIPL(SR_IM);
  u_short serdatr = custom->serdatr;
  ClearIRQ(INTF_RBF);
  PushCharISR(recvq, serdatr);
  (void)SetIPL(ipl);
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
    IntrDisable();
    if (custom->serdatr & SERDATF_TBE)
      CauseIRQ(SERDATF_TBE);
    IntrEnable();
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
