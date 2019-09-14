#ifndef __LZO_H__
#define __LZO_H__

#include "types.h"

int lzo1x_decompress(const u_char *in asm("a0"), u_int in_len asm("d0"),
                      u_char *out asm("a1"), u_int *out_len asm("a2"));

#endif
