#ifndef __STACKSWAP_H__
#define __STACKSWAP_H__

#include <exec/tasks.h>

struct StackSwapArgs {
  int arg[8];
};

void NewStackSwap(struct StackSwapStruct *stack, void *func,
                  struct StackSwapArgs *args);

#endif
