#ifndef __LZO_H__
#define __LZO_H__

#include <exec/types.h>

LONG lzo1x_decompress(CONST UBYTE *in asm("a0"), ULONG in_len asm("d0"),
                      UBYTE *out asm("a1"), ULONG *out_len asm("a2"));

#endif
