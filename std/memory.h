#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include "std/types.h"

/* Think twice before you use it. */
PtrT MemNewInternal(size_t n, uint32_t flags, const TypeT *type);

PtrT MemNew(size_t n);
PtrT MemNewObject(const TypeT *type);
PtrT MemRef(PtrT mem);
PtrT MemUnref(PtrT mem);

PtrT MemDupGC(PtrT mem, size_t s);
PtrT MemDup(const void *p, size_t s);
StrT StrDup(const StrT s);

#define NewRecord(TYPE) \
    (TYPE *)MemNew(sizeof(TYPE))
#define NewInstance(TYPE) \
    (TYPE *)MemNewObject(&Type##TYPE)
#define NewTable(TYPE, NUM) \
    (TYPE *)MemNew(sizeof(TYPE) * (NUM))

#endif
