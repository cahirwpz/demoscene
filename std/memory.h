#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include <proto/exec.h>
#include "std/types.h"

/* Think twice before you use them. */
PtrT MemNewCustom(size_t size, const TypeT *type);
PtrT MemNewCustomTable(size_t size, size_t count, const TypeT *type);

/* Common functions. */
PtrT MemNew(size_t n);
PtrT MemNewOfType(const TypeT *type);
PtrT MemNewTable(size_t elemSize, size_t n);
PtrT MemNewTableOfType(const TypeT *type, size_t n);
PtrT MemUnref(PtrT mem);

size_t TableElemSize(PtrT mem asm("a0"));
size_t TableSize(PtrT mem asm("a0"));
PtrT TableResize(PtrT mem, size_t count);

PtrT MemCopy(PtrT dst asm("a1"), const PtrT src asm("a0"), size_t n asm("d0"));
PtrT MemClone(PtrT mem);
PtrT MemDup(const void *p, size_t l);
char *StrDup(const char *s);
char *StrNDup(const char *s, size_t l);

#define NewRecord(TYPE) \
    (TYPE *)MemNew(sizeof(TYPE))
#define NewInstance(TYPE) \
    (TYPE *)MemNewOfType(&Type##TYPE)
#define NewTable(TYPE, NUM) \
    (TYPE *)MemNewTable(sizeof(TYPE), (NUM))
#define NewTableOfType(TYPE, NUM) \
    (TYPE *)MemNewTableOfType(&Type##TYPE, (NUM))

#endif
