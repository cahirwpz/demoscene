        include 'hardware/custom.i'
        include 'hardware/cia.i'
        include 'exec/memory.i'

ChipStart equ $400
ChipEnd   equ $80000
SlowStart equ $c00000
SlowEnd   equ $c80000

; custom chips addresses

custom  equ     $dff000
ciaa    equ     $bfe001
ciab    equ     $bfd000

; 512kB kickstart begin address

        org     $f80000

Kickstart:
        dc.l    $400             ; Initial SP
        dc.l    Entry            ; Initial PC

HunkFilePtr:
        dc.l    0
HunkFileSize:
        dc.l    0

; The ROM is located at $fc0000 but is mapped at $0 after reset shadowing RAM
Entry:
        lea     ciaa,a4
        lea     ciab,a5
        lea     custom,a6
        move.b  #3,ciaddra(a4)  ; Set port A direction to output for /LED and OVL
        move.b  #0,ciapra(a4)   ; Disable OVL (Memory from $0 onwards available)

InitHW:
        move.w  #$7fff,d0       ; Make sure DMA and interrupts are disabled
        move.w  d0,intena(a5)
        move.w  d0,intreq(a5)
        move.w  d0,dmacon(a5)

        ; Wake up line & frame counters
        clr.b   ciatodhi(a4)
        clr.b   ciatodmid(a4)
        clr.b   ciatodlow(a4)
        clr.b   ciatodhi(a5)
        clr.b   ciatodmid(a5)
        clr.b   ciatodlow(a5)

Setup:
        move.l  HunkFileSize(pc),d2
        move.l  HunkFilePtr(pc),d3
        move.l  #BD_SIZE+2*MR_SIZE,a3
        sub.l   a3,sp
        move.l  sp,a6

        clr.l   BD_HUNK(a6)
        clr.l   BD_VBR(a6)
        move.b  #1,BD_BOOTDEV(a6)
        clr.b   BD_CPUMODEL(a6)
        move.w  #2,BD_NREGIONS(a6)

        lea     BD_REGION(a6),a0
        move.l  #SlowStart,(a0)+
        move.l  #SlowEnd,(a0)+
        move.w  #MEMF_FAST|MEMF_PUBLIC,(a0)+
        move.l  #ChipStart,(a0)+
        move.l  #ChipEnd,(a0)+
        move.w  #MEMF_CHIP|MEMF_PUBLIC,(a0)+

ROM = 1

        include 'bootloader.asm'

; vim: ft=asm68k:ts=8:sw=8:noet:
