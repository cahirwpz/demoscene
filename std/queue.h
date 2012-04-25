#ifndef __STD_QUEUE_H__
#define __STD_QUEUE_H__

#include "std/types.h"

typedef struct Queue QueueT;

QueueT *NewQueue(size_t size, size_t elemSize);

void QueueReset(QueueT *self);
bool QueuePushBack(QueueT *self, PtrT data);
bool QueuePopFront(QueueT *self, PtrT data);

#endif
