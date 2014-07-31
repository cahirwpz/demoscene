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
        WORD    rotateX
        WORD    rotateY
        WORD    rotateZ
	LABEL   View3D_SIZE

    STRUCTURE   Object3D,0
        UWORD   nVertex
        UWORD   nEdge
        APTR    vertex
        APTR    edge
        APTR    point
        APTR    pointFlags
        APTR    line
        APTR    lineFlags
        LABEL   Object3D_SIZE

        XDEF    _CalculateView3D
        XDEF    _TransformVertices

        XREF    _sincos

        SECTION 3d,code

; a0 [View3D *] view structure

sinX    EQUR    d0
sinY    EQUR    d1
sinZ    EQUR    d2
cosX    EQUR    d3
cosY    EQUR    d4
cosZ    EQUR    d5
saved   EQURL   d2-d7/a2

_CalculateView3D:
        movem.l saved,-(sp)

        movem.w rotateX(a0),d0-d2

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
        muls.w  sinZ,d6
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
; a1 [Object3D *] 3d object structure

x       EQUR    d0
y       EQUR    d1
z       EQUR    d2
saved   EQURL   d2-d7/a2-a6

width   EQU     320
height  EQU     256

_TransformVertices:
        movem.l saved,-(sp)

        move.l  vertex(a1),a6
        move.l  point(a1),a2
        move.l  pointFlags(a1),a3
        move.w  nVertex(a1),d0
        subq.w  #1,d0

        move.w  #width,a4
        move.w  #height,a5

.loop:
        movem.l d0/a0,-(sp) ; these get clobbered within loop's body

        movem.w (a6)+,x/y/z

        movem.w (a0)+,d3-d5
        muls.w  x,d3
        muls.w  y,d4
        muls.w  z,d5
        add.l   d4,d3
        add.l   d5,d3
        asr.l   #8,d3   ; x' = m[0][0] * x + m[1][0] * y + m[2][0] * z

        movem.w (a0)+,d4-d6
        muls.w  x,d4
        muls.w  y,d5
        muls.w  z,d6
        add.l   d5,d4
        add.l   d6,d4
        asr.l   #8,d4   ; y' = m[0][1] * x + m[1][1] * y + m[2][1] * z

        movem.w (a0)+,d5-d7
        muls.w  x,d5
        muls.w  y,d6
        muls.w  z,d7
        add.l   d6,d5
        add.l   d7,d5
        asr.l   #8,d5   ; z' = m[0][2] * x + m[1][2] * y + m[2][2] * z

        movem.w (a0)+,x/y/z

        ; some magic value
        add.w   #384,d5

        tst.w   d5
        bne     .perspective

        move.w  x,(a2)+
        move.w  y,(a2)+
        clr.b   (a3)+
        bra     .continue

.perspective
        clr.w   d6

        muls.w  z,d3
        divs.w  d5,d3
        add.w   x,d3     ; x2d = x' * viewerZ / z' + viewerX
        move.w  d3,(a2)+
        
        slt     d6       ; set PF_LEFT iff x2d < 0
        add.w   d6,d6
        cmp.w   a4,d3    ; set PF_RIGHT iff x2d >= width
        sge     d6
        add.w   d6,d6

        muls.w  z,d4
        divs.w  d5,d4
        add.w   y,d4     ; y2d = y' * viewerZ / z' + viewerY
        move.w  d4,(a2)+

        slt     d6       ; set PF_TOP iff y2d < 0
        add.w   d6,d6
        cmp.w   a5,d4    ; set PF_BOTTOM iff y2d >= height
        sge     d6
        lsr.w   #7,d6

        move.b  d6,(a3)+

.continue
        movem.l (sp)+,d0/a0
        dbf     d0,.loop

        movem.l (sp)+,saved
        rts
