#ifndef __STACKSWAP_H__
#define __STACKSWAP_H__

#include <exec/tasks.h>

/* This is implementation of Exec StackSwap for OS 1.3 */
void _StackSwap(struct StackSwapStruct * asm("a0"));

#endif
