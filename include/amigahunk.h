#ifndef __AMIGAHUNK_H__
#define __AMIGAHUNK_H__

#include "io.h"

typedef struct Hunk HunkT;

HunkT *LoadHunkList(FileT *file);
void FreeHunkList(HunkT *hunklist);

#endif /* !__AMIGAHUNK__ */
