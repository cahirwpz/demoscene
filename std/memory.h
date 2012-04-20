#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include "std/types.h"

PtrT MemNew(size_t n);
PtrT MemNew0(size_t n);
PtrT MemDup(const void *p, size_t s);
char *StrDup(const char *s);
void MemFree(PtrT p);

#define NEW_S(TYPE) \
    (TYPE *)MemNew0(sizeof(TYPE))
#define NEW_A(TYPE, NUM) \
    (TYPE *)MemNew(sizeof(TYPE) * (NUM))

#define DELETE(PTR) MemFree(PTR)
#define DELETE_S(PTR, FUNC) { if (PTR) FUNC(PTR); MemFree(PTR); }

#endif
