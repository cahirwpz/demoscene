#ifndef __AMIGAHUNK_H__
#define __AMIGAHUNK_H__

#include "io.h"

typedef struct Hunk HunkT;

__regargs HunkT *LoadHunkList(FileT *file);
__regargs void FreeHunkList(HunkT *hunklist);

#endif /* !__AMIGAHUNK__ */
