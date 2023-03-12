#ifndef __INFLATE_H__
#define __INFLATE_H__

void Inflate(const void *input asm("a4"), void *output asm("a5"));

#endif
