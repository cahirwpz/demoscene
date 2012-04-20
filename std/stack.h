#ifndef __STD_STACK_H__
#define __STD_STACK_H__

#include "std/atompool.h"

typedef struct Stack StackT;

StackT *NewStack(AtomPoolT *pool);
void DeleteStack(StackT *stack);

void StackReset(StackT *stack);
void StackRemove(StackT *stack);
PtrT StackPeek(StackT *stack, size_t index);
PtrT StackTop(StackT *stack);
PtrT StackPushNew(StackT *stack);
size_t StackSize(StackT *stack);

#endif
