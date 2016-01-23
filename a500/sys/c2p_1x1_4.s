; Date: 1999-12-06			Mikael Kalms (Scout/C-Lous & more)
;					Email: mikael@kalms.org
;
; 4bpl c2p which performs word writes, rather than longword ones.

        xdef    _c2p_1x1_4

	section	code,code

; d0.w	chunkyx [chunky-pixels] (must be even multiple of 16)
; d1.w	chunkyy [chunky-pixels]
; d5.l	bplsize [bytes] -- offset between one row in one bpl and the next bpl
; a0	chunkybuffer
; a1	bitplanes

_c2p_1x1_4
	movem.l	d2-d7/a2-a6,-(sp)

	lsr.w	#3,d0
	
	move.w	d1,d2
	mulu.w	d0,d2
        lsl.l   #3,d2
	lea	(a0,d2.l),a2

	move.l	a1,a3
	lea	(a1,d5.l),a4
        add.l   d5,d5
	lea	(a1,d5.l),a5
	lea	(a4,d5.l),a6

	move.l	#$0f0f0f0f,d4
	move.l	#$00ff00ff,d5

	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	and.l	d4,d0
	and.l	d4,d1
	and.l	d4,d2
	and.l	d4,d3
	lsl.l	#4,d0
	lsl.l	#4,d1
	or.l	d2,d0
	or.l	d3,d1

; a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
; i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#8,d2
	eor.l	d0,d2
	and.l	d5,d2
	eor.l	d2,d0
	lsl.l	#8,d2
	eor.l	d2,d1

; a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
; b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#1,d2
	eor.l	d0,d2
	and.l	#$55555555,d2
	eor.l	d2,d0
	add.l	d2,d2
	eor.l	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
; a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d2
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0
; c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d2

	bra.s	.start

.pix16
	move.l	(a0)+,d0
	move.l	(a0)+,d2
	move.l	(a0)+,d1
	move.l	(a0)+,d3

	move.w	d6,(a5)+
	swap	d6

	and.l	d4,d0
	and.l	d4,d1
	and.l	d4,d2
	and.l	d4,d3
	lsl.l	#4,d0
	lsl.l	#4,d1
	or.l	d2,d0
	or.l	d3,d1

; a3a2a1a0e3e2e1e0 b3b2b1b0f3f2f1f0 c3c2c1c0g3g2g1g0 d3d2d1d0h3h2h1h0
; i3i2i1i0m3m2m1m0 j3j2j1j0n3n2n1n0 k3k2k1k0o3o2o1o0 l3l2l1l0p3p2p1p0

	move.w	d7,(a3)+
	swap	d7

	move.l	d1,d2
	lsr.l	#8,d2
	eor.l	d0,d2
	and.l	d5,d2
	eor.l	d2,d0
	lsl.l	#8,d2
	eor.l	d2,d1

; a3a2a1a0e3e2e1e0 i3i2i1i0m3m2m1m0 c3c2c1c0g3g2g1g0 k3k2k1k0o3o2o1o0
; b3b2b1b0f3f2f1f0 j3j2j1j0n3n2n1n0 d3d2d1d0h3h2h1h0 l3l2l1l0p3p2p1p0

	move.l	d1,d2
	lsr.l	#1,d2

	move.w	d6,(a6)+

	eor.l	d0,d2
	and.l	#$55555555,d2
	eor.l	d2,d0
	add.l	d2,d2
	eor.l	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1
; a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.w	d1,d2
	move.w	d0,d1
	swap	d1
	move.w	d1,d0
	move.w	d2,d1

; a3b3a1b1e3f3e1f1 i3j3i1j1m3n3m1n1 a2b2a0b0e2f2f0f0 i2j2i0j0m2n2m0n0
; c3d3c1d1g3h3g1h1 k3l3k1l1o3p3o1p1 c2d2c0d0g2h2g0h0 k2l2k0l0o2p2o0p0

	move.l	d1,d2

	move.w	d7,(a4)+
.start
	lsr.l	#2,d2
	eor.l	d0,d2
	and.l	#$33333333,d2
	eor.l	d2,d0
	lsl.l	#2,d2
	eor.l	d2,d1

; a3b3c3d3e3f3g3h3 i3j3k3l3m3n3o3p3 a2b2c2d2e2f2g2h2 i2j2k2l2m2n2o2p2
; a1b1c1d1e1f1g1h1 i1j1k1l1m1n1o1p1 a0b0c0d0e0f0g0h0 i0j0k0l0m0n0o0p0

	move.l	d0,d6
	move.l	d1,d7

	cmp.l	a0,a2
	bne.s	.pix16

	move.w	d6,(a5)+
	swap	d6
	move.w	d7,(a3)+
	swap	d7
	move.w	d6,(a6)+
	move.w	d7,(a4)+

	movem.l	(sp)+,d2-d7/a2-a6
	rts

; vim: ft=asm68k:ts=8:sw=8
