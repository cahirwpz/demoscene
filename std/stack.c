#include "system/memory.h"
#include "std/stack.h"

StackT *NewStack(AllocFuncT allocFunc, FreeFuncT freeFunc) {
  StackT *stack = NEW_SZ(StackT);

  if (stack) {
    stack->used = NewSList();
    stack->free = NewSList();
    stack->allocFunc = allocFunc;
    stack->freeFunc = freeFunc;

    if (stack->used && stack->free)
      return stack;

    DeleteStack(stack);
  }

  return NULL;
}

void DeleteStack(StackT *stack) {
  if (stack) {
    DeleteSList(stack->used, stack->freeFunc);
    DeleteSList(stack->free, stack->freeFunc);
    DELETE(stack);
  }
}

void StackReset(StackT *stack) {
  SL_Concat(stack->free, stack->used);
}

void *StackGet(StackT *stack, size_t index) {
  return SL_GetNth(stack->used, index);
}

void *StackPush(StackT *stack) {
  SNodeT *link = SL_PopFrontNode(stack->free);

  if (link) {
    SL_PushFrontNode(stack->used, link);
    return SL_GetNth(stack->used, 0);
  }

  void *item;
 
  if ((item = stack->allocFunc())) {
    if (SL_PushFront(stack->used, item))
      return item;

    DELETE(item);
  }

  return NULL;
}
