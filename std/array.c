#include "std/memory.h"
#include "std/array.h"

struct _Array {
  PtrT data;
  size_t size;
  size_t reserved;
  size_t elemSize;
  FreeFuncT freeFunc;
};
