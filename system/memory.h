#ifndef __SYSTEM_MEMORY_H__
#define __SYSTEM_MEMORY_H__

#include <std/types.h>

void *xmalloc(size_t n);
void *xzalloc(size_t n);
void *xmemdup(const void *p, size_t s);
char *xstrdup(const char *s);
void xfree(void *p);

#define NEW_S(TYPE) \
    (TYPE *)xzalloc(sizeof(TYPE))
#define NEW_A(TYPE, NUM) \
    (TYPE *)xmalloc(sizeof(TYPE) * (NUM))

#define DELETE(PTR) xfree(PTR)

#endif
