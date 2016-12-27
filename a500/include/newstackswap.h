#ifndef __STACKSWAP_H__
#define __STACKSWAP_H__

#include <exec/tasks.h>

struct StackSwapArgs {
  LONG arg[8];
};

void NewStackSwap(struct StackSwapStruct *stack, APTR func, struct
                  StackSwapArgs *args);

#endif
