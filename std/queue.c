#include "std/queue.h"
#include "std/memory.h"
#include "std/table.h"

struct Queue {
  int first;
  int last;
  PtrT data;
};

static void DeleteQueue(QueueT *self) {
  MemUnref(self->data);
}

TYPEDECL(QueueT, (FreeFuncT)DeleteQueue);

QueueT *NewQueue(size_t size, size_t elemSize) {
  QueueT *queue = NewInstance(QueueT);

  queue->data = MemNewTable(elemSize, size);

  return queue;
}

static inline int Next(QueueT *self, size_t index) {
  size_t n = TableSize(self->data);

  return (++index >= n) ? (index - n) : index;
}

static inline bool QueueIsFull(QueueT *self) {
  return (self->first == Next(self, self->last));
}

static inline bool QueueIsEmpty(QueueT *self) {
  return (self->first == self->last);
}

void QueueReset(QueueT *self) {
  self->first = 0;
  self->last = 0;
  memset(&self->data, 0, TableElemSize(self->data) * TableSize(self->data));
}

bool QueuePushBack(QueueT *self, PtrT data) {
  if (!QueueIsFull(self)) {
    PtrT *elem = TableElemGet(self->data, self->last);
    memcpy(elem, data, TableElemSize(self->data));
    self->last = Next(self, self->last);
    return TRUE;
  }

  return FALSE;
}

bool QueuePopFront(QueueT *self, PtrT data) {
  if (!QueueIsEmpty(self)) {
    PtrT *elem = TableElemGet(self->data, self->first);
    memcpy(data, elem, TableElemSize(self->data));
    memset(elem, 0, TableElemSize(self->data));
    self->first = Next(self, self->first);
    return TRUE;
  }

  return FALSE;
}
