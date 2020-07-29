        xdef    _CinterInit
        xdef    _CinterPlay1
        xdef    _CinterPlay2

CINTER_DEGREES  equ     16384

                rsreset
c_SampleState   rs.l    3
c_PeriodTable   rs.w    36
c_TrackSize     rs.w    1
c_InstPointer   rs.l    1
c_MusicPointer  rs.l    1
c_MusicEnd      rs.l    1
c_MusicLoop     rs.l    1
c_MusicState    rs.l    4*3
c_dma           rs.w    1
c_waitline      rs.w    1
c_Instruments   rs.l    32*2
c_Sinus         rs.w    CINTER_DEGREES
c_SIZE          rs.w    0

        section '.text',code

_CinterInit:
        movem.l d2-d7/a2-a6,-(sp)

        ; Raw instrument data must be copied to the start of the
        ; instrument space before or after CinterInit is called.

CinterMakeSinus:
        lea     c_Sinus(a6),a0
        addq.l  #2,a0
        lea.l   CINTER_DEGREES/2*2-2(a0),a1

        moveq.l #1,d7
.loop:
        move.w  d7,d1
        mulu.w  d7,d1
        lsr.l   #8,d1

        move.w  #2373,d0
        mulu.w  d1,d0
        swap.w  d0
        neg.w   d0
        add.w   #21073,d0
        mulu.w  d1,d0
        swap.w  d0
        neg.w   d0
        add.w   #51469,d0
        mulu.w  d7,d0
        lsr.l   #8,d0
        lsr.l   #5,d0

        move.w  d0,(a0)+
        move.w  d0,-(a1)
        neg.w   d0
        move.w  d0,CINTER_DEGREES/2*2-2(a0)
        move.w  d0,CINTER_DEGREES/2*2(a1)

        addq.w  #1,d7
        cmp.w   #CINTER_DEGREES/4,d7
        blt.b   .loop

        move.w  #16384,d0
        move.w  d0,(a0)+
        move.w  d0,-(a1)
        neg.w   d0
        move.w  d0,CINTER_DEGREES/2*2-2(a0)
        move.w  d0,CINTER_DEGREES/2*2(a1)

; Sample parameters:
; short length, replength
; short mpitch, mod, bpitch
; short attack, distortions, decay
; short mpitchdecay, moddecay, bpitchdecay

; Sample state:
; long mpitch, mod, bpitch
; short ampdelta,amp

LONGMUL macro
        move.w  d0,d1
        swap.w  d0
        mulu.w  d2,d0
        mulu.w  d2,d1
        clr.w   d1
        swap.w  d1
        add.l   d1,d0
        endm

CinterMakeInstruments:
        lea     c_Sinus(a6),a0
        lea     c_Instruments(a6),a5

        ; Loop through instruments
        move.w  (a2)+,d7
        bpl.b   .instrumentloop
        not.w   d7
        ; Raw instruments
.rawloop:
        ; Read length
        clr.l   d5
        move.w  (a2),d5
        move.l  (a2)+,(a5)+
        move.l  a4,(a5)+
        add.l   d5,d5

        add.l   d5,a4
        dbf     d7,.rawloop

        move.w  (a2)+,d7
        ; Generated instruments
.instrumentloop:
        ; Read length
        clr.l   d5
        move.w  (a2),d5
        move.l  (a2)+,(a5)+
        move.l  a4,(a5)+
        add.l   d5,d5

        ; Init state
        move.l  a6,a1
        rept    3
        move.w  (a2)+,(a1)+
        clr.w   (a1)+
        endr
        move.w  (a2)+,(a1)+
        clr.w   (a1)+

        clr.w   (a4)+
        subq.l  #2,d5
        moveq.l #0,d6   ; Index
.sampleloop:

        ; Distortion parameters
        move.l  a2,a3
        move.w  (a3)+,d4

        ; Modulation wave
        move.l  a6,a1
        move.w  d6,d2
        move.l  (a1)+,d0
        lsr.l   #2,d0
        LONGMUL
.mdist: lsr.w   #2,d0
        add.w   d0,d0
        move.w  (a0,d0.w),d0
        sub.w   #$1000,d4
        bcc.b   .mdist
        lsl.w   #4,d4

        ; Modulation strength
        move.w  d0,d2
        add.w   #$8000,d2
        move.l  (a1),d3
        lsr.l   #3,d3
        move.l  (a1)+,d0
        lsr.l   #2,d0
        LONGMUL
        sub.l   d0,d3

        ; Base wave
        move.w  d6,d2
        move.l  (a1)+,d0
        lsr.l   #2,d0
        LONGMUL
        sub.l   d3,d0   ; Modulation
.bdist: lsr.w   #2,d0
        add.w   d0,d0
        move.w  (a0,d0.w),d0
        sub.w   #$1000,d4
        bcc.b   .bdist
        lsl.w   #4,d4

        ; Amplitude
        move.w  (a1)+,d1
.vpower:        muls.w  0(a1),d0        ; Dummy offset for better compression
        add.l   d0,d0
        swap.w  d0
        sub.w   #$1000,d4
        bcc.b   .vpower
        lsl.w   #4,d4

        ; Final distortion
        bra.b   .fdist_in
.fdist: lsr.w   #2,d0
        add.w   d0,d0
        move.w  (a0,d0.w),d0
.fdist_in:      sub.w   #$1000,d4
        bcc.b   .fdist

        ; Write sample
        add.w   d0,d0
        bvc.b   .notover
        subq.w  #1,d0
.notover:       asr.w   #8,d0
        move.b  d0,(a4)+

        ; Attack-Decay
        move.w  (a3)+,d2
        sub.w   d1,(a1)
        bvc.b   .nottop
        move.w  #32767,(a1)
        move.w  d2,-(a1)
.nottop:        bpl.b   .notzero
        clr.w   (a1)
.notzero:
        ; Pitch and mod decays
        move.l  a6,a1
        rept    3
        move.l  (a1),d0
        move.w  (a3)+,d2
        beq.b   *+16
        LONGMUL
        move.l  d0,(a1)+
        endr

        addq.l  #1,d6
        cmp.l   d5,d6
        blt.w   .sampleloop

        move.l  a3,a2
        dbf     d7,.instrumentloop

CinterComputePeriods:
;       lea     c_PeriodTable(a6),a1
        move.w  #$e2b3,d0
        move.l  #$0fc0fd20,d2
        moveq.l #0,d6
        moveq.l #3*12-1,d7
.loop1: mulu.w  #61865,d0
        swap.w  d0
        move.w  d0,d1
        lsr.w   #6,d1
        add.l   d2,d2
        subx.w  d6,d1
        move.w  d1,(a1)+
        dbf     d7,.loop1

CinterParseMusic:
;       lea     c_TrackSize(a6),a1
        move.w  (a2)+,d1
        move.w  (a2)+,d0
        move.w  d1,(a1)+
        move.l  a2,(a1)+
        add.w   d0,a2
        move.l  a2,(a1)+
        move.w  -(a2),d0
        add.w   d1,a2
        move.l  a2,(a1)+
        add.w   d0,a2
        move.l  a2,(a1)+
CinterInitEnd:
        movem.l (sp)+,d2-d7/a2-a6
        rts


_CinterPlay1:
        movem.l d4-d5/a2-a3,-(sp)

        ; No filter!
        bset.b  #1,$bfe001

        ; Read music data
        lea.l   $dff000,a3
        lea     c_TrackSize(a6),a0
        move.w  (a0)+,d1
        move.l  (a0)+,a2
        move.l  (a0)+,a1

        ; Loop when end is reached
        cmp.l   (a0)+,a1
        bls.b   .notend
        move.l  (a0),a1
        move.l  a1,-8(a0)
.notend:

        ; Turn off DMA for triggered channels
        moveq.l #0,d4
        rept    4
        move.w  (a1),d0
        add.w   d1,a1
        add.w   d0,d0
        addx.w  d4,d4
        endr
        move.w  d4,$096(a3)

        ; Save line and dma
        move.w  $006(a3),d5
        movem.w d4/d5,c_dma-c_MusicLoop(a0)

        movem.l (sp)+,d4-d5/a2-a3
        rts

_CinterPlay2:
        movem.l d2-d7/a2-a5,-(sp)

        ; Advance position
        lea.l   $dff000,a3
        lea     c_TrackSize(a6),a0
        move.w  (a0)+,d1
        move.l  (a0)+,a2
        move.l  (a0),a1
        addq.l  #2,(a0)+
        addq.l  #8,a0

        ; Write to audio registers
        lea.l   $0e0(a3),a3
        clr.l   d5
        moveq.l #4-1,d7
.channelloop:
        move.l  (a0)+,d3
        move.l  (a0)+,a4
        move.l  (a0),d2 ; Period|Volume
        move.w  (a1),d0
        add.w   d1,a1
        bmi.b   .trigger

        ; Adjust volume
        rol.w   #7,d0
        add.w   d0,d2
        and.w   #63,d2
        swap.w  d2

        ; Adjust or set period
        asr.w   #7,d0
        add.w   d0,d2
        add.b   d0,d0
        bvc.b   .slide
        move.w  c_PeriodTable(a6,d0.w),d2
.slide: swap.w  d2
        bra.b   .write

.trigger:
        ; Set volume
        rol.w   #7,d0
        move.w  d0,d2
        and.w   #63,d2
        swap.w  d2

        ; Look up note
        lsr.w   #7,d0
        move.l  a2,a5
        moveq.l #-8,d3
        moveq.l #0,d4
.noteloop:      move.b  (a5)+,d2
        move.b  (a5)+,d4
        move.w  (a5)+,d5
        bne.b   .sameinst
        addq.w  #8,d3
.sameinst:      sub.w   d4,d0
        bge.b   .noteloop
        add.w   d4,d0
        add.b   d2,d0
        add.b   d0,d0
        move.w  c_PeriodTable(a6,d0.w),d2
        swap.w  d2

        ; Set instrument
        lea     c_Instruments(a6),a5
        add.w   d3,a5

        ; Read sample address, length and repeat
        moveq.l #1,d6
        move.w  (a5)+,d3
        move.w  (a5)+,d0
        move.l  (a5),a4
        beq.b   .norepeat
        move.w  d0,d6
        sub.w   d3,d0
        sub.w   d0,a4
        sub.w   d0,a4
.norepeat:
        ; Save restart position and length
        movem.l d6/a4,-8(a0)
        ; Add offset to sample address
        move.l  (a5),a4
        sub.w   d5,d3
        add.w   d5,d5
        add.l   d5,a4
.write:
        ; Save period and volume
        move.l  d2,(a0)+

        ; Write to audio registers
        subq.l  #6,a3
        move.l  d2,-(a3)        ; Period|Volume
        move.w  d3,-(a3)        ; Length
        move.l  a4,-(a3)        ; Pointer
        dbf     d7,.channelloop

        ; Wait for old DMA to stop, then start new DMA
        move.w  (a0)+,d4
        beq.b   .nodma
        or.w    #$8000,d4
        move.w  (a0)+,d5
        add.w   #$0780,d5
        lea.l   $dff000,a3
.dmawait:
        cmp.w   $006(a3),d5
        bgt.b   .dmawait
        move.w  d4,$096(a3)
.nodma:
        movem.l (sp)+,d2-d7/a2-a5
        rts

; vim: ts=8 sw=8 ft=asm68k
