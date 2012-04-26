#ifndef __STD_STACK_H__
#define __STD_STACK_H__

#include "std/types.h"

typedef struct Stack StackT;

StackT *NewStack(size_t size, size_t elemSize);

void StackReset(StackT *self);
PtrT StackPeek(StackT *self, size_t index);
PtrT StackTop(StackT *self);
PtrT StackPushNew(StackT *self);
size_t StackSize(StackT *self);

#endif
