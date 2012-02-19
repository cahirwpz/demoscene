#include "system/memory.h"
#include "std/stack.h"

static void StackAddItem(StackT *stack) {
  SL_PushFront(stack->used, stack->allocFunc());
}

StackT *NewStack(AllocFuncT allocFunc, FreeFuncT freeFunc) {
  StackT *stack = NEW_S(StackT);

  stack->used = NewSList();
  stack->free = NewSList();
  stack->allocFunc = allocFunc;
  stack->freeFunc = freeFunc;

  StackAddItem(stack);

  return stack;
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
  SL_PushFrontNode(stack->used, SL_PopFrontNode(stack->free));
}

void *StackGet(StackT *stack, size_t index) {
  return SL_GetNth(stack->used, index);
}

void *StackPush(StackT *stack) {
  void *item = SL_GetNth(stack->used, 0);

  SNodeT *link = SL_PopFrontNode(stack->free);

  if (link) {
    SL_PushFrontNode(stack->used, link);
  } else {
    StackAddItem(stack);
  }

  return item;
}
