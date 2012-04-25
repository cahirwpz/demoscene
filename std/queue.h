#ifndef __STD_QUEUE_H__
#define __STD_QUEUE_H__

#include "std/types.h"

typedef struct Queue QueueT;

/*
 * This is implementation of FIFO queue of limited size.  Space for all elements
 * is preallocated. 
 *
 * Elements are pushed to / popped from the queue by copying their contents.
 */
QueueT *NewQueue(size_t size, size_t elemSize);

void QueueReset(QueueT *self);
bool QueuePushBack(QueueT *self, PtrT data);
bool QueuePopFront(QueueT *self, PtrT data);

#endif
