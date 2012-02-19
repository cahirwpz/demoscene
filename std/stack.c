#include "std/memory.h"
#include "std/stack.h"

struct Stack {
  SListT *list;
  AtomPoolT *pool;
};

StackT *NewStack(AtomPoolT *pool) {
  StackT *stack = NEW_S(StackT);

  stack->list = NewSList();
  stack->pool = pool;

  StackPushNew(stack);

  return stack;
}

void DeleteStack(StackT *stack) {
  if (stack) {
    DeleteSList(stack->list);
    DeleteAtomPool(stack->pool);
    DELETE(stack);
  }
}

void StackReset(StackT *stack) {
  ResetSList(stack->list);
  ResetAtomPool(stack->pool);
}

void *StackPeek(StackT *stack, size_t index) {
  return SL_GetNth(stack->list, index);
}

void *StackTop(StackT *stack) {
  return SL_GetNth(stack->list, 0);
}

void *StackPushNew(StackT *stack) {
  void *item = AtomNew0(stack->pool);

  SL_PushFront(stack->list, item);

  return item;
}

size_t StackSize(StackT *stack) {
  return SL_Size(stack->list);
}
