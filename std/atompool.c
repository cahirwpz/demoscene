#include <strings.h>
#include <proto/exec.h>

#include "std/atompool.h"
#include "std/memory.h"
#include "std/debug.h"

struct AtomPool {
  PtrT pool;
  size_t atomSize;
  size_t perChunk;
};

static void DeleteAtomPool(AtomPoolT *self) {
  DeletePool(self->pool);
}

AtomPoolT *NewAtomPool(size_t atomSize, size_t perChunk) {
  AtomPoolT *atomPool = NewRecordGC(AtomPoolT, (FreeFuncT)DeleteAtomPool);

  atomPool->atomSize = atomSize;
  atomPool->perChunk = perChunk;

  ResetAtomPool(atomPool);

  return atomPool;
}

void ResetAtomPool(AtomPoolT *atomPool) {
  if (atomPool->pool)
    DeletePool(atomPool->pool);

  atomPool->pool = CreatePool(MEMF_PUBLIC,
                              atomPool->atomSize * atomPool->perChunk,
                              atomPool->atomSize);

  if (!atomPool->pool)
    PANIC("CreatePool(%ld, %ld) failed.",
          atomPool->atomSize * atomPool->perChunk, atomPool->atomSize);
}

PtrT AtomNew(AtomPoolT *atomPool) {
  PtrT ptr = AllocPooled(atomPool->pool, atomPool->atomSize);

  if (!ptr)
    PANIC("AllocPooled(%p, %ld) failed.", atomPool->pool, atomPool->atomSize);

  return ptr;
}

PtrT AtomNew0(AtomPoolT *atomPool) {
  PtrT ptr = AtomNew(atomPool);

  bzero(ptr, atomPool->atomSize);

  return ptr;
}

void AtomFree(AtomPoolT *atomPool, PtrT atom) {
  FreePooled(atomPool->pool, atom, atomPool->atomSize);
}
