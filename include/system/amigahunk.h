#ifndef __AMIGAHUNK_H__
#define __AMIGAHUNK_H__

struct File;

typedef struct Hunk {
  u_int size;
  struct Hunk *next;
  u_char data[0];
} HunkT;

HunkT *LoadHunkList(struct File *file);
void FreeHunkList(HunkT *hunklist);

#endif
