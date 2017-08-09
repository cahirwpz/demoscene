#ifndef __C2P_1x1_4_H__
#define __C2P_1x1_4_H__

/*
 * width: must be even multiple of 16
 * bplSize: offset between one row in one bpl and the next bpl in bytes
 */

void c2p_1x1_4(APTR chunky asm("a0"), APTR bpls asm("a1"),
               WORD width asm("d0"), WORD height asm("d1"),
               LONG bplSize asm("d5"));

#endif
