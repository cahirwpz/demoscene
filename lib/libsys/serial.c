#include <stdio.h>
#include <string.h>
#include "serial.h"
#include "interrupts.h"
#include "hardware.h"

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

  custom->intena = INTF_TBE;
  {
    queue->data[queue->tail] = data;
    queue->tail = (queue->tail + 1) & (QUEUELEN - 1);
    queue->used++;
  }
  custom->intena = INTF_SETCLR | INTF_TBE;
}

static int PopChar(CharQueueT *queue) {
  int result;

  if (queue->used == 0)
    return -1;

  custom->intena = INTF_RBF;
  {
    result = queue->data[queue->head];
    queue->head = (queue->head + 1) & (QUEUELEN - 1);
    queue->used--;
  }
  custom->intena = INTF_SETCLR | INTF_RBF;

  return result;
}

static void SendIntHandler(void) {
  int data;
  custom->intreq = INTF_TBE;
  data = PopChar(&serial.sendq);
  if (data >= 0)
    custom->serdat = data | 0x100;
}

static void RecvIntHandler(void) {
  u_short serdatr = custom->serdatr;
  custom->intreq = INTF_RBF;
  PushChar(&serial.recvq, serdatr);
}

INTERRUPT(RecvInterrupt, 0, RecvIntHandler, NULL);
INTERRUPT(SendInterrupt, 0, SendIntHandler, NULL);

static struct Interrupt *oldTBE;
static struct Interrupt *oldRBF;

void SerialInit(int baud) {
  memset(&serial, 0, sizeof(serial));

  custom->serper = CLOCK / baud - 1;

  oldTBE = SetIntVector(INTB_TBE, SendInterrupt);
  oldRBF = SetIntVector(INTB_RBF, RecvInterrupt);

  custom->intreq = INTF_TBE | INTF_RBF;
  custom->intena = INTF_SETCLR | INTF_TBE | INTF_RBF;
}

void SerialKill() {
  custom->intena = INTF_TBE | INTF_RBF;
  custom->intreq = INTF_TBE | INTF_RBF;

  SetIntVector(INTB_RBF, oldRBF);
  SetIntVector(INTB_TBE, oldTBE);
}

void SerialPut(u_char data) {
  PushChar(&serial.sendq, data);
  if (data == '\n')
    PushChar(&serial.sendq, '\r');
  custom->intena = INTF_TBE;
  if (custom->serdatr & SERDATR_TBE)
    SendIntHandler();
  custom->intena = INTF_SETCLR | INTF_TBE;
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
