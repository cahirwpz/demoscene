#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include <proto/exec.h>
#include "std/types.h"

/*
 * Tables allocated by this interface have nice property of being cache line
 * (16 bytes) aligned. Thus we can make use of MOVE16 instruction for data
 * copying.
 */

/* Think twice before you use them. */
PtrT MemNewCustom(uint32_t size, const TypeT *type);
PtrT MemNewCustomTable(uint32_t size, uint32_t count, const TypeT *type);

/* Common functions. */
PtrT MemNew(uint32_t n);
PtrT MemNewOfType(const TypeT *type);
PtrT MemNewTable(uint32_t elemSize, uint32_t n);
PtrT MemNewTableOfType(const TypeT *type, uint32_t n);
void MemUnref(PtrT mem);

uint32_t TableElemSize(PtrT mem asm("a0"));
uint32_t TableSize(PtrT mem asm("a0"));
PtrT TableResize(PtrT mem, uint32_t count);

PtrT MemCopy(PtrT dst asm("a1"), const PtrT src asm("a0"), uint32_t n asm("d0"));
PtrT MemClone(PtrT mem);
PtrT MemDup(const void *p, uint32_t l);
char *StrDup(const char *s);
char *StrNDup(const char *s, uint32_t l);

#define NewRecord(TYPE) \
    (TYPE *)MemNew(sizeof(TYPE))
#define NewInstance(TYPE) \
    (TYPE *)MemNewOfType(&Type##TYPE)
#define NewTable(TYPE, NUM) \
    (TYPE *)MemNewTable(sizeof(TYPE), (NUM))
#define NewTableOfType(TYPE, NUM) \
    (TYPE *)MemNewTableOfType(&Type##TYPE, (NUM))

#endif
