#ifndef __AMIGAHUNK_H__
#define __AMIGAHUNK_H__

#include <file.h>

typedef struct Hunk {
  u_int size;
  struct Hunk *next;
  u_char data[0];
} HunkT;

HunkT *LoadHunkList(FileT *file);
void FreeHunkList(HunkT *hunklist);

#endif
