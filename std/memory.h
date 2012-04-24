#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include "std/types.h"

/* Think twice before you use it. */
PtrT MemNewInternal(size_t n, uint32_t flags, FreeFuncT func);

PtrT MemNew(size_t n);
PtrT MemNew0(size_t n);
PtrT MemRef(PtrT mem);
PtrT MemUnref(PtrT mem);

PtrT MemDup(const void *p, size_t s);
StrT StrDup(const StrT s);

#define NewRecord(TYPE) \
    (TYPE *)MemNew0(sizeof(TYPE))
#define NewTable(TYPE, NUM) \
    (TYPE *)MemNew(sizeof(TYPE) * (NUM))

#endif
