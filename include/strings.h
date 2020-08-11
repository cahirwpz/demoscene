#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <types.h>

void bcopy(const void *src asm("a0"), void *dst asm("a1"),
           size_t len asm("d1"));
void bzero(void *s asm("a0"), size_t n asm("d1"));

#endif
