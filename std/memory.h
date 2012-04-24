#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include "std/types.h"

/* Think twice before you use it. */
PtrT MemNewInternal(size_t n, uint32_t flags, FreeFuncT func);

PtrT MemNew(size_t n);
PtrT MemNew0(size_t n);
void MemFree(PtrT p);
PtrT RefInc(PtrT mem);
PtrT RefDec(PtrT mem);

PtrT MemDup(const void *p, size_t s);
StrT StrDup(const StrT s);

#define NEW_S(TYPE) \
    (TYPE *)MemNew0(sizeof(TYPE))
#define NEW_A(TYPE, NUM) \
    (TYPE *)MemNew(sizeof(TYPE) * (NUM))

#define DELETE(PTR) MemFree(PTR)
#define DELETE_S(PTR, FUNC) { if (PTR) FUNC(PTR); MemFree(PTR); }

#endif
