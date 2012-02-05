#ifndef __STD_STACK_H__
#define __STD_STACK_H__

#include "std/slist.h"

typedef struct Stack {
  SListT *used;
  SListT *free;

  AllocFuncT allocFunc;
  FreeFuncT freeFunc;
} StackT;

StackT *NewStack(AllocFuncT allocFunc, FreeFuncT freeFunc);
void DeleteStack(StackT *stack);

void StackReset(StackT *stack);
void *StackGet(StackT *stack, size_t index);
void *StackPush(StackT *stack);

#endif
