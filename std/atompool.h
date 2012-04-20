#ifndef __STD_ATOMPOOL_H__
#define __STD_ATOMPOOL_H__

#include "std/types.h"

typedef struct AtomPool AtomPoolT;

AtomPoolT *NewAtomPool(size_t atomSize, size_t perChunk);
void DeleteAtomPool(AtomPoolT *atomPool);
void ResetAtomPool(AtomPoolT *atomPool);

PtrT AtomNew(AtomPoolT *atomPool);
PtrT AtomNew0(AtomPoolT *atomPool);
void AtomFree(AtomPoolT *atomPool, PtrT atom);

#endif
