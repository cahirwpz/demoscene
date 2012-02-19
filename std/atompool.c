#include <strings.h>
#include <proto/exec.h>

#include "std/atompool.h"
#include "std/memory.h"
#include "std/debug.h"

struct AtomPool {
  void *pool;
  size_t atomSize;
  size_t perChunk;
};

AtomPoolT *NewAtomPool(size_t atomSize, size_t perChunk) {
  AtomPoolT *atomPool = NEW_S(AtomPoolT);

  atomPool->atomSize = atomSize;
  atomPool->perChunk = perChunk;

  ResetAtomPool(atomPool);

  return atomPool;
}

void DeleteAtomPool(AtomPoolT *atomPool) {
  if (atomPool) {
    DeletePool(atomPool->pool);
    DELETE(atomPool);
  }
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

void *AtomNew(AtomPoolT *atomPool) {
  void *ptr = AllocPooled(atomPool->pool, atomPool->atomSize);

  if (!ptr)
    PANIC("AllocPooled(%p, %ld) failed.", atomPool->pool, atomPool->atomSize);

  return ptr;
}

void *AtomNew0(AtomPoolT *atomPool) {
  void *ptr = AtomNew(atomPool);

  bzero(ptr, atomPool->atomSize);

  return ptr;
}

void AtomFree(AtomPoolT *atomPool, void *atom) {
  FreePooled(atomPool->pool, atom, atomPool->atomSize);
}
