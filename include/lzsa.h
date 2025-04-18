#ifndef __LZSA_H__
#define __LZSA_H__

void lzsa_depack_stream(const void *input asm("a0"), void *output asm("a2"));

#endif /* !__LZSA_H__ */
