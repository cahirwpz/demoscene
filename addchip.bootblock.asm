; Downloaded from https://github.com/earok/close-and-run-amiga68k

        include 'lvo/exec_lib.i'
        include 'lvo/intuition_lib.i'

Height		EQU	50
zeropage	EQU	$100

RT_INIT		EQU	$16
lib_Version	EQU	$14
LibList		EQU	$17a
io_Device	EQU	$14
KS20		EQU	36


boot		dc.b	'DOS',0
		dc.b	'r05s'				;tagged
		dc.l	880

start		movem.l	d2-d5/a2/a5,-(a7)
		
		moveq	#-1,d0				;disable externals drives
		movea.l	io_Device(a1),a1
		movea.l	$3c(a1),a1			;the 'old times mod'
		lea	$34(a1),a1
		move.l	d0,(a1)+			;df1:
		move.l	d0,(a1)+			;df2:
		move.l	d0,(a1)				;and why not.. df3:
		
		moveq	#0,d4
		cmpi.w	#KS20,lib_Version(a6)		;fork
		shs	d4

		lea	zeropage.w,a5			;a0jmp
		move.l	a5,d5
		lea	a0jmp(pc),a0
		moveq	#(endscreen-a0jmp)/4-1,d0
_l		move.l	(a0)+,(a5)+
		dbf	d0,_l				;at end a5=zerobss
		
		movea.l	LibList(a6),a0
		lea	intuitionlib(pc),a1
		jsr	_LVOFindName(a6)
		move.l	d0,(a5)+			;intuitionbase
		beq.b	_exit
		
		lea	intuitionlib(pc),a1
		jsr	_LVOFindResident(a6)
		tst.l	d0				;can be omitted..
		beq.b	_exit
		
		lea	wbname(pc),a1			;'Workbench Screen',0
		moveq	#doslib-wbname,d1		;strlen
		bsr.b	findstrres
		move.l	a0,title-(zerobss+4)(a5)

		move.l	d4,(a5)+			;v36?
		beq.b	notags
		moveq	#10,d1				;strlen
		clr.b	-1(a1,d1.w)			;'Workbench',0
		bsr.b	findstrres
		move.l	a0,pubname-(zerobss+8)(a5)
		
		
notags		lea	doslib(pc),a1
		jsr	_LVOFindResident(a6)
		tst.l	d0
		beq.b	_exit
		movea.l	d0,a1
		move.l	RT_INIT(a1),(a5)		;dosinit

		tst.l	d4				;v36?
		beq.b	_done
		move.w	#Height,_h-(zerobss+8)(a5)	;set height for >=v36
		jsr	_LVOCacheClearU(a6)

_done		moveq	#-1,d0
_exit		movea.l	d5,a0				;a0jmp
		not.l	d0
		movem.l	(sp)+,d2-d5/a2/a5
		rts


;input:		d0=resident base
;		d1=strlen
;		a1=string to search
;output:	a0=&str
;used:		d2/d3/a2 saved by main, a0 as return code

findstrres	movea.l	d0,a0
_search		move.w	d1,d2
		subq.w	#2,d2			;-(dbne,+)
		movea.l	a1,a2
		move.b	(a2)+,d3
_s1		cmp.b	(a0)+,d3
		bne.b	_s1
_s2		cmpm.b	(a2)+,(a0)+
		dbne	d2,_s2
		bne.b	_search
		suba.l	d1,a0
		rts

wbname		dc.b	'Workbench Screen',0
doslib		dc.b	'dos.library',0
intuitionlib	dc.b	'intuition.library',0

		cnop	0,4

fakechecksum	dc.l	$b99c2e9d

a0jmp		lea	zerobss(pc),a5
		movea.l	(a5)+,a6		;intuitionbase
		jsr	_LVOOpenIntuition(a6)	;KS1.x mandatory (undocumented)
		lea	wbscreen(pc),a0
		tst.l	(a5)+
		bne.b	v36
		jsr	_LVOOpenScreen(a6)
		bra.s	dos
v36		lea	tagitems(pc),a1		;Workbench need to be Pub
		jsr	_LVOOpenScreenTagList(a6)
dos		movea.l	(a5),a0			;dosinit
		jmp	(a0)			;start amigados
		cnop	0,4
		
tagitems	dc.l	$8000002f		;SA_PubName
pubname		dc.l	0			;'Workbench',0		
;		dc.l	0			;TAG_DONE (fixed to wbscreen struct)
		;
wbscreen	dc.w	0			;WORD LeftEdge
		dc.w	0			;WORD TopEdge
		dc.w	640			;WORD Widht
_h		dc.w	200			;WORD Height (KS1.x min height)
		dc.w	1			;WORD Depth
		dc.b	0,1			;UBYTE DetailPen, BlockPen
		dc.w	$8000			;UWORD ViewModes -> HIRES
		dc.w	1			;UWORD Type -> WBENCHSCREEN
		dc.l	0			;struct TextAttr *Font, defaults
title		dc.l	0			;UBYTE *DefaultTitle, ROM
		dc.l	0			;struct Gadget *Gadgets, UNUSED
		dc.l	0			;struct BitMap *CustomBitMap, NULL
endscreen	;
		;
zerobss		;				;(fixed to wbscreen struct)
;intuitionbase	;zerobss+0
;		ds.l	1
;stag		;zerobss+4
;		ds.l	1
;dosinit	;zerobss+8
;		ds.l	1
endzero

		cnop	0,16
ver		dc.b	'$VER: addchip.bootblock 1.1 (27.07.2017) by ross',0

		cnop	0,16
message		dc.b	' This code snippet allows you to regain some precious chip ram. '
		cnop	0,16
		dc.b	'*       Greetings to all Evergreen Amiga Board members!        *'
		even
		
		END

; vim: ft=asm68k:ts=8:sw=8:et
