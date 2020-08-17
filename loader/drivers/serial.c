#include <stdio.h>
#include <string.h>
#include <serial.h>
#include <interrupt.h>
#include <custom.h>

#define CLOCK 3546895
#define QUEUELEN 512

typedef struct {
  u_short head, tail;
  volatile u_short used;
  u_char data[QUEUELEN];
} CharQueueT;

static struct {
  CharQueueT sendq;
  CharQueueT recvq;
} serial;

static void PushChar(CharQueueT *queue, u_char data) {
  while (queue->used == QUEUELEN);

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

static void SendIntHandler(void) {
  int data;
  ClearIRQ(INTF_TBE);
  data = PopChar(&serial.sendq);
  if (data >= 0)
    custom->serdat = data | 0x100;
}

static void RecvIntHandler(void) {
  u_short serdatr = custom->serdatr;
  ClearIRQ(INTF_RBF);
  PushChar(&serial.recvq, serdatr);
}

void SerialInit(int baud) {
  memset(&serial, 0, sizeof(serial));

  custom->serper = CLOCK / baud - 1;

  SetIntVector(TBE, (IntHandlerT)SendIntHandler, NULL);
  SetIntVector(RBF, (IntHandlerT)RecvIntHandler, NULL);

  ClearIRQ(INTF_TBE | INTF_RBF);
  EnableINT(INTF_TBE | INTF_RBF);
}

void SerialKill() {
  DisableINT(INTF_TBE | INTF_RBF);
  ClearIRQ(INTF_TBE | INTF_RBF);

  ResetIntVector(RBF);
  ResetIntVector(TBE);
}

void SerialPut(u_char data) {
  PushChar(&serial.sendq, data);
  if (data == '\n')
    PushChar(&serial.sendq, '\r');
  DisableINT(INTF_TBE);
  if (custom->serdatr & SERDATF_TBE)
    SendIntHandler();
  EnableINT(INTF_TBE);
}

void SerialPrint(const char *format, ...) {
  va_list args;

  va_start(args, format);
  kvprintf(format, (kvprintf_fn_t *)SerialPut, NULL, args);
  va_end(args);
}

int SerialGet() {
  return PopChar(&serial.recvq);
}
