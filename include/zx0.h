#ifndef __ZX0_H__
#define __ZX0_H__

void zx0_decompress(const void *input asm("a0"), void *output asm("a1"));

#endif /* !__ZX0_H__ */
