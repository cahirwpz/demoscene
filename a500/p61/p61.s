; vim: ft=asm68k:ts=8:sw=8:
;
; The Player 6.1A - by Guru / Sahara Surfers
;
; Interrupt problem with 060 fixed by NoName/Haujobb^Sector 7
; Some Enforcer Hits fixed by Platon42
; Memory trashing bug in routine P61_End fixed by Tolkien
; Bug in routine P61_Init (when opt020=1) fixed by The Dark Coder/Trinity^Morbid Visions
;
; for Devpac 3, ASM-One, PhxAss and maybe some other assemblers
;
; NEEDS: Default sizes must be words (ASM-One default...)
;        Preferably no optimizations on (the jump table must be word jumps)
;
; Tested with Devpac 3.04 by Guru
; Tested with PhxAss 4.14, Asm-Pro 1.1 and ASM-One 1.29 
;
; Note 1:All the bugfixes done after version 610.2 (the latest release from
;        Guru / Sahara Surfers) are marked with the comment "* BUGFIX"
;        followed by some explanations, so that if can quickly copy them
;        into a previous version tailored to your needs.
;
; Note 2:The unelegant double WRITE to INTREQ unforunately seems to be
;	 the best solution to the "interrupt problem" that occurs on
;	 some 040/060 equipped Amiga. On many 040/060 Amigas it is enough
;	 to put an access to whatever hardware register and a NOP between
;	 the first write to INTREQ and he RTE. However we had reports that
;	 on some Amiga it is really necessary to do a double write to INTREQ

	xdef _P61_Init, _P61_SetPosition, _P61_End, _P61_ControlBlock

_P61_Init
	movem.l	d2-d7/a2-a6,-(sp)
	moveq.l	#0,d0
	bsr	P61_Init
	movem.l	(sp)+,d2-d7/a2-a6
	rts

_P61_SetPosition
	movem.l a3/a6,-(sp)
	lea	$dff000,a6
	bsr	P61_SetPosition
	movem.l	(sp)+,a3/a6
	rts

_P61_End
	movem.l a3/a6,-(sp)
	lea	$dff000,a6
	bsr	P61_End
	movem.l	(sp)+,a3/a6
	rts

******************************************
* Assembler Control Parameters

example = 0	;0 = assemble the pure replay
		;1 = assemble as an example, "P61.testmod" should be played

;makebin	;uncomment this and a new binary will be
		;AUTO-saved to "ram:610.6.bin_new" (ASM-One only)

asmone = 0	;0 = Devpac, PhxAss and the like
		;1 = ASM-One (prints out some infos while assembling)

;start = 6	;Starting position - Leave undefined to start from beginning 

fade  = 1	;0 = Normal, NO master volume control possible
		;1 = Use master volume (control parameter P61_Master)

jump = 1	;0 = do NOT include position jump code (P61_SetPosition)
		;1 = Include

system = 0	;0 = killer
		;1 = friendly

CIA = 1		;0 = CIA disabled - Tempo commands are skipped !!
		;1 = CIA enabled

exec = 1	;0 = ExecBase destroyed - Remember to set VBR by yourself
		;1 = ExecBase valid

; The "NonLev6" mode was initially planned but never implemented because
; slower than the two lev6 modes (CIA disabled and CIA enabled).
lev6 = 1	;0 = NonLev6
		;1 = Lev6 used

opt020 = 0	;0 = MC680x0 code
		;1 = MC68020+ or better

channels = 4	;amount of channels to be played

use = -1	;The Usecode as returned by the P61 converter. If the Usecode
		;for a module is not known, or the same routine has to play
		;several modules, set the parameter to -1. 

	incdir	include/
	include Player61.i
        ifne    asmone
        include infos.s
        endc

	ifne	example
	section	Player6.1A,code

	movem.l	d0-a6,-(sp)
	lea	$dff000,a6
	ifeq	system
	move	$1c(a6),-(sp)
	move	#$7fff,$9a(a6)
	move	2(a6),-(sp)
	move	#$7ff,$96(a6)
	endc

	lea	P61_data,a0	;Module
	sub.l	a1,a1		;No separate samples
;	lea	P61_smp,a1	;Samples

	sub.l	a2,a2		;No packed samples
;       lea     samples,a2      ;Sample buffer in chip,only for packed samples
	moveq	#0,d0		;Auto Detect
	bsr	P61_motuuli+P61_InitOffset

	tst	d0		;Went ok?
	bne	P61_exit

P61_sync
	ifeq	CIA
	move.l	4(a6),d0
	andi.l	#$1ff00,d0
	cmp.l	#$8100,d0
	bne.b	P61_sync

P61_sync2
	move.l	4(a6),d0
	andi.l	#$1ff00,d0
	cmp.l	#$8200,d0
	bne.b	P61_sync2

	ifne	example
	ifnd	makebin
	move	#$fff,$180(a6)
	endc
	endc
	bsr	P61_motuuli+P61_MusicOffset
	ifne	example
	ifnd	makebin
	clr	$180(a6)
	endc
	endc

	moveq	#0,d0
	move	6(a6),d0
	sub.l	#$8200,d0
	cmp.l	P61_raster(pc),d0
	ble.b	P61_kosj
	move	d0,P61_raster+2
P61_kosj
	tst	P61_raster2+2
	bne.b	P61_doing
	move	d0,P61_raster2+2
	bra	P61_doneg
P61_doing
	add.l	P61_raster2(pc),d0
	asr.l	#1,d0
	move.l	d0,P61_raster2
P61_doneg
	addq.l	#1,P61_frames

	ifne	fade
	btst	#2,$16(a6)
	bne.b	P61_jid
	move	P61_diri(pc),d0
	sub	d0,P61_motuuli+P61_MasterVolume
	bne.b	P61_judo
	neg	P61_diri
	bra.b	P61_jid
P61_judo
	cmp	#64,P61_motuuli+P61_MasterVolume
	bne.b	P61_jid
	neg	P61_diri
	endc

P61_jid
	endc

	btst	#6,$bfe001
	bne	P61_sync

P61_exit
	bsr	P61_motuuli+P61_EndOffset

	ifeq	system
	move	(sp)+,d7
	bset	#15,d7
	move	#$7ff,$96(a6)
	move	d7,$96(a6)

	move	(sp)+,d7
	bset	#15,d7
	move	#$7fff,$9a(a6)
	move	d7,$9a(a6)
	endc
	movem.l	(sp)+,d0-a6

	move.l	P61_raster(pc),d0
	move.l	P61_raster2(pc),d1
	move.l	P61_frames(pc),d2
	move.l	P61_positionbase(pc),a0
	move.l	P61_patternbase(pc),a1
	move.l	P61_spos(pc),a2
	rts

P61_IRQsave
		dc	0
P61_DMAsave
		dc	0
P61_raster
		dc.l	0
P61_raster2
		dc.l	0
P61_frames
		dc.l	0
P61_diri
		dc	1
	endc
********************************
*        Player 6.1A ®         *
*      All in one-version      *
*        Version 610.6         *
*   © 1992-95 Jarno Paananen   *
*     All rights reserved      *
* Fixed by:  NoName, Platon42, *
*  Tolkien and The Dark Coder  *
********************************

******************************************
* START OF BINARY FILE

P61_motuuli
	bra.w	P61_Init
	ifeq	CIA
	bra.w	P61_Music
	else
	rts
	rts
	endc
	bra.w	P61_End
	rts				;no P61_SetRepeat
	rts
	ifne	jump
	bra.w	P61_SetPosition
	else
	rts
	rts
	endc


******************************************
* Run-time Control Parameters

_P61_ControlBlock

P61_Master
		dc.w	64		;Master volume (0-64)
P61_Tempo
		dc.w	1		;Use tempo? (0=no,non-zero=yes)
P61_Play
		dc.w	0		;Activation flag (0=stop,1=play)
P61_E8
		dc.w	0		;Info nybble after command E8
P61_VBR
		dc.l	0		;If you're using non-valid execbase
					;put VBR here! (Otherwise 0 assumed)
					;You can also get VBR from here, if
					;using exec-valid version

P61_Pos
		dc.w	0		;Current song position (read only)
P61_Patt
		dc.w	0		;Current pattern (read only)
P61_CRow
		dc.w	0		;Current pattern row (read only)

P61_Temp0Offset
	dc.l	P61_temp0-P61_motuuli
P61_Temp1Offset
	dc.l	P61_temp1-P61_motuuli
P61_Temp2Offset
	dc.l	P61_temp2-P61_motuuli
P61_Temp3Offset
	dc.l	P61_temp3-P61_motuuli

P61_getnote	macro
	moveq	#$7e,d0
	and.b	(a5),d0
	beq.b	.nonote
	ifne	P61_vib
	clr.b	P61_VibPos(a5)
	endc
	ifne	P61_tre
	clr.b	P61_TrePos(a5)
	endc

	ifne	P61_ft
	add	P61_Fine(a5),d0
	endc
	move	d0,P61_Note(a5)
	move	(a2,d0),P61_Period(a5)

.nonote
	endm

	ifeq	system
	ifne	CIA
P61_intti
	movem.l	d0-a6,-(sp)
	tst.b	$bfdd00
	lea	$dff000,a6
	move	#$2000,$9c(a6)

;---------------------------------------------
* BUGFIX - 610.3 / 27.04.98 - done by NoName
; code added in 610.3:
	move	#$2000,$9c(a6)
;---------------------------------------------

	ifne	example
	ifnd	makebin
	move	#$fff,$180(a6)
	endc
	endc
	bsr	P61_Music
	ifne	example
	ifnd	makebin
	move	#0,$180(a6)
	endc
	endc
	movem.l	(sp)+,d0-a6
	nop
	rte
	endc
	endc

	ifne	system
P61_lev6server
	movem.l	d2-d7/a2-a6,-(sp)
	lea	P61_timeron(pc),a0
	tst	(a0)
	beq.b	P61_ohi

	lea	$dff000,a6
	move	P61_server(pc),d0
	beq.b	P61_musica
	subq	#1,d0
	beq	P61_dmason
	bra	P61_setrepeat

P61_musica
	bsr	P61_Music

P61_ohi	
	movem.l	(sp)+,d2-d7/a2-a6
	moveq	#1,d0
	rts
	endc

******************************************
* P61_Init
; Call this routine to initialize the playroutine

* Input:	A0 [LONG] = Address of the module
*		A1 [LONG] = 0 if samples are internal to the module
*		     Address of samples otherwise
*		A2 [LONG] = Address of sample buffer if the module uses packed
*		     samples, otherwise can be left uninitialized
*		D0 [WORD] = 0  autodetect CIA Timer frequency, if ExecBase
*			       is valid otherwise assume PAL
*		     	    1  assume PAL
*		            2  assume NTSC
*		   [Used only il CIA-enabled mode]

* Output:	D0 [LONG] = 0 if success, non-zero otherwise
*		A6 [LONG] = $DFF000

* Uses:		D0-D7/A0-A6

P61_Init
	cmp.l	#"P61A",(a0)+
	beq.b	.modok
	subq.l	#4,a0

.modok
	ifne	CIA
	move	d0,-(sp)
	endc

	moveq	#0,d0
	cmp.l	d0,a1
	bne.b	.redirect

	move	(a0),d0
	lea	(a0,d0.l),a1
.redirect
	move.l	a2,a6
	lea	8(a0),a2
	moveq	#$40,d0
	and.b	3(a0),d0
	bne.b	.buffer
	move.l	a1,a6
	subq.l	#4,a2
.buffer

	lea	P61_cn(pc),a3
	moveq	#$1f,d1
	and.b	3(a0),d1
	move.l	a0,-(sp)
	lea	P61_samples(pc),a4
	subq	#1,d1
	moveq	#0,d4
P61_lopos
	move.l	a6,(a4)+
	move	(a2)+,d4
	bpl.b	P61_kook
	neg	d4
	lea	P61_samples-16(pc),a5
	ifeq	opt020
	asl	#4,d4
	move.l	(a5,d4),d6
	else
	add	d4,d4
	move.l	(a5,d4*8),d6
	endc
	move.l	d6,-4(a4)

;---------------------------------------------
* BUGFIX - 610.6 / 25.07.04 - done by The Dark Coder
; original code in version 610.2: 
;	move	4(a5,d4),d4
; replaced with:
	ifeq	opt020
	move	4(a5,d4),d4
	else
	move	4(a5,d4*8),d4
	endc
;---------------------------------------------

	sub.l	d4,a6
	sub.l	d4,a6
	bra.b	P61_jatk

P61_kook
	move.l	a6,d6
	tst.b	3(a0)
	bpl.b	P61_jatk

	tst.b	(a2)
	bmi.b	P61_jatk

	move	d4,d0
	subq	#2,d0
	bmi.b	P61_jatk

	move.l	a1,a5
	move.b	(a5)+,d2
	sub.b	(a5),d2
	move.b	d2,(a5)+
.loop	sub.b	(a5),d2
	move.b	d2,(a5)+
	sub.b	(a5),d2
	move.b	d2,(a5)+
	dbf	d0,.loop

P61_jatk
	move	d4,(a4)+
	moveq	#0,d2
	move.b	(a2)+,d2
	moveq	#0,d3
	move.b	(a2)+,d3

	moveq	#0,d0
	move	(a2)+,d0
	bmi.b	.norepeat

	move	d4,d5
	sub	d0,d5
	move.l	d6,a5

	add.l	d0,a5
	add.l	d0,a5

	move.l	a5,(a4)+
	move	d5,(a4)+
	bra.b	P61_gene
.norepeat
	move.l	d6,(a4)+
	move	#1,(a4)+
P61_gene
	move	d3,(a4)+
	moveq	#$f,d0
	and	d2,d0
	mulu	#74,d0
	move	d0,(a4)+

	tst	-6(a2)
	bmi.b	.nobuffer

	moveq	#$40,d0
	and.b	3(a0),d0
	beq.b	.nobuffer

	move	d4,d7
	tst.b	d2
	bpl.b	.copy

	subq	#1,d7
	moveq	#0,d5
	moveq	#0,d4
.lo	move.b	(a1)+,d4
	moveq	#$f,d3
	and	d4,d3
	lsr	#4,d4

	sub.b	.table(pc,d4),d5
	move.b	d5,(a6)+
	sub.b	.table(pc,d3),d5
	move.b	d5,(a6)+
	dbf	d7,.lo
	bra.b	.kop

.copy
	add	d7,d7
	subq	#1,d7
.cob
	move.b	(a1)+,(a6)+
	dbf	d7,.cob
	bra.b	.kop

.table
	dc.b	0,1,2,4,8,16,32,64,128,-64,-32,-16,-8,-4,-2,-1

.nobuffer
	move.l	d4,d6
	add.l	d6,d6
	add.l	d6,a6
	add.l	d6,a1
.kop
	dbf	d1,P61_lopos

	move.l	(sp)+,a0
	and.b	#$7f,3(a0)

	move.l	a2,-(sp)

	lea	P61_temp0(pc),a1
	lea	P61_temp1(pc),a2
	lea	P61_temp2(pc),a4
	lea	P61_temp3(pc),a5
	moveq	#Channel_Block_SIZE/2-2,d0

	moveq	#0,d1
.cl	move	d1,(a1)+
	move	d1,(a2)+
	move	d1,(a4)+
	move	d1,(a5)+
	dbf	d0,.cl

	lea	P61_temp0-P61_cn(a3),a1
	lea	P61_emptysample-P61_cn(a3),a2
	moveq	#channels-1,d0
.loo

;---------------------------------------------
* BUGFIX - 610.4 / 16.06.98 - done by Platon42
; original code in version 610.2: 
;	move.l	a2,P61_Sample(a2)
; replaced with:
	move.l	a2,P61_Sample(a1)
	lea	Channel_Block_SIZE(a1),a1
;---------------------------------------------

	dbf	d0,.loo

	move.l	(sp)+,a2
	move.l	a2,P61_positionbase-P61_cn(a3)

	moveq	#$7f,d1
	and.b	2(a0),d1

	ifeq	opt020
	lsl	#3,d1
	lea	(a2,d1.l),a4
	else
	lea	(a2,d1.l*8),a4
	endc
	move.l	a4,P61_possibase-P61_cn(a3)

	move.l	a4,a1
	moveq	#-1,d0
.search
	cmp.b	(a1)+,d0
	bne.b	.search
	move.l	a1,P61_patternbase-P61_cn(a3)	
	move.l	a1,d0
	sub.l	a4,d0
	move	d0,P61_slen-P61_cn(a3)

	ifd	start
	lea	start(a4),a4
	endc

	moveq	#0,d0
	move.b	(a4)+,d0
	move.l	a4,P61_spos-P61_cn(a3)
	lsl	#3,d0
	add.l	d0,a2

	move.l	a1,a4
	moveq	#0,d0	
	move	(a2)+,d0
	lea	(a4,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp0-P61_cn(a3)
	move	(a2)+,d0
	lea	(a4,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp1-P61_cn(a3)
	move	(a2)+,d0
	lea	(a4,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp2-P61_cn(a3)
	move	(a2)+,d0
	lea	(a4,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp3-P61_cn(a3)

	lea	P61_setrepeat(pc),a0
	move.l	a0,P61_intaddr-P61_cn(a3)

	move	#63,P61_rowpos-P61_cn(a3)
	move	#6,P61_speed-P61_cn(a3)
	move	#5,P61_speed2-P61_cn(a3)
	clr	P61_speedis1-P61_cn(a3)

	ifne	P61_pl
	clr.l	P61_plcount-P61_cn(a3)
	endc

	ifne	P61_pde
	clr	P61_pdelay-P61_cn(a3)
	clr	P61_pdflag-P61_cn(a3)
	endc
	clr	(a3)

	moveq	#2,d0
	and.b	$bfe001,d0
	move.b	d0,P61_ofilter-P61_cn(a3)
	bset	#1,$bfe001

	ifeq	system
	ifne	exec
	move.l	4.w,a6
	moveq	#0,d0
	btst	d0,297(a6)
	beq.b	.no68010

	lea	P61_liko(pc),a5
	jsr	-$1e(a6)

.no68010
	move.l	d0,P61_VBR-P61_cn(a3)
	endc

	move.l	P61_VBR-P61_cn(a3),a0
	lea	$78(a0),a0
	move.l	a0,P61_vektori-P61_cn(a3)

	move.l	(a0),P61_oldlev6-P61_cn(a3)
	lea	P61_dmason(pc),a1
	move.l	a1,(a0)
	endc

	moveq	#0,d0
	lea	$dff000,a6
	move	d0,$a8(a6)
	move	d0,$b8(a6)
	move	d0,$c8(a6)
	move	d0,$d8(a6)
	move	#$f,$96(a6)

	ifeq	system
	lea	P61_dmason(pc),a1
	move.l	a1,(a0)
	move	#$2000,$9a(a6)
	lea	$bfd000,a0
	lea	P61_timers(pc),a1
	move.b	#$7f,$d00(a0)
	move.b	#$10,$e00(a0)
	move.b	#$10,$f00(a0)
	move.b	$400(a0),(a1)+
	move.b	$500(a0),(a1)+
	move.b	$600(a0),(a1)+
	move.b	$700(a0),(a1)
	endc

	ifeq	system!CIA
	move.b	#$82,$d00(a0)
	endc

	ifne	CIA
	move	(sp)+,d0
	subq	#1,d0
	beq.b	P61_ForcePAL
	subq	#1,d0
	beq.b	P61_NTSC
	ifne	exec
	move.l	4.w,a1
	cmp.b	#60,$213(a1)	;PowerSupplyFrequency
	beq.b	P61_NTSC
	endc
P61_ForcePAL
	move.l	#1773447,d0	;PAL
	bra.b	P61_setcia
P61_NTSC
	move.l	#1789773,d0	;NTSC
P61_setcia
	move.l	d0,P61_timer-P61_cn(a3)
	divu	#125,d0
	move	d0,P61_thi2-P61_cn(a3)
	sub	#$1f0*2,d0
	move	d0,P61_thi-P61_cn(a3)

	ifeq	system
	move	P61_thi2-P61_cn(a3),d0
	move.b	d0,$400(a0)
	lsr	#8,d0
	move.b	d0,$500(a0)
	lea	P61_intti(pc),a1
	move.l	a1,P61_tintti-P61_cn(a3)
	move.l	P61_vektori(pc),a2
	move.l	a1,(a2)
	move.b	#$83,$d00(a0)
	move.b	#$11,$e00(a0)
	endc
	endc

	ifeq	system
	move	#$e000,$9a(a6)
	moveq	#0,d0
	rts

	ifne	exec
P61_liko
	dc.l	$4E7A0801		;MOVEC	VBR,d0
	rte
	endc
	endc

	ifne	system
	move.l	a6,-(sp)

	ifne	CIA
	clr	P61_server-P61_cn(a3)
	else
	move	#1,P61_server-P61_cn(a3)
	endc

	move.l	4.w,a6
	moveq	#-1,d0
	jsr	-$14a(a6)
	move.b	d0,P61_sigbit-P61_cn(a3)
	bmi	P61_err

	lea	P61_allocport(pc),a1
	move.l	a1,P61_portti-P61_cn(a3)
	move.b	d0,15(a1)
	move.l	a1,-(sp)
	suba.l	a1,a1
	jsr	-$126(a6)
	move.l	(sp)+,a1
	move.l	d0,16(a1)
	lea	P61_reqlist(pc),a0
	move.l	a0,(a0)
	addq.l	#4,(a0)
	clr.l	4(a0)
	move.l	a0,8(a0)

	lea	P61_dat(pc),a1
	move.l	a1,P61_reqdata-P61_cn(a3)
	lea	P61_allocreq(pc),a1
	lea	P61_audiodev(pc),a0
	moveq	#0,d0
	moveq	#0,d1
	jsr	-$1bc(a6)
	tst.l	d0
	bne	P61_err
	st.b	P61_audioopen-P61_cn(a3)

	lea	P61_timerint(pc),a1
	move.l	a1,P61_timerdata-P61_cn(a3)
	lea	P61_lev6server(pc),a1
	move.l	a1,P61_timerdata+8-P61_cn(a3)

	moveq	#0,d3
	lea	P61_cianame(pc),a1
P61_openciares
	moveq	#0,d0
	move.l	4.w,a6
	jsr	-$1f2(a6)
	move.l	d0,P61_ciares-P61_cn(a3)
	beq.b	P61_err
	move.l	d0,a6
	lea	P61_timerinterrupt(pc),a1
	moveq	#0,d0
	jsr	-6(a6)
	tst.l	d0
	beq.b	P61_gottimer
	addq.l	#4,d3
	lea	P61_timerinterrupt(pc),a1
	moveq	#1,d0
	jsr	-6(a6)
	tst.l	d0
	bne.b	P61_err

P61_gottimer
	lea	P61_craddr+8(pc),a6
	move.l	P61_ciaaddr(pc,d3),d0
	move.l	d0,(a6)
	sub	#$100,d0
	move.l	d0,-(a6)
	moveq	#2,d3
	btst	#9,d0
	bne.b	P61_timerB
	subq.b	#1,d3
	add	#$100,d0
P61_timerB
	add	#$900,d0
	move.l	d0,-(a6)
	move.l	d0,a0
	and.b	#%10000000,(a0)
	move.b	d3,P61_timeropen-P61_cn(a3)
	moveq	#0,d0
	ifne	CIA
	move.l	P61_craddr+4(pc),a1
	move.b	P61_tlo(pc),(a1)
	move.b	P61_thi(pc),$100(a1)
	endc
	or.b	#$19,(a0)
	st	P61_timeron-P61_cn(a3)
P61_pois
	move.l	(sp)+,a6
	rts

P61_err	moveq	#-1,d0
	bra.b	P61_pois
	rts

P61_ciaaddr
	dc.l	$bfd500,$bfd700
	endc

******************************************
* P61_End
; Call this routine to stop the music

* Input:	A6 [LONG] = $DFF000 

* Uses:		D0-D1/A0/A1/A3

P61_End
	moveq	#0,d0
	move	d0,$a8(a6)
	move	d0,$b8(a6)
	move	d0,$c8(a6)
	move	d0,$d8(a6)
	move	#$f,$96(a6)

	and.b	#~2,$bfe001
	move.b	P61_ofilter(pc),d0
	or.b	d0,$bfe001

	ifeq	system
	move	#$2000,$9a(a6)
	move.l	P61_vektori(pc),a0
	move.l	P61_oldlev6(pc),(a0)
	lea	$bfd000,a0
	lea	P61_timers(pc),a1
	move.b	(a1)+,$400(a0)
	move.b	(a1)+,$500(a0)
	move.b	(a1)+,$600(a0)
	move.b	(a1)+,$700(a0)
	move.b	#$10,$e00(a0)
	move.b	#$10,$f00(a0)

	else

;---------------------------------------------
* BUGFIX - 610.5 / ??.??.?? - done by Tolkien
; original code in version 610.2: 
;	clr	P61_timeron-P61_cn(a3)
;	move.l	a6,-(sp)
;	lea	P61_cn(pc),a3
;	moveq	#0,d0
; replaced with:
	move.l	a6,-(sp)
	lea	P61_cn(pc),a3
	moveq	#0,d0
	clr	P61_timeron-P61_cn(a3)
;---------------------------------------------

	move.b	P61_timeropen(pc),d0
	beq.b	P61_rem1
	move.l	P61_ciares(pc),a6
	lea	P61_timerinterrupt(pc),a1
	subq.b	#1,d0
	jsr	-12(a6)
P61_rem1
	move.l	4.w,a6
	tst.b	P61_audioopen-P61_cn(a3)
	beq.b	P61_rem2
	lea	P61_allocreq(pc),a1
	jsr	-$1c2(a6)
	clr.b	P61_audioopen-P61_cn(a3)
P61_rem2
	moveq	#0,d0
	move.b	P61_sigbit(pc),d0
	bmi.b	P61_rem3
	jsr	-$150(a6)
	st	P61_sigbit-P61_cn(a3)
P61_rem3
	move.l	(sp)+,a6
	endc
	rts

	ifne	fade
P61_mfade
	move	P61_Master(pc),d0
	move	P61_temp0+P61_Shadow(pc),d1
	mulu	d0,d1
	lsr	#6,d1
	move	d1,$a8(a6)

	ifgt	channels-1
	move	P61_temp1+P61_Shadow(pc),d1
	mulu	d0,d1
	lsr	#6,d1
	move	d1,$b8(a6)
	endc

	ifgt	channels-2
	move	P61_temp2+P61_Shadow(pc),d1
	mulu	d0,d1
	lsr	#6,d1
	move	d1,$c8(a6)
	endc

	ifgt	channels-3
	move	P61_temp3+P61_Shadow(pc),d1
	mulu	d0,d1
	lsr	#6,d1
	move	d1,$d8(a6)
	endc
	rts
	endc
	
******************************************
* P61_SetPosition
; Call P61_SetPosition to jump to a specific position in the song
;­ Starts from the beginning if out of limits.	­

* Input:	D0 [LONG] = Position (Starts from the beginning if is
*			    out of limits)

* Uses:		D0-D3/A0/A1/A6

	ifne	jump
P61_SetPosition
	lea	P61_cn(pc),a3
	ifne	P61_pl
	clr	P61_plflag-P61_cn(a3)
	endc
	moveq	#0,d1
	move.b	d0,d1
	move.l	d1,d0
	cmp	P61_slen-P61_cn(a3),d0
	blo.b	.e
	moveq	#0,d0
.e	move	d0,P61_Pos-P61_cn(a3)
	add.l	P61_possibase(pc),d0
	move.l	d0,P61_spos-P61_cn(a3)

	moveq	#64,d0
	move	d0,P61_rowpos-P61_cn(a3)
	clr	P61_CRow-P61_cn(a3)
	move.l	P61_spos(pc),a1
	move.l	P61_patternbase(pc),a0
	addq	#1,P61_Pos-P61_cn(a3)
	move.b	(a1)+,d0
	move.l	a1,P61_spos-P61_cn(a3)
	move.l	P61_positionbase(pc),a1
	move	d0,P61_Patt-P61_cn(a3)
	lsl	#3,d0
	add.l	d0,a1
	movem	(a1),d0-d3
	lea	(a0,d0.l),a1
	move	d1,d0
	move.l	a1,P61_ChaPos+P61_temp0-P61_cn(a3)
	lea	(a0,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp1-P61_cn(a3)
	move	d2,d0
	lea	(a0,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp2-P61_cn(a3)
	move	d3,d0
	add.l	d0,a0
	move.l	a0,P61_ChaPos+P61_temp3-P61_cn(a3)
	rts
	endc

******************************************
* P61_Music
; If in CIA-disabled mode, call P61_Music every frame to play the music
; otherwise the routine is called by the CIA interrupt handler. 

* Input:	A6 [LONG] = $DFF000 

* Uses:		D0-D7/A0-A5

P61_Music
	lea	P61_cn(pc),a3

	tst	P61_Play-P61_cn(a3)
	bne.b	P61_ohitaaa
	ifne	CIA
	ifne	system
	move.l	P61_craddr+4(pc),a0
	move.b	P61_tlo2(pc),(a0)
	move.b	P61_thi2(pc),$100(a0)
	endc
	endc
	rts

P61_ohitaaa
	ifne	fade
	pea	P61_mfade(pc)
	endc

	moveq	#Channel_Block_SIZE,d6
	moveq	#16,d7

	move	(a3),d4
	addq	#1,d4
	cmp	P61_speed(pc),d4
	beq	P61_playtime

	move	d4,(a3)

P61_delay
	ifne	CIA
	ifne	system
	move.l	P61_craddr+4(pc),a0
	move.b	P61_tlo2(pc),(a0)
	move.b	P61_thi2(pc),$100(a0)
	endc
	endc

	lea	P61_temp0(pc),a5
	lea	$a0(a6),a4

	moveq	#channels-1,d5
P61_lopas
	tst	P61_OnOff(a5)
	beq	P61_contfxdone
	moveq	#$f,d0
	and	(a5),d0
	ifeq	opt020
	add	d0,d0
	move	P61_jtab2(pc,d0),d0
	else
	move	P61_jtab2(pc,d0*2),d0
	endc
	jmp	P61_jtab2(pc,d0)

P61_jtab2
	dc	P61_contfxdone-P61_jtab2

	ifne	P61_pu
	dc	P61_portup-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_pd
	dc	P61_portdwn-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_tp
	dc	P61_toneport-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_vib
	dc	P61_vib2-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_tpvs
	dc	P61_tpochvslide-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_vbvs
	dc	P61_vibochvslide-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_tre
	dc	P61_tremo-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	ifne	P61_arp
	dc	P61_arpeggio-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	dc	P61_contfxdone-P61_jtab2

	ifne	P61_vs
	dc	P61_volslide-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc

	dc	P61_contfxdone-P61_jtab2
	dc	P61_contfxdone-P61_jtab2
	dc	P61_contfxdone-P61_jtab2

	ifne	P61_ec
	dc	P61_contecommands-P61_jtab2
	else
	dc	P61_contfxdone-P61_jtab2
	endc
	dc	P61_contfxdone-P61_jtab2

	ifne	P61_ec
P61_contecommands
	move.b	P61_Info(a5),d0
	and	#$f0,d0
	lsr	#3,d0
	move	P61_etab2(pc,d0),d0
	jmp	P61_etab2(pc,d0)

P61_etab2
	dc	P61_contfxdone-P61_etab2

	ifne	P61_fsu
	dc	P61_fineup2-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	ifne	P61_fsd
	dc	P61_finedwn2-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	dc	P61_contfxdone-P61_etab2
	dc	P61_contfxdone-P61_etab2

	dc	P61_contfxdone-P61_etab2
	dc	P61_contfxdone-P61_etab2

	dc	P61_contfxdone-P61_etab2
	dc	P61_contfxdone-P61_etab2

	ifne	P61_rt
	dc	P61_retrig-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	ifne	P61_fvu
	dc	P61_finevup2-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	ifne	P61_fvd
	dc	P61_finevdwn2-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	ifne	P61_nc
	dc	P61_notecut-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	ifne	P61_nd
	dc	P61_notedelay-P61_etab2
	else
	dc	P61_contfxdone-P61_etab2
	endc

	dc	P61_contfxdone-P61_etab2
	dc	P61_contfxdone-P61_etab2
	endc

	ifne	P61_fsu
P61_fineup2
	tst	(a3)
	bne	P61_contfxdone
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	sub	d0,P61_Period(a5)
	moveq	#113,d0
	cmp	P61_Period(a5),d0
	ble.b	.jup
	move	d0,P61_Period(a5)
.jup	move	P61_Period(a5),6(a4)
	bra	P61_contfxdone
	endc

	ifne	P61_fsd
P61_finedwn2
	tst	(a3)
	bne	P61_contfxdone
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	add	d0,P61_Period(a5)
	cmp	#856,P61_Period(a5)
	ble.b	.jup
	move	#856,P61_Period(a5)
.jup	move	P61_Period(a5),6(a4)
	bra	P61_contfxdone
	endc

	ifne	P61_fvu
P61_finevup2
	tst	(a3)
	bne	P61_contfxdone
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	add	d0,P61_Volume(a5)
	moveq	#64,d0
	cmp	P61_Volume(a5),d0
	bge.b	.jup
	move	d0,P61_Volume(a5)
.jup	move	P61_Volume(a5),8(a4)
	bra	P61_contfxdone
	endc

	ifne	P61_fvd
P61_finevdwn2
	tst	(a3)
	bne	P61_contfxdone
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	sub	d0,P61_Volume(a5)
	bpl.b	.jup
	clr	P61_Volume(a5)
.jup	move	P61_Volume(a5),8(a4)
	bra	P61_contfxdone
	endc

	ifne	P61_nc
P61_notecut
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	cmp	(a3),d0
	bne	P61_contfxdone
	ifeq	fade
	clr	8(a4)
	else
	clr	P61_Shadow(a5)
	endc
	clr	P61_Volume(a5)
	bra	P61_contfxdone
	endc

	ifne	P61_nd
P61_notedelay
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	cmp	(a3),d0
	bne	P61_contfxdone

	moveq	#$7e,d0
	and.b	(a5),d0
	beq	P61_contfxdone
	move	P61_DMABit(a5),d0
	move	d0,$96(a6)
	or	d0,P61_dma-P61_cn(a3)
	move.l	P61_Sample(a5),a1		;* Trigger *
	move.l	(a1)+,(a4)+			;Pointer
	move	(a1),(a4)+			;Length
	move	P61_Period(a5),(a4)
	subq.l	#6,a4

	ifeq	system
	lea	P61_dmason(pc),a1
	move.l	P61_vektori(pc),a0
	move.l	a1,(a0)
	move.b	#$f0,$bfd600
	move.b	#$01,$bfd700
	move.b	#$19,$bfdf00
	else
	move	#1,P61_server-P61_cn(a3)
	move.l	P61_craddr+4(pc),a1
	move.b	#$f0,(a1)
	move.b	#1,$100(a1)
	endc
	bra	P61_contfxdone
	endc

	ifne	P61_rt
P61_retrig
	subq	#1,P61_RetrigCount(a5)
	bne	P61_contfxdone
	move	P61_DMABit(a5),d0
	move	d0,$96(a6)
	or	d0,P61_dma-P61_cn(a3)
	move.l	P61_Sample(a5),a1		;* Trigger *
	move.l	(a1)+,(a4)			;Pointer
	move	(a1),4(a4)			;Length

	ifeq	system
	lea	P61_dmason(pc),a1
	move.l	P61_vektori(pc),a0
	move.l	a1,(a0)
	move.b	#$f0,$bfd600
	move.b	#$01,$bfd700
	move.b	#$19,$bfdf00
	else
	move	#1,P61_server-P61_cn(a3)
	move.l	P61_craddr+4(pc),a1
	move.b	#$f0,(a1)
	move.b	#1,$100(a1)
	endc

	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	move	d0,P61_RetrigCount(a5)
	bra	P61_contfxdone
	endc

	ifne	P61_arp
P61_arplist
 dc.b 0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1,-1,0,1

P61_arpeggio
	move	(a3),d0
	move.b	P61_arplist(pc,d0),d0
	beq.b	.arp0
	bmi.b	.arp1

	move.b	P61_Info(a5),d0
	lsr	#4,d0
	bra.b	.arp3

.arp0
	move	P61_Note(a5),d0
	move	P61_periods(pc,d0),6(a4)
	bra	P61_contfxdone

.arp1
	moveq	#$f,d0
	and.b	P61_Info(a5),d0

.arp3
	add	d0,d0
	add	P61_Note(a5),d0
	move	P61_periods(pc,d0),6(a4)
	bra	P61_contfxdone
	endc

P61_periods
	ifne	P61_ft
	incbin	periods
	else
	incbin	periods.nft
	endc

	ifne	P61_vs
P61_volslide
	move.b	P61_Info(a5),d0
	sub.b	d0,P61_Volume+1(a5)
	bpl.b	.test
	clr	P61_Volume(a5)
	ifeq	fade
	clr	8(a4)
	else
	clr	P61_Shadow(a5)
	endc
	bra	P61_contfxdone
.test
	moveq	#64,d0
	cmp	P61_Volume(a5),d0
	bge.b	.ncs
	move	d0,P61_Volume(a5)
	ifeq	fade
	move	d0,8(a4)
	else
	move	d0,P61_Shadow(a5)
	endc
	bra.b	P61_contfxdone
.ncs
	ifeq	fade
	move	P61_Volume(a5),8(a4)
	else
	move	P61_Volume(a5),P61_Shadow(a5)
	endc
	bra.b	P61_contfxdone
	endc

	ifne	P61_tpvs
P61_tpochvslide
	move.b	P61_Info(a5),d0
	sub.b	d0,P61_Volume+1(a5)
	bpl.b	.test
	clr	P61_Volume(a5)
	ifeq	fade
	clr	8(a4)
	else
	clr	P61_Shadow(a5)
	endc
	bra.b	P61_toneport
.test
	moveq	#64,d0
	cmp	P61_Volume(a5),d0
	bge.b	.ncs
	move	d0,P61_Volume(a5)
.ncs
	ifeq	fade
	move	P61_Volume(a5),8(a4)
	else
	move	P61_Volume(a5),P61_Shadow(a5)
	endc
	endc

	ifne	P61_tp
P61_toneport
	move	P61_ToPeriod(a5),d0
	beq.b	P61_contfxdone
	move	P61_TPSpeed(a5),d1
	cmp	P61_Period(a5),d0
	blt	.topoup

	add	d1,P61_Period(a5)
	cmp	P61_Period(a5),d0
	bgt.b	.setper
	move	d0,P61_Period(a5)
	clr	P61_ToPeriod(a5)
	move	d0,6(a4)
	bra	P61_contfxdone

.topoup
	sub	d1,P61_Period(a5)
	cmp	P61_Period(a5),d0
	blt.b	.setper
	move	d0,P61_Period(a5)
	clr	P61_ToPeriod(a5)
.setper
	move	P61_Period(a5),6(a4)
	else
	nop
	endc

P61_contfxdone
	ifne	P61_il
	bsr	P61_funk2
	endc

	add.l	d6,a5
	add.l	d7,a4
	dbf	d5,P61_lopas

	cmp	P61_speed2(pc),d4
	beq.b	P61_preplay
	rts

	ifne	P61_pu
P61_portup
	moveq	#0,D0
	move.b	P61_Info(a5),d0
	sub	d0,P61_Period(a5)
	moveq	#113,d0
	cmp	P61_Period(a5),d0
	ble.b	.skip
	move	d0,P61_Period(a5)
	move	d0,6(a4)
	bra.b	P61_contfxdone
.skip
	move	P61_Period(a5),6(a4)
	bra.b	P61_contfxdone
	endc

	ifne	P61_pd
P61_portdwn
	moveq	#0,d0
	move.b	P61_Info(a5),d0
	add	d0,P61_Period(a5)
	cmp	#856,P61_Period(a5)
	ble.b	.skip
	move	#856,d0
	move	d0,P61_Period(a5)
	move	d0,6(a4)
	bra.b	P61_contfxdone
.skip
	move	P61_Period(a5),6(a4)
	bra.b	P61_contfxdone
	endc

	ifne	P61_pde
P61_return
	rts

P61_preplay
	tst	P61_pdflag-P61_cn(a3)
	bne.b	P61_return
	else
P61_preplay
	endc

	lea	P61_temp0(pc),a5
	lea	P61_samples-16(pc),a0

	moveq	#channels-1,d5
P61_loaps
	ifne	P61_pl
	lea	P61_TData(a5),a1
	move	2(a5),(a1)+
	move.l	P61_ChaPos(a5),(a1)+
	move.l	P61_TempPos(a5),(a1)+
	move	P61_TempLen(a5),(a1)
	endc

	move.b	P61_Pack(a5),d0
	and.b	#$3f,d0
	beq.b	P61_takeone

	tst.b	P61_Pack(a5)
	bmi.b	.keepsame

	subq.b	#1,P61_Pack(a5)
	clr	P61_OnOff(a5)			; Empty row
	add.l	d6,a5
	dbf	d5,P61_loaps
	rts

.keepsame
	subq.b	#1,P61_Pack(a5)
	bra	P61_dko

P61_takeone
	tst.b	P61_TempLen+1(a5)
	beq.b	P61_takenorm

	subq.b	#1,P61_TempLen+1(a5)
	move.l	P61_TempPos(a5),a2

P61_jedi
	move.b	(a2)+,d0
	moveq	#%01100000,d1
	and.b	d0,d1
	cmp.b	#%01100000,d1
	bne.b	.all

	moveq	#%01110000,d1
	and.b	d0,d1
	cmp.b	#%01110000,d1
	bne.b	.cmd

	moveq	#%01111000,d1
	and.b	d0,d1
	cmp.b	#%01111000,d1
	bne.b	.note

.empty
	clr	P61_OnOff(a5)			; Empty row
	clr	(a5)+
	clr.b	(a5)+
	tst.b	d0
	bpl.b	.ex
	move.b	(a2)+,(a5)			; Compression info
	bra.b	.ex

.all
	move.b	d0,(a5)+
	ifeq	opt020
	move.b	(a2)+,(a5)+
	move.b	(a2)+,(a5)+
	else
	move	(a2)+,(a5)+
	endc
	tst.b	d0
	bpl.b	.ex
	move.b	(a2)+,(a5)			; Compression info
	bra.b	.ex

.cmd
	moveq	#$f,d1
	and	d0,d1
	move	d1,(a5)+			; cmd
	move.b	(a2)+,(a5)+			; info
	tst.b	d0
	bpl	.ex
	move.b	(a2)+,(a5)			; Compression info
	bra.b	.ex

.note
	moveq	#7,d1
	and	d0,d1
	lsl	#8,d1
	move.b	(a2)+,d1
	lsl	#4,d1
	move	d1,(a5)+
	clr.b	(a5)+	
	tst.b	d0
	bpl.b	.ex
	move.b	(a2)+,(a5)			; Compression info
.ex
	subq.l	#3,a5
	move.l	a2,P61_TempPos(a5)
	bra	P61_dko


P61_takenorm
	move.l	P61_ChaPos(a5),a2

	move.b	(a2)+,d0
	moveq	#%01100000,d1
	and.b	d0,d1
	cmp.b	#%01100000,d1
	bne.b	.all

	moveq	#%01110000,d1
	and.b	d0,d1
	cmp.b	#%01110000,d1
	bne.b	.cmd

	moveq	#%01111000,d1
	and.b	d0,d1
	cmp.b	#%01111000,d1
	bne.b	.note

.empty
	clr	P61_OnOff(a5)			; Empty row
	clr	(a5)+
	clr.b	(a5)+
	tst.b	d0
	bpl.b	.proccomp
	move.b	(a2)+,(a5)			; Compression info
	bra.b	.proccomp


.all
	move.b	d0,(a5)+
	ifeq	opt020
	move.b	(a2)+,(a5)+
	move.b	(a2)+,(a5)+
	else
	move	(a2)+,(a5)+
	endc
	tst.b	d0
	bpl.b	.proccomp
	move.b	(a2)+,(a5)			; Compression info
	bra.b	.proccomp

.cmd
	moveq	#$f,d1
	and	d0,d1
	move	d1,(a5)+			; cmd
	move.b	(a2)+,(a5)+			; info
	tst.b	d0
	bpl	.proccomp
	move.b	(a2)+,(a5)			; Compression info
	bra.b	.proccomp

.note
	moveq	#7,d1
	and	d0,d1
	lsl	#8,d1
	move.b	(a2)+,d1
	lsl	#4,d1
	move	d1,(a5)+
	clr.b	(a5)+	
	tst.b	d0
	bpl.b	.proccomp
	move.b	(a2)+,(a5)			; Compression info

.proccomp
	subq.l	#3,a5
	move.l	a2,P61_ChaPos(a5)

	tst.b	d0
	bpl.b	P61_dko

	move.b	3(a5),d0
	move.b	d0,d1
	and	#%11000000,d1
	beq.b	P61_dko				; Empty datas
	cmp.b	#%10000000,d1
	beq.b	P61_dko				; Same datas

	clr.b	3(a5)
	and	#$3f,d0
	move.b	d0,P61_TempLen+1(a5)

	cmp.b	#%11000000,d1
	beq.b	.bit16				; 16-bit

	moveq	#0,d0				; 8-bit
	move.b	(a2)+,d0
	move.l	a2,P61_ChaPos(a5)
	sub.l	d0,a2
	bra	P61_jedi

.bit16
	moveq	#0,d0
	ifeq	opt020
	move.b	(a2)+,d0
	lsl	#8,d0
	move.b	(a2)+,d0
	else
	move	(a2)+,d0
	endc

	move.l	a2,P61_ChaPos(a5)
	sub.l	d0,a2
	bra	P61_jedi


P61_dko
	st	P61_OnOff(a5)
	move	(a5),d0
	and	#$1f0,d0
	beq	.koto
	lea	(a0,d0),a1
	move.l	a1,P61_Sample(a5)
	ifne	P61_ft
	move.l	P61_SampleVolume(a1),P61_Volume(a5)
	else
	move	P61_SampleVolume(a1),P61_Volume(a5)
	endc
	ifne	P61_il
	move.l	P61_RepeatOffset(a1),P61_Wave(a5)
	endc
	ifne	P61_sof
	clr	P61_Offset(a5)
	endc

.koto
	add.l	d6,a5
	dbf	d5,P61_loaps
	rts

P61_playtime
	clr	(a3)

	ifne	P61_pde
	tst	P61_pdelay-P61_cn(a3)
	beq	.djdj
	subq	#1,P61_pdelay-P61_cn(a3)
	bne	P61_delay
	tst	P61_speedis1-P61_cn(a3)
	bne	P61_delay
	clr	P61_pdflag-P61_cn(a3)
	bra	P61_delay
.djdj
	clr	P61_pdflag-P61_cn(a3)
	endc

	tst	P61_speedis1-P61_cn(a3)
	beq.b	.mo
	bsr	P61_preplay

.mo
	lea	P61_temp0(pc),a5
	lea	$a0(a6),a4

	ifeq	system
	lea	P61_dmason(pc),a1
	move.l	P61_vektori(pc),a0
	move.l	a1,(a0)
	move.b	#$f0,$bfd600
	move.b	#$01,$bfd700
	move.b	#$19,$bfdf00
	else
	move	#1,P61_server-P61_cn(a3)
	move.l	P61_craddr+4(pc),a1
	move.b	#$f0,(a1)
	move.b	#1,$100(a1)
	endc

	lea	P61_periods(pc),a2

	moveq	#0,d4
	moveq	#channels-1,d5
P61_los
	tst	P61_OnOff(a5)
	beq.b	P61_nocha

	moveq	#$f,d0
	and	(a5),d0
	lea	P61_jtab(pc),a1
	add	d0,d0
	add.l	d0,a1
	add	(a1),a1
	jmp	(a1)

P61_fxdone
	moveq	#$7e,d0
	and.b	(a5),d0
	beq.b	P61_nocha
	ifne	P61_vib
	clr.b	P61_VibPos(a5)
	endc
	ifne	P61_tre
	clr.b	P61_TrePos(a5)
	endc

 	ifne	P61_ft
	add	P61_Fine(a5),d0
	endc
	move	d0,P61_Note(a5)
	move	(a2,d0),P61_Period(a5)

P61_zample
	ifne	P61_sof
	tst	P61_Offset(a5)
	bne	P61_pek
	endc

	or	P61_DMABit(a5),d4
	move	d4,$96(a6)
	move.l	P61_Sample(a5),a1		;* Trigger *
	move.l	(a1)+,(a4)			;Pointer
	move	(a1),4(a4)			;Length

P61_nocha
	ifeq	fade
	move.l	P61_Period(a5),6(a4)
	else
	move	P61_Period(a5),6(a4)
	move	P61_Volume(a5),P61_Shadow(a5)
	endc

P61_skip
	ifne	P61_il
	bsr	P61_funk2
	endc

	add.l	d6,a5
	add.l	d7,a4
	dbf	d5,P61_los

	move.b	d4,P61_dma+1-P61_cn(a3)

	ifne	P61_pl
	tst.b	P61_plflag+1-P61_cn(a3)
	beq.b	P61_ohittaa

	lea	P61_temp0(pc),a1
	lea	P61_looppos(pc),a0
	moveq	#channels-1,d0
.talt
	move.b	1(a0),3(a1)
	addq.l	#2,a0
	move.l	(a0)+,P61_ChaPos(a1)
	move.l	(a0)+,P61_TempPos(a1)
	move	(a0)+,P61_TempLen(a1)
	add.l	d6,a1
	dbf	d0,.talt

	move	P61_plrowpos(pc),P61_rowpos-P61_cn(a3)
	clr.b	P61_plflag+1-P61_cn(a3)
	moveq	#63,d0
	sub	P61_rowpos-P61_cn(a3),d0
	move	d0,P61_CRow-P61_cn(a3)
	rts
	endc

P61_ohittaa
	subq	#1,P61_rowpos-P61_cn(a3)
	bmi.b	P61_nextpattern
	moveq	#63,d0
	sub	P61_rowpos-P61_cn(a3),d0
	move	d0,P61_CRow-P61_cn(a3)
	rts

P61_nextpattern
	ifne	P61_pl
	clr	P61_plflag-P61_cn(a3)
	endc
	move.l	P61_patternbase(pc),a4
	moveq	#63,d0
	move	d0,P61_rowpos-P61_cn(a3)
	clr	P61_CRow-P61_cn(a3)
	move.l	P61_spos(pc),a1
	addq	#1,P61_Pos-P61_cn(a3)
	move.b	(a1)+,d0
	bpl.b	P61_dk
	move.l	P61_possibase(pc),a1
	move.b	(a1)+,d0
	clr	P61_Pos-P61_cn(a3)
P61_dk
	move.l	a1,P61_spos-P61_cn(a3)
	move	d0,P61_Patt-P61_cn(a3)
	lsl	#3,d0
	move.l	P61_positionbase(pc),a1
	add.l	d0,a1

	move	(a1)+,d0
	lea	(a4,d0.l),a2
	move.l	a2,P61_ChaPos+P61_temp0-P61_cn(a3)
	move	(a1)+,d0
	lea	(a4,d0.l),a2
	move.l	a2,P61_ChaPos+P61_temp1-P61_cn(a3)
	move	(a1)+,d0
	lea	(a4,d0.l),a2
	move.l	a2,P61_ChaPos+P61_temp2-P61_cn(a3)
	move	(a1),d0
	add.l	d0,a4
	move.l	a4,P61_ChaPos+P61_temp3-P61_cn(a3)
	rts

	ifne	P61_tp
P61_settoneport
	move.b	P61_Info(a5),d0
	beq.b	P61_toponochange
	move.b	d0,P61_TPSpeed+1(a5)
P61_toponochange
	moveq	#$7e,d0
	and.b	(a5),d0
	beq	P61_nocha
	add	P61_Fine(a5),d0
	move	d0,P61_Note(a5)
	move	(a2,d0),P61_ToPeriod(a5)
	bra	P61_nocha
	endc

	ifne	P61_sof
P61_sampleoffse
	moveq	#0,d1
	move	#$ff00,d1
	and	2(a5),d1
	bne.b	.deq
	move	P61_LOffset(a5),d1
.deq
	move	d1,P61_LOffset(a5)
	add	d1,P61_Offset(a5)

	moveq	#$7e,d0
	and.b	(a5),d0
	beq	P61_nocha

	move	P61_Offset(a5),d2
	add	d1,P61_Offset(a5)		; THIS IS A PT-FEATURE!
	move	d2,d1

	ifne	P61_vib
	clr.b	P61_VibPos(a5)
	endc
	ifne	P61_tre
	clr.b	P61_TrePos(a5)
	endc

	ifne	P61_ft
	add	P61_Fine(a5),d0
	endc
	move	d0,P61_Note(a5)
	move	(a2,d0),P61_Period(a5)
	bra.b	P61_hup

P61_pek
	moveq	#0,d1
	move	P61_Offset(a5),d1
P61_hup
	or	P61_DMABit(a5),d4
	move	d4,$96(a6)
	move.l	P61_Sample(a5),a1		;* Trigger *
	move.l	(a1)+,d0
	add.l	d1,d0
	move.l	d0,(a4)				;Pointer
	lsr	#1,d1
	move	(a1),d0
	sub	d1,d0
	bpl.b	P61_offok
	move.l	-4(a1),(a4)			;Pointer is over the end
	moveq	#1,d0
P61_offok
	move	d0,4(a4)			;Length
	bra	P61_nocha
	endc

	ifne	P61_vl
P61_volum
	move.b	P61_Info(a5),P61_Volume+1(a5)
	bra	P61_fxdone
	endc

	ifne	P61_pj
P61_posjmp
	moveq	#0,d0
	move.b	P61_Info(a5),d0
	cmp	P61_slen-P61_cn(a3),d0
	blo.b	.e
	moveq	#0,d0
.e	move	d0,P61_Pos-P61_cn(a3)
	add.l	P61_possibase(pc),d0
	move.l	d0,P61_spos-P61_cn(a3)
	endc

	ifne	P61_pb
P61_pattbreak
	moveq	#64,d0
	move	d0,P61_rowpos-P61_cn(a3)
	clr	P61_CRow-P61_cn(a3)
	move.l	P61_spos(pc),a1
	move.l	P61_patternbase(pc),a0
	addq	#1,P61_Pos-P61_cn(a3)
	move.b	(a1)+,d0
	bpl.b	P61_dk2
	move.l	P61_possibase(pc),a1
	move.b	(a1)+,d0
	clr	P61_Pos-P61_cn(a3)
P61_dk2
	move.l	a1,P61_spos-P61_cn(a3)
	move.l	P61_positionbase(pc),a1
	move	d0,P61_Patt-P61_cn(a3)
	lsl	#3,d0
	add.l	d0,a1
	movem	(a1),d0-d3
	lea	(a0,d0.l),a1
	move	d1,d0
	move.l	a1,P61_ChaPos+P61_temp0-P61_cn(a3)
	lea	(a0,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp1-P61_cn(a3)
	move	d2,d0
	lea	(a0,d0.l),a1
	move.l	a1,P61_ChaPos+P61_temp2-P61_cn(a3)
	move	d3,d0
	add.l	d0,a0
	move.l	a0,P61_ChaPos+P61_temp3-P61_cn(a3)
	bra	P61_fxdone
	endc

	ifne	P61_vib
P61_vibrato
	move.b	P61_Info(a5),d0
	beq	P61_fxdone
	move.b	d0,d1
	move.b	P61_VibCmd(a5),d2
	and.b	#$f,d0
	beq.b	P61_vibskip
	and.b	#$f0,d2
	or.b	d0,d2
P61_vibskip
	and.b	#$f0,d1
	beq.b	P61_vibskip2
	and.b	#$f,d2
	or.b	d1,d2
P61_vibskip2
	move.b	d2,P61_VibCmd(a5)
	bra	P61_fxdone
	endc

	ifne	P61_tre
P61_settremo
	move.b	P61_Info(a5),d0
	beq	P61_fxdone
	move.b	d0,d1
	move.b	P61_TreCmd(a5),d2
	moveq	#$f,d3
	and.b	d3,d0
	beq.b	P61_treskip
	and.b	#$f0,d2
	or.b	d0,d2
P61_treskip
	and.b	#$f0,d1
	beq.b	P61_treskip2
	and.b	d3,d2
	or.b	d1,d2
P61_treskip2
	move.b	d2,P61_TreCmd(a5)
	bra	P61_fxdone
	endc

	ifne	P61_ec
P61_ecommands
	move.b	P61_Info(a5),d0
	and.b	#$f0,d0
	lsr	#3,d0
	move	P61_etab(pc,d0),d0
	jmp	P61_etab(pc,d0)

P61_etab
	ifne	P61_fi
	dc	P61_filter-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_fsu
	dc	P61_fineup-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_fsd
	dc	P61_finedwn-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	dc	P61_fxdone-P61_etab
	dc	P61_fxdone-P61_etab

	ifne	P61_sft
	dc	P61_setfinetune-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_pl
	dc	P61_patternloop-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	dc	P61_fxdone-P61_etab

	ifne	P61_timing
	dc	P61_sete8-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_rt
	dc	P61_setretrig-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_fvu
	dc	P61_finevup-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_fvd
	dc	P61_finevdwn-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	dc	P61_fxdone-P61_etab

	ifne	P61_nd
	dc	P61_ndelay-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_pde
	dc	P61_pattdelay-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc

	ifne	P61_il
	dc	P61_funk-P61_etab
	else
	dc	P61_fxdone-P61_etab
	endc
	endc

	ifne	P61_fi
P61_filter
	move.b	P61_Info(a5),d0
	and.b	#$fd,$bfe001
	or.b	d0,$bfe001
	bra	P61_fxdone
	endc

	ifne	P61_fsu
P61_fineup
	P61_getnote

	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	sub	d0,P61_Period(a5)
	moveq	#113,d0
	cmp	P61_Period(a5),d0
	ble.b	.jup
	move	d0,P61_Period(a5)
.jup
	moveq	#$7e,d0
	and.b	(a5),d0
	bne	P61_zample
	bra	P61_nocha
	endc

	ifne	P61_fsd
P61_finedwn
	P61_getnote

	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	add	d0,P61_Period(a5)
	cmp	#856,P61_Period(a5)
	ble.b	.jup
	move	#856,P61_Period(a5)
.jup	moveq	#$7e,d0
	and.b	(a5),d0
	bne	P61_zample
	bra	P61_nocha
	endc

	ifne	P61_sft
P61_setfinetune
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	ifeq	opt020
	add	d0,d0
	move	P61_mulutab(pc,d0),P61_Fine(a5)
	else
	move	P61_mulutab(pc,d0*2),P61_Fine(a5)
	endc
	bra	P61_fxdone

P61_mulutab
	dc	0,74,148,222,296,370,444,518,592,666,740,814,888,962,1036,1110
	endc

	ifne	P61_pl
P61_patternloop
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	beq.b	P61_setloop

	tst.b	P61_plflag-P61_cn(a3)
	bne.b	P61_noset

	move	d0,P61_plcount-P61_cn(a3)
	st.b	P61_plflag-P61_cn(a3)
P61_noset
	tst	P61_plcount-P61_cn(a3)
	bne.b	P61_looppaa
	clr.b	P61_plflag-P61_cn(a3)
	bra	P61_fxdone
	
P61_looppaa
	st.b	P61_plflag+1-P61_cn(a3)
	subq	#1,P61_plcount-P61_cn(a3)
	bra	P61_fxdone

P61_setloop
	tst.b	P61_plflag-P61_cn(a3)
	bne	P61_fxdone
	move	P61_rowpos(pc),P61_plrowpos-P61_cn(a3)
	lea	P61_temp0+P61_TData(pc),a1
	lea	P61_looppos(pc),a0
	moveq	#channels-1,d0
.talt
	move.l	(a1)+,(a0)+
	move.l	(a1)+,(a0)+
	move.l	(a1),(a0)+
	subq.l	#8,a1
	add.l	d6,a1
	dbf	d0,.talt
	bra	P61_fxdone
	endc

	ifne	P61_fvu
P61_finevup
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	add	d0,P61_Volume(a5)
	moveq	#64,d0
	cmp	P61_Volume(a5),d0
	bge	P61_fxdone
	move	d0,P61_Volume(a5)
	bra	P61_fxdone
	endc

	ifne	P61_fvd
P61_finevdwn
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	sub	d0,P61_Volume(a5)
	bpl	P61_fxdone
	clr	P61_Volume(a5)
	bra	P61_fxdone
	endc

	ifne	P61_timing
P61_sete8
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	move	d0,P61_E8-P61_cn(a3)
	bra	P61_fxdone
	endc

	ifne	P61_rt
P61_setretrig
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	move	d0,P61_RetrigCount(a5)
	bra	P61_fxdone
	endc

	ifne	P61_nd
P61_ndelay
	moveq	#$7e,d0
	and.b	(a5),d0
	beq	P61_skip
	ifne	P61_vib
	clr.b	P61_VibPos(a5)
	endc
	ifne	P61_tre
	clr.b	P61_TrePos(a5)
	endc
	ifne	P61_ft
	add	P61_Fine(a5),d0
	endc
	move	d0,P61_Note(a5)
	move	(a2,d0),P61_Period(a5)
	ifeq	fade
	move	P61_Volume(a5),8(a4)
	else
	move	P61_Volume(a5),P61_Shadow(a5)
	endc
	bra	P61_skip
	endc

	ifne	P61_pde
P61_pattdelay
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	move	d0,P61_pdelay-P61_cn(a3)
	st	P61_pdflag-P61_cn(a3)
	bra	P61_fxdone
	endc

	ifne	P61_sd
P61_cspeed
	moveq	#0,d0
	move.b	P61_Info(a5),d0

	ifne	CIA
	tst	P61_Tempo-P61_cn(a3)
	beq.b	P61_VBlank
	cmp.b	#32,d0
	bhs.b	P61_STempo
	endc

P61_VBlank
	cmp.b	#1,d0
	beq.b	P61_jkd

	move.b	d0,P61_speed+1-P61_cn(a3)
	subq.b	#1,d0
	move.b	d0,P61_speed2+1-P61_cn(a3)
	clr	P61_speedis1-P61_cn(a3)
	bra	P61_fxdone

P61_jkd
	move.b	d0,P61_speed+1-P61_cn(a3)
	move.b	d0,P61_speed2+1-P61_cn(a3)
	st	P61_speedis1-P61_cn(a3)
	bra	P61_fxdone


	ifne	CIA
P61_STempo
	move.l	P61_timer(pc),d1
	divu	d0,d1
	move	d1,P61_thi2-P61_cn(a3)
	sub	#$1f0*2,d1
	move	d1,P61_thi-P61_cn(a3)

	ifeq	system
	move	P61_thi2-P61_cn(a3),d1
	move.b	d1,$bfd400
	lsr	#8,d1
	move.b	d1,$bfd500
	endc

	bra	P61_fxdone
	endc
	endc



	ifne	P61_vbvs
P61_vibochvslide
	move.b	P61_Info(a5),d0
	sub.b	d0,P61_Volume+1(a5)
	bpl.b	P61_test62
	clr	P61_Volume(a5)
	ifeq	fade
	clr	8(a4)
	else
	clr	P61_Shadow(a5)
	endc
	bra.b	P61_vib2
P61_test62
	moveq	#64,d0
	cmp	P61_Volume(a5),d0
	bge.b	.ncs2
	move	d0,P61_Volume(a5)
.ncs2
	ifeq	fade
	move	P61_Volume(a5),8(a4)
	else
	move	P61_Volume(a5),P61_Shadow(a5)
	endc
	endc

	ifne	P61_vib
P61_vib2
	move	#$f00,d0
	move	P61_VibCmd(a5),d1
	and	d1,d0
	lsr	#3,d0

	lsr	#2,d1
	and	#$1f,d1
	add	d1,d0

	move	P61_Period(a5),d1
	moveq	#0,d2
	move.b	P61_vibtab(pc,d0),d2

	tst.b	P61_VibPos(a5)
	bmi.b	.vibneg
	add	d2,d1
	bra.b	P61_vib4

.vibneg	sub	d2,d1

P61_vib4
	move	d1,6(a4)
	move.b	P61_VibCmd(a5),d0
	lsr.b	#2,d0
	and	#$3c,d0
	add.b	d0,P61_VibPos(a5)
	bra	P61_contfxdone
	endc

	ifne	P61_tre
P61_tremo
	move	#$f00,d0
	move	P61_TreCmd(a5),d1
	and	d1,d0
	lsr	#3,d0
	
	lsr	#2,d1
	and	#$1f,d1
	add	d1,d0

	move	P61_Volume(a5),d1
	moveq	#0,d2
	move.b	P61_vibtab(pc,d0),d2

	tst.b	P61_TrePos(a5)
	bmi.b	.treneg
	add	d2,d1
	cmp	#64,d1
	ble.b	P61_tre4
	moveq	#64,d1
	bra.b	P61_tre4

.treneg
	sub	d2,d1
	bpl.b	P61_tre4
	moveq	#0,d1
P61_tre4
	ifeq	fade
	move	d1,8(a4)
	else
	move	d1,P61_Shadow(a5)
	endc

	move.b	P61_TreCmd(a5),d0
	lsr.b	#2,d0
	and	#$3c,d0
	add.b	d0,P61_TrePos(a5)
	bra	P61_contfxdone
	endc

	ifne	P61_vib!P61_tre
P61_vibtab
	incbin	vibtab
	endc

	ifne	P61_il
P61_funk
	moveq	#$f,d0
	and.b	P61_Info(a5),d0
	move.b	d0,P61_Funkspd(a5)
	bra	P61_fxdone

P61_funk2
	moveq	#0,d0
	move.b	P61_Funkspd(a5),d0
	beq.b	P61_funkend
	move.b	P61_FunkTable(pc,d0),d0
	add.b	d0,P61_Funkoff(a5)
	bpl.b	P61_funkend
	clr.b	P61_Funkoff(a5)

	move.l	P61_Sample(a5),a1
	move.l	P61_RepeatOffset(a1),d1
	move	P61_RepeatLength(a1),d0
	add.l	d0,d0
	add.l	d1,d0
	move.l	P61_Wave(a5),a0
	addq.l	#1,a0
	cmp.l	d0,a0
	blo.b	P61_funkok
	move.l	d1,a0
P61_funkok
	move.l	a0,P61_Wave(a5)
	not.b	(a0)
P61_funkend
	rts

P61_FunkTable
	dc.b 0,5,6,7,8,10,11,13,16,19,22,26,32,43,64,128
	endc

P61_jtab
	dc	P61_fxdone-*
	dc	P61_fxdone-*
	dc	P61_fxdone-*

	ifne	P61_tp
	dc	P61_settoneport-*
	else
	dc	P61_fxdone-*
	endc

	ifne	P61_vib
	dc	P61_vibrato-*
	else
	dc	P61_fxdone-*
	endc

	ifne	P61_tpvs
	dc	P61_toponochange-*
	else
	dc	P61_fxdone-*
	endc

	dc	P61_fxdone-*

	ifne	P61_tre
	dc	P61_settremo-*
	else
	dc	P61_fxdone-*
	endc

	dc	P61_fxdone-*

	ifne	P61_sof
	dc	P61_sampleoffse-*
	else
	dc	P61_fxdone-*
	endc
	dc	P61_fxdone-*

	ifne	P61_pj
	dc	P61_posjmp-*
	else
	dc	P61_fxdone-*
	endc

	ifne	P61_vl
	dc	P61_volum-*
	else
	dc	P61_fxdone-*
	endc

	ifne	P61_pb
	dc	P61_pattbreak-*
	else
	dc	P61_fxdone-*
	endc

	ifne	P61_ec
	dc	P61_ecommands-*
	else
	dc	P61_fxdone-*
	endc
	
	ifne	P61_sd
	dc	P61_cspeed-*
	else
	dc	P61_fxdone-*
	endc


P61_dmason
	ifeq	system
	tst.b	$bfdd00
	move	#$2000,$dff09c

;---------------------------------------------
* BUGFIX - 610.3 / 27.04.98 - done by NoName
; code added in 610.3:
	move	#$2000,$dff09c
;---------------------------------------------

	move.b	#$19,$bfdf00
	move.l	a0,-(sp)
	move.l	P61_vektori(pc),a0
	move.l	P61_intaddr(pc),(a0)
	move.l	(sp)+,a0
	move	P61_dma(pc),$dff096
	nop
	rte

	else

	move	P61_dma(pc),$96(a6)
	lea	P61_server(pc),a3
	addq	#1,(a3)
	move.l	P61_craddr(pc),a0
	move.b	#$19,(a0)
	bra	P61_ohi
	endc


P61_setrepeat
	ifeq	system
	tst.b	$bfdd00
	movem.l	a0/a1,-(sp)
	lea	$dff0a0,a1
	move	#$2000,-4(a1)

;---------------------------------------------
* BUGFIX - 610.3 / 27.04.98 - done by NoName
; code added in 610.3:
	move	#$2000,-4(a1)
;---------------------------------------------

	else
	lea	$a0(a6),a1
	endc

	move.l	P61_Sample+P61_temp0(pc),a0
	addq.l	#6,a0
	move.l	(a0)+,(a1)+
	move	(a0),(a1)

	ifgt	channels-1
	move.l	P61_Sample+P61_temp1(pc),a0
	addq.l	#6,a0
	move.l	(a0)+,12(a1)
	move	(a0),16(a1)
	endc
	
	ifgt	channels-2
	move.l	P61_Sample+P61_temp2(pc),a0
	addq.l	#6,a0
	move.l	(a0)+,28(a1)
	move	(a0),32(a1)
	endc

	ifgt	channels-3
	move.l	P61_Sample+P61_temp3(pc),a0
	addq.l	#6,a0
	move.l	(a0)+,44(a1)
	move	(a0),48(a1)
	endc

	ifne	system
	ifne	CIA
	lea	P61_server(pc),a3
	clr	(a3)
	move.l	P61_craddr+4(pc),a0
	move.b	P61_tlo(pc),(a0)
	move.b	P61_thi(pc),$100(a0)
	endc
	bra	P61_ohi
	endc

	ifeq	system
	ifne	CIA
	move.l	P61_vektori(pc),a0
	move.l	P61_tintti(pc),(a0)
	endc
	movem.l	(sp)+,a0/a1
	nop
	rte
	endc

P61_temp0
	dcb.b	Channel_Block_SIZE-2,0
	dc	1

P61_temp1
	dcb.b	Channel_Block_SIZE-2,0
	dc	2

P61_temp2
	dcb.b	Channel_Block_SIZE-2,0
	dc	4

P61_temp3
	dcb.b	Channel_Block_SIZE-2,0
	dc	8

P61_cn
	dc	0
P61_dma		
	dc	$8200
P61_rowpos
	dc	0
P61_slen
	dc	0
P61_speed
	dc	0
P61_speed2
	dc	0
P61_speedis1
	dc	0
P61_spos
	dc.l	0

	ifeq	system
P61_vektori
	dc.l	0
P61_oldlev6
	dc.l	0
	endc

P61_ofilter
	dc	0
P61_timers
	dc.l	0

	ifne	CIA
P61_tintti
	dc.l	0
P61_thi
	dc.b	0
P61_tlo	
	dc.b	0
P61_thi2
	dc.b	0
P61_tlo2
	dc.b	0
P61_timer
	dc.l	0
	endc

	ifne	P61_pl
P61_plcount
	dc	0
P61_plflag
	dc	0
P61_plreset
	dc	0
P61_plrowpos
	dc	0
P61_looppos
	dcb.b	12*channels,0
	endc

	ifne	P61_pde
P61_pdelay
	dc	0
P61_pdflag
	dc	0
	endc

P61_samples
	dcb.b	16*31,0
P61_emptysample
	dcb.b	16,0
P61_positionbase
	dc.l	0
P61_possibase
	dc.l	0
P61_patternbase
	dc.l	0
P61_intaddr
	dc.l	0

	ifne	system
P61_server
	dc	0
P61_miscbase
	dc.l	0
P61_audioopen
	dc.b	0
P61_sigbit
	dc.b	-1
P61_ciares
	dc.l	0
P61_craddr
	dc.l	0,0,0
P61_dat
		dc	$f00
P61_timerinterrupt
	dc	0,0,0,0,127
P61_timerdata
	dc.l	0,0,0
P61_timeron
	dc	0
P61_allocport
	dc.l	0,0
	dc.b	4,0
	dc.l	0
	dc.b	0,0
	dc.l	0
P61_reqlist
	dc.l	0,0,0
	dc.b	5,0
P61_allocreq
	dc.l	0,0
	dc	127
	dc.l	0
P61_portti
	dc.l	0
	dc	68
	dc.l	0,0,0
	dc	0
P61_reqdata
	dc.l	0
	dc.l	1,0,0,0,0,0,0
	dc	0
P61_audiodev
	dc.b	'audio.device',0

P61_cianame
	dc.b	'ciab.resource',0
P61_timeropen
	dc.b	0
P61_timerint
	dc.b	'P61_TimerInterrupt',0,0
	endc
P61_etu

******** END OF BINARY FILE **************

	ifne	example
	section	chip,data_c
P61_data
	incbin	"tempest-acidjazzed_evening.p61"

; uncomment if samples are external to the module
;P61_smp	incbin	"ram:SMP.fields of green"

;	section	smp,bss_c
;samples
        ;ds.b   $10000          ;uncomment if you have packed samples
                                ;and insert sample buffer length
                                ;also go to the line where samples
                                ;is loaded into a2 and uncomment it
	endc
	
	ifne    asmone
	ifd     makebin
	auto    wb ram:610.6.bin_new\P61_motuuli\P61_etu\
	endc
	endc
	END

