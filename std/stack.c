#include "std/list.h"
#include "std/memory.h"
#include "std/stack.h"

struct Stack {
  ListT *list;
  AtomPoolT *pool;
};

StackT *NewStack(AtomPoolT *pool) {
  StackT *stack = NEW_S(StackT);

  stack->list = NewList();
  stack->pool = pool;

  StackPushNew(stack);

  return stack;
}

void DeleteStack(StackT *stack) {
  if (stack) {
    DeleteList(stack->list);
    DeleteAtomPool(stack->pool);
    DELETE(stack);
  }
}

void StackReset(StackT *stack) {
  ResetList(stack->list);
  ResetAtomPool(stack->pool);
}

void StackRemove(StackT *stack) {
  AtomFree(stack->pool, ListPopFront(stack->list));
}

void *StackPeek(StackT *stack, size_t index) {
  return ListGetNth(stack->list, index);
}

void *StackTop(StackT *stack) {
  return ListGetNth(stack->list, 0);
}

void *StackPushNew(StackT *stack) {
  void *item = AtomNew0(stack->pool);

  ListPushFront(stack->list, item);

  return item;
}

size_t StackSize(StackT *stack) {
  return ListSize(stack->list);
}
