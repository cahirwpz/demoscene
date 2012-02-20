#ifndef __STD_STACK_H__
#define __STD_STACK_H__

#include "std/atompool.h"
#include "std/slist.h"

typedef struct Stack StackT;

StackT *NewStack(AtomPoolT *pool);
void DeleteStack(StackT *stack);

void StackReset(StackT *stack);
void StackRemove(StackT *stack);
void *StackPeek(StackT *stack, size_t index);
void *StackTop(StackT *stack);
void *StackPushNew(StackT *stack);
size_t StackSize(StackT *stack);

#endif
