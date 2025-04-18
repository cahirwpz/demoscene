#ifndef __SYSTEM_AMIGAHUNK_H__
#define __SYSTEM_AMIGAHUNK_H__

#include <types.h>

struct File;

typedef struct Hunk {
  u_int size;
  struct Hunk *next;
  u_char data[0];
} HunkT;

HunkT *LoadHunkList(struct File *file);
void FreeHunkList(HunkT *hunklist);
void SetupSharedHunks(HunkT *hunklist);

#endif /* !__SYSTEM_AMIGAHUNK_H__ */
