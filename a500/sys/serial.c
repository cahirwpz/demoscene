#include "serial.h"
#include "interrupts.h"
#include "hardware.h"

#define CLOCK 3546895
#define QUEUELEN 512

typedef struct {
  UWORD head, tail, used;
  UBYTE data[QUEUELEN];
} CharQueueT;

static struct {
  CharQueueT sendq;
  CharQueueT recvq;
} serial;

static inline BOOL PushChar(CharQueueT *queue, UBYTE data) {
  if (queue->used == QUEUELEN)
    return FALSE;

  queue->data[queue->tail] = data;
  queue->tail = (queue->tail + 1) & (QUEUELEN - 1);
  queue->used++;
  return TRUE;
}

static inline LONG PopChar(CharQueueT *queue) {
  LONG result;

  if (queue->used == 0)
    return -1;

  result = queue->data[queue->head];
  queue->head = (queue->head + 1) & (QUEUELEN - 1);
  queue->used--;
  return result;
}

static void SendIntHandler() {
  LONG data = PopChar(&serial.sendq);
  if (data >= 0)
    custom->serdat = data | 0x100;
  custom->intreq = INTF_TBE;
}

static void RecvIntHandler() {
  PushChar(&serial.recvq, custom->serdatr & 0xff);
  custom->intreq = INTF_RBF;
}

INTERRUPT(RecvInterrupt, 0, RecvIntHandler);
INTERRUPT(SendInterrupt, 0, SendIntHandler);

static struct Interrupt *oldTBE;
static struct Interrupt *oldRBF;

__regargs void SerialInit(LONG baud) {
  memset(&serial, 0, sizeof(serial));

  custom->serper = CLOCK / baud - 1;

  oldTBE = SetIntVector(INTB_TBE, &SendInterrupt);
  oldRBF = SetIntVector(INTB_RBF, &RecvInterrupt);

  custom->intena = INTF_SETCLR | INTF_TBE;
  custom->intena = INTF_SETCLR | INTF_RBF;
}

void SerialKill() {
  SetIntVector(INTB_RBF, oldRBF);
  SetIntVector(INTB_TBE, oldTBE);
}

#define TSRE 0x2000

__regargs void SerialPut(UBYTE data) {
  Disable();

  if (custom->serdatr & TSRE)
    custom->serdat = data | 0x100;
  else
    PushChar(&serial.sendq, data);

  if (data == '\n')
    PushChar(&serial.sendq, '\r');

  Enable();
}

LONG SerialGet() {
  LONG result;

  Disable();
  result = PopChar(&serial.recvq);
  Enable();

  return result;
}
