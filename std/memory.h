#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include "std/types.h"

/* Think twice before you use them. */

PtrT MemNewCustom(size_t size, uint32_t flags, const TypeT *type);
PtrT MemNewCustomTable(size_t size, size_t count, uint32_t flags,
                       const TypeT *type);

PtrT MemNew(size_t n);
PtrT MemNewOfType(const TypeT *type);
PtrT MemNewTable(size_t elemSize, size_t n);
PtrT MemNewTableOfType(const TypeT *type, size_t n);

PtrT MemRef(PtrT mem);
PtrT MemUnref(PtrT mem);

PtrT TableResize(PtrT mem, size_t);
PtrT MemClone(PtrT mem);
PtrT MemDup(const void *p, size_t s);
StrT StrDup(const StrT s);

#define NewRecord(TYPE) \
    (TYPE *)MemNew(sizeof(TYPE))
#define NewInstance(TYPE) \
    (TYPE *)MemNewOfType(&Type##TYPE)
#define NewTable(TYPE, NUM) \
    (TYPE *)MemNewTable(sizeof(TYPE), (NUM))
#define NewTableOfType(TYPE, NUM) \
    (TYPE *)MemNewTableOfType(&Type##TYPE, (NUM))

#endif
