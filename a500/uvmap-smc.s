; vim: ft=asm68k:ts=8:sw=8

        XDEF    _UVMapRenderTemplate

        section "UVMapRender", data

; 15 cycles per pixel
_UVMapRenderTemplate:
        REPT    160*100/2
        move.b  $1000(a1),d0    ; 12 * 2
        or.b    $1000(a2),d0
        move.b  d0,(a0)+        ; 8
        ENDR
        rts
