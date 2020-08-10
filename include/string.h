#ifndef __STRING_H__
#define __STRING_H__

#include <types.h>

void *memset(void *b, int c, size_t len);
void *memcpy(void *restrict dst, const void *restrict src, size_t n);
char *strcpy(char *dst, const char *src);
int strcmp(const char *s1, const char *s2);
size_t strlen(const char *s);

#endif
