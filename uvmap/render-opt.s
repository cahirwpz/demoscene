; vim: ft=asm68k:ts=8:sw=8:

        XDEF    _RenderFastUVMapOptimized
        XDEF    _RenderNormalUVMapOptimized

        section RenderNormalUVMapOptimized, code

; a0 [uint16_t *] mapU
; a1 [uint16_t *] mapV
; a2 [uint8_t *] texture
; a6 [uint8_t *] dst
; d5 [int32_t] n
; d6 [uint16_t] offsetU
; d7 [uint16_t] offsetV

_RenderFastUVMapOptimized:
        movem.l d2-d7/a2-a6,-(sp)
        add.l   #32768,a2
        move.b  d7,d6
        nop

.loop:
        move.w  (a0)+,d0
        move.w  (a1)+,d1
        
        move.w  d0,d2   ; u1u2
        rol.w   #8,d0   ; u2u1

        move.w  d1,d3   ; v1v2
        rol.w   #8,d1   ; v2v1

        move.b  d3,d0   ; u2v2
        move.b  d1,d2   ; u1v1
        
        add.w   d6,d0
        add.w   d6,d2

        move.b  (a2,d2.w),(a6)+
        move.b  (a2,d0.w),(a6)+
        
        subq.l  #2,d5
        bgt.s   .loop

        movem.l (sp)+,d2-d7/a2-a6
        rts

_RenderNormalUVMapOptimized:
        movem.l d2-d7/a2-a6,-(sp)
        add.l   #32768,a2
        move.b  d7,d6
        nop

.loop:
        move.l  (a0)+,d0        ; --uu--UU
        move.l  (a1)+,d1        ; --vv--VV
        lsl.l   #8,d0           ; uu--UU00
        and.l   #$00ff00ff,d1   ; 00vv00VV
        and.l   #$ff00ff00,d0   ; uu00UU00
        or.l    d1,d0           ; uuvvUUVV 
        move.l  d0,d2
        swap    d2
        add.w   d6,d0
        add.w   d6,d2
        move.b  (a2,d2.w),(a6)+
        move.b  (a2,d0.w),(a6)+
        subq.l  #2,d5
        bgt.s   .loop

        movem.l (sp)+,d2-d7/a2-a6
        rts
