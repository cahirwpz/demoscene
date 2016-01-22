#ifndef __AMIGA_HUNK_H__
#define __AMIGA_HUNK_H__

#include "common.h"
#include "io.h"

typedef struct {
  LONG size;
  BPTR next;
  BYTE data[0];
} HunkT;

__regargs BPTR LoadExecutable(FileT *file);

#endif
