#include <strings.h>

#include "std/memory.h"
#include "std/stack.h"

struct Stack {
  int top;
  size_t size;
  size_t elemSize;
  uint8_t data[0];
};

StackT *NewStack(size_t size, size_t elemSize) {
  StackT *stack = MemNew(sizeof(StackT) + elemSize * size);

  stack->top = -1;
  stack->size = size;
  stack->elemSize = elemSize;

  return stack;
}

static inline PtrT StackGet(StackT *self, size_t index) {
  return self->data + (self->top - index) * self->elemSize;
}

void StackReset(StackT *self) {
  self->top = -1;
  bzero(&self->data, self->elemSize * self->size);
}

PtrT StackPeek(StackT *self, size_t index) {
  return (self->top >= index) ? StackGet(self, index) : NULL;
}

PtrT StackTop(StackT *self) {
  return StackPeek(self, 0);
}

PtrT StackPushNew(StackT *self) {
  if (self->top < (int)self->size) {
    self->top++;
    return StackGet(self, 0);
  }
  return NULL;
}

size_t StackSize(StackT *self) {
  return self->top + 1;
}
