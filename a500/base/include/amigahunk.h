#ifndef __AMIGA_HUNK_H__
#define __AMIGA_HUNK_H__

#include "common.h"
#include "io.h"

typedef struct {
  int size;
  BPTR next;
  char data[0];
} HunkT;

__regargs BPTR LoadExecutable(FileT *file);
__regargs void FreeSegList(BPTR seglist);

#endif
