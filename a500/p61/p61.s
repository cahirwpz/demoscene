; vim: ft=asm68k:ts=8:sw=8:

usecode = -1	;CHANGE! to the USE hexcode from P61con for a big 
		;CPU-time gain! (See module usecodes at end of source)
		;Multiple songs, single playroutine? Just "OR" the 
		;usecodes together!

		;...STOP! Have you changed it yet!? ;)
		;You will LOSE RASTERTIME AND FEATURES if you don't.

P61pl = usecode & $400000

split4 = 0	;Great time gain, but INCOMPATIBLE with F03, F02, and F01
		;speeds in the song! That's the ONLY reason it's default 0.
		;So ==> PLEASE try split4=1 in ANY mode!
		;Overrides splitchans to decrunch 1 chan/frame.
		;See ;@@ note for P61_SetPosition.


splitchans = 1	;#channels to be split off to be decrunched at "playtime frame"
		;0=use normal "decrunch all channels in the same frame"
		;Experiment to find minimum rastertime, but it should be 1 or 2
		;for 3-4 channels songs and 0 or 1 with less channels.

visuctrs = 1	;enables visualizers in this example: P61_visuctr0..3.w 
		;containing #frames (#lev6ints if cia=1) elapsed since last
		;instrument triggered. (0=triggered this frame.)
		;Easy alternative to E8x or 1Fx sync commands.

asmonereport = 0	;ONLY for printing a settings report on assembly. Use
			;if you get problems (only works in AsmOne/AsmPro, tho)

p61system = 0	;1=system-friendly. Use for DOS/Workbench programs.

p61exec = 1	;0 if execbase is destroyed, such as in a trackmo.

p61fade = 0	;enable channel volume fading from your demo

channels = 4	;<4 for game sound effects in the higher channels. Incompatible
		; with splitchans/split4.

playflag = 1	;1=enable music on/off capability (at run-time). .If 0, you can
		;still do this by just, you know, not calling P61_Music...
		;It's a convenience function to "pause" music in CIA mode.

p61bigjtab = 0	;1 to waste 480b and save max 56 cycles on 68000.

opt020 = 0	;1=enable optimizations for 020+. Please be 68000 compatible!
		;splitchans will already give MUCH bigger gains, and you can
		;try the MAXOPTI mode.

p61jump = 1	;0 to leave out P61_SetPosition (size gain)
		;1 if you need to force-start at a given position fex in a game

C = 0	        ;If you happen to have some $dffxxx value in a6, you can 
		;change this to $xxx to not have to load it before P61_Music.

clraudxdat = 0	;enable smoother start of quiet sounds. probably not needed.

optjmp = 1	;0=safety check for jump beyond end of song. Clear it if you 
		;play unknown P61 songs with erroneous Bxx/Dxx commands in them

oscillo = 1	;1 to get a sample window (ptr, size) to read and display for 
		;oscilloscope type effects (beta, noshorts=1, pad instruments)
		;IMPORTANT: see ;@@ note about chipmem dc.w buffer.

quietstart = 0	;attempt to avoid the very first click in some modules
		;IMPORTANT: see ;@@ note about chipmem dc.w buffer.

use1Fx = 0	;Optional extra effect-sync trigger (*). If your module is free
		;from E commands, and you add E8x to sync stuff, this will 
		;change the usecode to include a whole code block for all E 
		;commands. You can avoid this by only using 1Fx. (You can 
		;also use this as an extra sync command if E8x is not enough, 
		;of course.)

;(*) Slideup values>116 causes bugs in Protracker, and E8 causes extra-code 
;for all E-commands, so I used this. It's only faster if your song contains 0
;E-commands, so it's only useful to a few, I guess. Bit of cyclemania. :)

;Just like E8x, you will get the trigger after the P61_Music call, 1 frame 
;BEFORE it's heard. This is good, because it allows double-buffered graphics 
;or effects running at < 50 fps to show the trigger synced properly.

p61cia = 1	;call P61_Music on the CIA interrupt instead of every frame.

lev6 = 1	;1=keep the timer B int at least for setting DMA.
		;0="FBI mode" - ie. "Free the B-timer Interrupt".

		;0 requires noshorts=1, p61system=0, and that YOU make sure DMA
		;is set at 11 scanlines (700 usecs) after P61_Music is called.
		;AsmOne will warn you if requirements are wrong.

		;DMA bits will be poked in the address you pass in A4 to 
		;P61_init. (Update P61_DMApokeAddr during playing if necessary,
		;for example if switching Coppers.)

		;P61_Init will still save old timer B settings, and initialize
		;it. P61_End will still restore timer B settings from P61_Init.
		;So don't count on it 'across calls' to these routines.
		;Using it after P61_Init and before P61_End is fine.

noshorts = 0	;1 saves ~1 scanline, requires Lev6=0. Use if no instrument is
		;shorter than ~300 bytes (or extend them to > 300 bytes).
		;It does this by setting repeatpos/length the next frame 
		;instead of after a few scanlines,so incompatible with MAXOPTI

dupedec = 0	;0=save 500 bytes and lose 26 cycles - I don't blame you. :)
		;1=splitchans or split4 must be on.

suppF01 = 1	;0 is incompatible with CIA mode. It moves ~100 cycles of
		;next-pattern code to the less busy 2nd frame of a notestep.
		;If you really need it, you have to experiment as the support 
		;is quite complex. Basically set it to 1 and try the various 
		;P61modes, if none work, change some settings.

********** CODE START **********

	xdef _P61_Init, _P61_SetPosition, _P61_Osc, _P61_End
        xdef _P61_ControlBlock, _P61_visuctr

	section	"P6111",code

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

_P61_Osc
        movem.l d2-d4/a2-a3,-(sp)
        bsr     P61_osc
        bne.s   .ready
        moveq.l #0,d0
        bra.s   .quit

.ready
        move.l  a2,(a3)+
        move.l  a1,(a3)+
        move.l  d2,(a3)+
        move.w  d0,(a3)+
        move.w  d1,(a3)+
        move.w  d4,(a3)+
        moveq.l #1,d0
.quit
        movem.l (sp)+,d2-d4/a2-a3
        rts

_P61_End
	movem.l a3/a6,-(sp)
	lea	$dff000,a6
	bsr	P61_End
	movem.l	(sp)+,a3/a6
	rts

	include "P6111-Play.i"
