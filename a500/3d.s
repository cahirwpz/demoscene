; vim: ft=asm68k:ts=8:sw=8:

        INCLUDE "exec/types.i"

   STRUCTURE    View3D,0
	WORD    m00
	WORD    m10
	WORD    m20
	WORD    m01
	WORD    m11
	WORD    m21
	WORD    m02
	WORD    m12
	WORD    m22
        WORD    viewerX
        WORD    viewerY
        WORD    viewerZ
	LABEL   View3D_SIZE

        XDEF    _CalculateView3D
        XDEF    _TransformVertices
        XREF    _sincos

        SECTION 3d,code

; a0 [View3D *] view structure
; d0 [WORD] alpha
; d1 [WORD] beta
; d2 [WORD] gamma

sinX    EQUR    d0
sinY    EQUR    d1
sinZ    EQUR    d2
cosX    EQUR    d3
cosY    EQUR    d4
cosZ    EQUR    d5
saved   EQURL   d2-d7/a2

_CalculateView3D:
        movem.l saved,-(sp)

        lea     _sincos,a1

        and.w   #$1ff,d0
        and.w   #$1ff,d1
        and.w   #$1ff,d2

        lsl.w   #2,d0
        lsl.w   #2,d1
        lsl.w   #2,d2

        movem.w (a1,d0.w),sinX/cosX
        movem.w (a1,d1.w),sinY/cosY
        movem.w (a1,d2.w),sinZ/cosZ

        move.w  cosY,d6
        muls.w  cosZ,d6
        asr.l   #8,d6
        move.w  d6,m00(a0)      ; m[0][0] = cosY * cosZ

        move.w  cosY,d6
        move.w  sinZ,d6
        asr.l   #8,d6
        move.w  d6,m01(a0)      ; m[0][1] = cosY * sinZ

        move.w  sinY,d6
        neg.w   d6
        move.w  d6,m02(a0)      ; m[0][2] = -sinY

        move.w  sinX,d7
        muls.w  sinY,d7
        asr.l   #8,d7
        move.w  d7,a2           ; a = sinX * sinY

        move.w  a2,d6
        muls.w  cosZ,d6
        move.w  cosX,d7
        muls.w  sinZ,d7
        sub.l   d7,d6
        asr.l   #8,d6
        move.w  d6,m10(a0)      ; m[1][0] = a * cosZ - cosX * sinZ

        move.w  a2,d6
        muls.w  sinZ,d6
        move.w  cosX,d7
        muls.w  cosZ,d7
        add.l   d7,d6
        asr.l   #8,d6
        move.w  d6,m11(a0)      ; m[1][1] = a * sinZ + cosX * cosZ

        move.w  sinX,d6
        muls.w  cosY,d6
        asr.l   #8,d6
        move.w  d6,m12(a0)      ; m[1][2] = sinX * cosY

        move.w  cosX,d7
        muls.w  sinY,d7
        asr.l   #8,d7
        move.w  d7,a2           ; b = cosX * sinY

        move.w  a2,d6
        muls.w  cosZ,d6
        move.w  sinX,d7
        muls.w  sinZ,d7
        add.l   d7,d6
        asr.l   #8,d6
        move.w  d6,m20(a0)      ; m[2][0] = b * cosZ + sinX * sinZ

        move.w  a2,d6
        muls.w  sinZ,d6
        move.w  sinX,d7
        muls.w  cosZ,d7
        sub.l   d7,d6
        asr.l   #8,d6
        move.w  d6,m21(a0)      ; m[2][1] = b * sinZ - sinX * cosZ

        move.w  cosX,d6
        muls.w  cosY,d6
        asr.l   #8,d6
        move.w  d6,m22(a0)      ; m[2][2] = cosX * cosY

        movem.l (sp)+,saved
        rts

; a0 [View3D *] view structure
; a1 [VertexT *] vertices array
; a2 [PointT *] 2d points array
; d0 [WORD] number of vertices

x       EQUR    d0
y       EQUR    d1
z       EQUR    d2
saved   EQURL   d2-d7/a2-a3

_TransformVertices:
        movem.l saved,-(sp)
        subq.l  #1,d0

.loop:
        move.l  a0,a3
        exg.l   d0,a4
        movem.w (a1)+,x/y/z

        movem.w (a3)+,d3-d5
        muls.w  x,d3
        muls.w  y,d4
        muls.w  z,d5
        add.l   d4,d3
        add.l   d5,d3
        asr.l   #8,d3   ; x' = m[0][0] * x + m[1][0] * y + m[2][0] * z

        movem.w (a3)+,d4-d6
        muls.w  x,d4
        muls.w  y,d5
        muls.w  z,d6
        add.l   d5,d4
        add.l   d6,d4
        asr.l   #8,d4   ; y' = m[0][1] * x + m[1][1] * y + m[2][1] * z

        movem.w (a3)+,d5-d7
        muls.w  x,d5
        muls.w  y,d6
        muls.w  z,d7
        add.l   d6,d5
        add.l   d7,d5
        asr.l   #8,d5   ; z' = m[0][2] * x + m[1][2] * y + m[2][2] * z

        movem.w (a3)+,x/y/z

        ; some magic value
        add.l   #250,d5

        tst.l   d5
        bne     .perspective

        move.w  x,(a2)+
        move.w  y,(a2)+
        bra     .continue

.perspective
        muls.w  z,d3
        divs.w  d5,d3
        add.w   x,d3     ; x2d = x' * viewerZ / z' + viewerX
        move.w  d3,(a2)+

        muls.w  z,d4
        divs.w  d5,d4
        add.w   y,d4     ; y2d = y' * viewerZ / z' + viewerY
        move.w  d4,(a2)+

.continue
        exg.l   d0,a4
        dbf     d0,.loop

        movem.l (sp)+,saved
        rts
