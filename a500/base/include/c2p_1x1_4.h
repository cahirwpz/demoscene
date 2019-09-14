#ifndef __C2P_1x1_4_H__
#define __C2P_1x1_4_H__

/*
 * width: must be even multiple of 16
 * bplSize: offset between one row in one bpl and the next bpl in bytes
 */

void c2p_1x1_4(void *chunky asm("a0"), void *bpls asm("a1"),
               short width asm("d0"), short height asm("d1"),
               int bplSize asm("d5"));

#endif
