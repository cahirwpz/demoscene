#include "std/memory.h"
#include "std/stack.h"
#include "std/table.h"

struct Stack {
  int top;
  PtrT data;
};

static void DeleteStack(StackT *self) {
  MemUnref(self->data);
}

TYPEDECL(StackT, (FreeFuncT)DeleteStack);

StackT *NewStack(size_t size, size_t elemSize) {
  StackT *stack = NewInstance(StackT);

  stack->data = MemNewTable(elemSize, size);
  stack->top = -1;

  return stack;
}

static inline PtrT StackGet(StackT *self, size_t index) {
  return TableElemGet(self->data, self->top - index);
}

void StackReset(StackT *self) {
  self->top = -1;
  memset(self->data, 0, TableSize(self->data) * TableElemSize(self->data));
}

PtrT StackPeek(StackT *self, size_t index) {
  return (self->top >= index) ? StackGet(self, index) : NULL;
}

PtrT StackTop(StackT *self) {
  return StackPeek(self, 0);
}

PtrT StackPushNew(StackT *self) {
  if (self->top < (int)TableSize(self->data)) {
    self->top++;
    return StackGet(self, 0);
  }
  return NULL;
}

size_t StackSize(StackT *self) {
  return self->top + 1;
}
