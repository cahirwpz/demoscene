#ifndef __C2P_H__
#define __C2P_H__

void c2p1x1_8_c5_bm(__reg("a0") UBYTE *chunky, __reg("a1") struct BitMap *bitmap,
                    __reg("d0") USHORT width, __reg("d1") USHORT height,
                    __reg("d2") USHORT offsetX, __reg("d3") USHORT offsetY);

#endif
