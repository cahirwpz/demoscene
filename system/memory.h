#ifndef __SYSTEM_MEMORY_H__
#define __SYSTEM_MEMORY_H__

#include <std/types.h>

void *g_new(size_t n);
void *g_new0(size_t n);
void *g_memdup(const void *p, size_t s);
char *g_strdup(const char *s);
void g_free(void *p);

#define NEW_S(TYPE) \
    (TYPE *)g_new0(sizeof(TYPE))
#define NEW_A(TYPE, NUM) \
    (TYPE *)g_new(sizeof(TYPE) * (NUM))

#define DELETE(PTR) g_free(PTR)

#endif
