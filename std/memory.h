#ifndef __STD_MEMORY_H__
#define __STD_MEMORY_H__

#include "std/types.h"

void *MemNew(size_t n);
void *MemNew0(size_t n);
void *MemDup(const void *p, size_t s);
char *StrDup(const char *s);
void MemFree(void *p);

#define NEW_S(TYPE) \
    (TYPE *)MemNew0(sizeof(TYPE))
#define NEW_A(TYPE, NUM) \
    (TYPE *)MemNew(sizeof(TYPE) * (NUM))

#define DELETE(PTR) MemFree(PTR)

#endif
