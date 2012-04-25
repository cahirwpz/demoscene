#include "std/queue.h"
#include "std/memory.h"

struct Queue {
  int first;
  int last;
  size_t size;
  size_t elemSize;
  uint8_t data[0];
};

QueueT *NewQueue(size_t size, size_t elemSize) {
  QueueT *queue = MemNew0(sizeof(QueueT) + elemSize * size, NULL);

  queue->size = size;
  queue->elemSize = elemSize;

  return queue;
}

static inline int Next(QueueT *self, size_t index) {
  index++;

  return (index >= self->size) ? (index - self->size) : index;
}

static inline PtrT QueueGet(QueueT *self, size_t index) {
  return self->data + index * self->elemSize;
}

static inline bool QueueIsFull(QueueT *self) {
  return (self->first == Next(self, self->last));
}

static inline bool QueueIsEmpty(QueueT *self) {
  return (self->first == self->last);
}

bool QueuePushBack(QueueT *self, PtrT data) {
  if (!QueueIsFull(self)) {
    PtrT *elem = QueueGet(self, self->last);
    memcpy(elem, data, self->elemSize);
    self->last = Next(self, self->last);
    return TRUE;
  }

  return FALSE;
}

bool QueuePopFront(QueueT *self, PtrT data) {
  if (!QueueIsEmpty(self)) {
    PtrT *elem = QueueGet(self, self->first);
    memcpy(data, elem, self->elemSize);
    self->first = Next(self, self->first);
    return TRUE;
  }

  return FALSE;
}
