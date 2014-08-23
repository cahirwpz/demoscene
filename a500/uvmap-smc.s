; vim: ft=asm68k:ts=8:sw=8

        XDEF    _UVMapTemplateStart
        XDEF    _UVMapTemplateEnd

        section "UVMapTemplate", code
; 32 pixel
; 13.25 cycles per pixel
; 160 * 128 in 1.92 vbl
_UVMapTemplateStart:
        move.w  $1000(a0),d0    ; 12 * 32
        or.w    $1000(a1),d0
        move.b  $1000(a0),d0
        or.b    $1000(a1),d0
        move.w  $1000(a0),d1
        or.w    $1000(a1),d1
        move.b  $1000(a0),d1
        or.b    $1000(a1),d1
        move.w  $1000(a0),d2
        or.w    $1000(a1),d2
        move.b  $1000(a0),d2
        or.b    $1000(a1),d2
        move.w  $1000(a0),d3
        or.w    $1000(a1),d3
        move.b  $1000(a0),d3
        or.b    $1000(a1),d3
        move.w  $1000(a0),d4
        or.w    $1000(a1),d4
        move.b  $1000(a0),d4
        or.b    $1000(a1),d4
        move.w  $1000(a0),d5
        or.w    $1000(a1),d5
        move.b  $1000(a0),d5
        or.b    $1000(a1),d5
        move.w  $1000(a0),d6
        or.w    $1000(a1),d6
        move.b  $1000(a0),d6
        or.b    $1000(a1),d6
        move.w  $1000(a0),d7
        or.w    $1000(a1),d7
        move.b  $1000(a0),d7
        or.b    $1000(a1),d7
        movem.w d0-d7,-(a2)     ; 8 + 32
_UVMapTemplateEnd:
        rts
