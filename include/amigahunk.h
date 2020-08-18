#ifndef __AMIGAHUNK_H__
#define __AMIGAHUNK_H__

#include <filesys.h>

typedef struct Hunk HunkT;

HunkT *LoadHunkList(FileT *file);
void FreeHunkList(HunkT *hunklist);

#endif
