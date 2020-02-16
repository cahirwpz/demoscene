#ifndef __INFLATE_H__
#define __INFLATE_H__

void Inflate(const void *input asm("a0"), void *output asm("a1"));

#endif
