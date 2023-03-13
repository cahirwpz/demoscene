        section '.bsschip', bss
        xdef _Samples

_Samples:
        ds.w AK_SMP_LEN/2

        section '.text', code
        xdef _AK_Generate

_AK_Generate:
        movem.l d2-d7/a2-a6,-(sp)
        lea     _Samples,a0
        sub.l   a2,a2
        lea     _AK_Progress,a3
        bsr     AK_Generate
        movem.l (sp)+,d2-d7/a2-a6
        rts

_AK_Progress:
        ds.l    1

;----------------------------------------------------------------------------
;
; Generated with Aklang2Asm V1.1, by Dan/Lemon. 2021-2022.
;
; Based on Alcatraz Amigaklang rendering core. (c) Jochen 'Virgill' Feldkötter 2020.
;
; What's new in V1.1?
; - Instance offsets fixed in ADSR operator
; - Incorrect shift direction fixed in OnePoleFilter operator
; - Loop Generator now correctly interleaved with instrument generation
; - Fine progress includes loop generation, and new AK_FINE_PROGRESS_LEN added
; - Reverb large buffer instance offsets were wrong, causing potential buffer overrun
;
; Call 'AK_Generate' with the following registers set:
; a0 = Sample Buffer Start Address
; a1 = 32768 Bytes Temporary Work Buffer Address (can be freed after sample rendering complete)
; a2 = External Samples Address (need not be in chip memory, and can be freed after sample rendering complete)
; a3 = Rendering Progress Address (2 modes available... see below)
;
; AK_FINE_PROGRESS equ 0 = rendering progress as a byte (current instrument number)
; AK_FINE_PROGRESS equ 1 = rendering progress as a long (current sample byte)
;
;----------------------------------------------------------------------------

AK_USE_PROGRESS			equ 1
AK_FINE_PROGRESS		equ 1
AK_FINE_PROGRESS_LEN	equ 200117
AK_SMP_LEN				equ 155040
AK_EXT_SMP_LEN			equ 0

AK_Generate:

				lea		AK_Vars(pc),a5

				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						move.b	#-1,(a3)
					else
						move.l	#0,(a3)
					endif
				endif

				; Create sample & external sample base addresses
				lea		AK_SmpLen(a5),a6
				lea		AK_SmpAddr(a5),a4
				move.l	a0,d0
				moveq	#31-1,d7
.SmpAdrLoop		move.l	d0,(a4)+
				add.l	(a6)+,d0
				dbra	d7,.SmpAdrLoop
				move.l	a2,d0
				moveq	#8-1,d7
.ExtSmpAdrLoop	move.l	d0,(a4)+
				add.l	(a6)+,d0
				dbra	d7,.ExtSmpAdrLoop

;----------------------------------------------------------------------------
; Instrument 1 - colombia_kick
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst1Loop
				; v2 = envd(0, 10, 0, 127)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#645120,d5
				bgt.s   .EnvDNoSustain_1_1
				moveq	#0,d5
.EnvDNoSustain_1_1
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#127,d1
				asr.l	#7,d1

				; v3 = mul(v2, 1024)
				move.w	d1,d2
				muls	#1024,d2
				add.l	d2,d2
				swap	d2

				; v3 = add(v3, -300)
				add.w	#-300,d2
				bvc.s	.AddNoClamp_1_3
				spl		d2
				ext.w	d2
				eor.w	#$7fff,d2
.AddNoClamp_1_3

				; v1 = osc_sine(3, v3, 128)
				add.w	d2,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				sub.w	#16384,d0
				move.w	d0,d5
				bge.s	.SineNoAbs_1_4
				neg.w	d5
.SineNoAbs_1_4
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d0
				swap	d0
				asl.w	#3,d0

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				; v2 = envd(5, 2, 0, 128)
				move.l	AK_EnvDValue+4(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#8388352,d5
				bgt.s   .EnvDNoSustain_1_6
				moveq	#0,d5
.EnvDNoSustain_1_6
				move.l	d5,AK_EnvDValue+4(a5)

				; v3 = osc_saw(6, 2304, 128)
				add.w	#2304,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d2

				; v3 = mul(v2, v3)
				muls	d1,d2
				add.l	d2,d2
				swap	d2

				; v1 = add(v1, v3)
				add.w	d2,d0
				bvc.s	.AddNoClamp_1_9
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_1_9

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+0(a5),d7
				blt		.Inst1Loop

;----------------------------------------------------------------------------
; Instrument 2 - colombia_snare
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst2Loop
				; v3 = envd(0, 16, 8, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d2
				swap	d2
				sub.l	#253952,d5
				cmp.l	#134217728,d5
				bgt.s   .EnvDNoSustain_2_1
				move.l	#134217728,d5
.EnvDNoSustain_2_1
				move.l	d5,AK_EnvDValue+0(a5)

				; v4 = envd(1, 9, 24, 128)
				move.l	AK_EnvDValue+4(a5),d5
				move.l	d5,d3
				swap	d3
				sub.l	#762368,d5
				cmp.l	#402653184,d5
				bgt.s   .EnvDNoSustain_2_2
				move.l	#402653184,d5
.EnvDNoSustain_2_2
				move.l	d5,AK_EnvDValue+4(a5)

				; v1 = osc_saw(2, v2, 128)
				add.w	d1,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0

				; v1 = mul(v1, v3)
				muls	d2,d0
				add.l	d0,d0
				swap	d0

				; v1 = sv_flt_n(4, v1, 12, 16, 1)
				move.w	AK_OpInstance+AK_BPF+2(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#12,d5
				move.w	AK_OpInstance+AK_LPF+2(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_2_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_2_5
				move.w	d4,AK_OpInstance+AK_LPF+2(a5)
				asl.w	#4,d6
				ext.l	d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_2_5
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_2_5
.NoClampMaxHPF_2_5
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_2_5
				move.w	#-32768,d5
.NoClampMinHPF_2_5
				move.w	d5,AK_OpInstance+AK_HPF+2(a5)
				asr.w	#7,d5
				muls	#12,d5
				add.w	AK_OpInstance+AK_BPF+2(a5),d5
				bvc.s	.NoClampBPF_2_5
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_2_5
				move.w	d5,AK_OpInstance+AK_BPF+2(a5)
				move.w	AK_OpInstance+AK_HPF+2(a5),d0

				; v3 = cmb_flt_n(5, v1, 256, 64, 48)
				move.l	a1,a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				asr.w	#1,d4
				add.w	d0,d4
				bvc.s	.CombAddNoClamp_2_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.CombAddNoClamp_2_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#256<<1,d5
				blt.s	.NoCombReset_2_6
				moveq	#0,d5
.NoCombReset_2_6
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d2
				muls	#48,d2
				asr.l	#7,d2

				; v2 = add(v1, 23269)
				move.w	d0,d1
				add.w	#23269,d1
				bvc.s	.AddNoClamp_2_7
				spl		d1
				ext.w	d1
				eor.w	#$7fff,d1
.AddNoClamp_2_7

				; v1 = add(v1, v3)
				add.w	d2,d0
				bvc.s	.AddNoClamp_2_8
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_2_8

				; v2 = mul(v2, v2)
				muls	d1,d1
				add.l	d1,d1
				swap	d1

				; v1 = mul(v1, v4)
				muls	d3,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+4(a5),d7
				blt		.Inst2Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 2 - Loop Generator (Offset: 4724 Length: 3468
;----------------------------------------------------------------------------

				move.l	#3468,d7
				move.l	AK_SmpAddr+4(a5),a0
				lea		4724(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_1
				moveq	#0,d0
.LoopGenVC_1
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_1
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_1

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 3 - colombia_kick+snare
;----------------------------------------------------------------------------

				moveq	#1,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst3Loop
				; v1 = clone(smp,0, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+0(a5),d7
				bge.s	.NoClone_3_1
				move.l	AK_SmpAddr+0(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_3_1

				; v2 = clone(smp,1, 0)
				moveq	#0,d1
				cmp.l	AK_SmpLen+4(a5),d7
				bge.s	.NoClone_3_2
				move.l	AK_SmpAddr+4(a5),a4
				move.b	(a4,d7.l),d1
				asl.w	#8,d1
.NoClone_3_2

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_3_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_3_3

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+8(a5),d7
				blt		.Inst3Loop

;----------------------------------------------------------------------------
; Instrument 4 - colombia_openhat
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst4Loop
				; v1 = clone(smp,1, 3072)
				move.l	d7,d6
				add.l	#3072,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+4(a5),d6
				bge.s	.NoClone_4_1
				move.l	AK_SmpAddr+4(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_4_1

				; v1 = sv_flt_n(1, v1, 120, 127, 1)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#120,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_4_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_4_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_4_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_4_2
.NoClampMaxHPF_4_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_4_2
				move.w	#-32768,d5
.NoClampMinHPF_4_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#120,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_4_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_4_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_HPF+0(a5),d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+12(a5),d7
				blt		.Inst4Loop

;----------------------------------------------------------------------------
; Instrument 5 - colombia_closedhat
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst5Loop
				; v2 = envd(0, 4, 24, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#2796032,d5
				cmp.l	#402653184,d5
				bgt.s   .EnvDNoSustain_5_1
				move.l	#402653184,d5
.EnvDNoSustain_5_1
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = clone(smp,3, 1044)
				move.l	d7,d6
				add.l	#1044,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+12(a5),d6
				bge.s	.NoClone_5_2
				move.l	AK_SmpAddr+12(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_5_2

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+16(a5),d7
				blt		.Inst5Loop

;----------------------------------------------------------------------------
; Instrument 6 - colombia_kick+hat
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst6Loop
				; v1 = clone(smp,0, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+0(a5),d7
				bge.s	.NoClone_6_1
				move.l	AK_SmpAddr+0(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_6_1

				; v2 = clone(smp,4, 0)
				moveq	#0,d1
				cmp.l	AK_SmpLen+16(a5),d7
				bge.s	.NoClone_6_2
				move.l	AK_SmpAddr+16(a5),a4
				move.b	(a4,d7.l),d1
				asl.w	#8,d1
.NoClone_6_2

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_6_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_6_3

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+20(a5),d7
				blt		.Inst6Loop

;----------------------------------------------------------------------------
; Instrument 7 - colombia_ghostsnare
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst7Loop
				; v2 = envd(0, 4, 24, 100)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#2796032,d5
				cmp.l	#402653184,d5
				bgt.s   .EnvDNoSustain_7_1
				move.l	#402653184,d5
.EnvDNoSustain_7_1
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#100,d1
				asr.l	#7,d1

				; v1 = clone(smp,1, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+4(a5),d7
				bge.s	.NoClone_7_2
				move.l	AK_SmpAddr+4(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_7_2

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+24(a5),d7
				blt		.Inst7Loop

;----------------------------------------------------------------------------
; Instrument 8 - colombia_reversekick
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst8Loop
				; v1 = clone_reverse(smp,0, 0)
				move.l	d7,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+0(a5),d6
				bge.s	.NoClone_8_1
				move.l	AK_SmpAddr+0+4(a5),a4
				neg.l	d6
				move.b	-1(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_8_1

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+28(a5),d7
				blt		.Inst8Loop

;----------------------------------------------------------------------------
; Instrument 9 - colombia_reversesnare
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst9Loop
				; v1 = clone_reverse(smp,1, 4096)
				move.l	d7,d6
				add.l	#4096,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+4(a5),d6
				bge.s	.NoClone_9_1
				move.l	AK_SmpAddr+4+4(a5),a4
				neg.l	d6
				move.b	-1(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_9_1

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+32(a5),d7
				blt		.Inst9Loop

;----------------------------------------------------------------------------
; Instrument 10 - colombia_superbass
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst10Loop
				; v2 = osc_sine(0, 123, 120)
				add.w	#123,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_10_1
				neg.w	d5
.SineNoAbs_10_1
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				muls	#120,d1
				asr.l	#7,d1

				; v3 = osc_tri(1, 252, 120)
				add.w	#252,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d2
				bge.s	.TriNoInvert_10_2
				not.w	d2
.TriNoInvert_10_2
				sub.w	#16384,d2
				add.w	d2,d2
				muls	#120,d2
				asr.l	#7,d2

				; v2 = ctrl(v2)
				moveq	#9,d4
				asr.w	d4,d1
				add.w	#64,d1

				; v1 = osc_pulse(3, 500, v2, 65)
				add.w	#500,AK_OpInstance+4(a5)
				move.w	d1,d5
				and.w	#255,d5
				cmp.w	#((65-63)<<9),AK_OpInstance+4(a5)
				slt		d0
				ext.w	d0
				eor.w	#$7fff,d0
				muls	d5,d0
				asr.l	#7,d0

				; v4 = mul(v2, -32768)
				move.w	d1,d3
				muls	#-32768,d3
				add.l	d3,d3
				swap	d3

				; v4 = add(v4, 127)
				add.w	#127,d3
				bvc.s	.AddNoClamp_10_6
				spl		d3
				ext.w	d3
				eor.w	#$7fff,d3
.AddNoClamp_10_6

				; v1 = sv_flt_n(6, v1, v4, 127, 0)
				move.w	AK_OpInstance+AK_BPF+6(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d3,d5
				move.w	AK_OpInstance+AK_LPF+6(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_10_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_10_7
				move.w	d4,AK_OpInstance+AK_LPF+6(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_10_7
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_10_7
.NoClampMaxHPF_10_7
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_10_7
				move.w	#-32768,d5
.NoClampMinHPF_10_7
				move.w	d5,AK_OpInstance+AK_HPF+6(a5)
				asr.w	#7,d5
				muls	d3,d5
				add.w	AK_OpInstance+AK_BPF+6(a5),d5
				bvc.s	.NoClampBPF_10_7
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_10_7
				move.w	d5,AK_OpInstance+AK_BPF+6(a5)
				move.w	AK_OpInstance+AK_LPF+6(a5),d0

				; v4 = envd(7, 32, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d3
				swap	d3
				sub.l	#65024,d5
				bgt.s   .EnvDNoSustain_10_8
				moveq	#0,d5
.EnvDNoSustain_10_8
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v4)
				muls	d3,d0
				add.l	d0,d0
				swap	d0

				; v2 = envd(9, 10, 12, 128)
				move.l	AK_EnvDValue+4(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#645120,d5
				cmp.l	#201326592,d5
				bgt.s   .EnvDNoSustain_10_10
				move.l	#201326592,d5
.EnvDNoSustain_10_10
				move.l	d5,AK_EnvDValue+4(a5)

				; v2 = mul(v2, 128)
				muls	#128,d1
				add.l	d1,d1
				swap	d1

				; v1 = sv_flt_n(11, v1, v2, 32, 0)
				move.w	AK_OpInstance+AK_BPF+12(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d1,d5
				move.w	AK_OpInstance+AK_LPF+12(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_10_12
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_10_12
				move.w	d4,AK_OpInstance+AK_LPF+12(a5)
				asl.w	#5,d6
				ext.l	d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_10_12
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_10_12
.NoClampMaxHPF_10_12
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_10_12
				move.w	#-32768,d5
.NoClampMinHPF_10_12
				move.w	d5,AK_OpInstance+AK_HPF+12(a5)
				asr.w	#7,d5
				muls	d1,d5
				add.w	AK_OpInstance+AK_BPF+12(a5),d5
				bvc.s	.NoClampBPF_10_12
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_10_12
				move.w	d5,AK_OpInstance+AK_BPF+12(a5)
				move.w	AK_OpInstance+AK_LPF+12(a5),d0

				; v1 = add(v1, v3)
				add.w	d2,d0
				bvc.s	.AddNoClamp_10_13
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_10_13

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+36(a5),d7
				blt		.Inst10Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 10 - Loop Generator (Offset: 8192 Length: 8192
;----------------------------------------------------------------------------

				move.l	#8192,d7
				move.l	AK_SmpAddr+36(a5),a0
				lea		8192(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_9
				moveq	#0,d0
.LoopGenVC_9
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_9
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_9

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 11 - colombia_superbass_high
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst11Loop
				; v2 = osc_sine(0, 246, 120)
				add.w	#246,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_11_1
				neg.w	d5
.SineNoAbs_11_1
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				muls	#120,d1
				asr.l	#7,d1

				; v3 = osc_tri(1, 252, 120)
				add.w	#252,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d2
				bge.s	.TriNoInvert_11_2
				not.w	d2
.TriNoInvert_11_2
				sub.w	#16384,d2
				add.w	d2,d2
				muls	#120,d2
				asr.l	#7,d2

				; v2 = ctrl(v2)
				moveq	#9,d4
				asr.w	d4,d1
				add.w	#64,d1

				; v1 = osc_pulse(3, 1000, v2, 65)
				add.w	#1000,AK_OpInstance+4(a5)
				move.w	d1,d5
				and.w	#255,d5
				cmp.w	#((65-63)<<9),AK_OpInstance+4(a5)
				slt		d0
				ext.w	d0
				eor.w	#$7fff,d0
				muls	d5,d0
				asr.l	#7,d0

				; v4 = mul(v2, -32768)
				move.w	d1,d3
				muls	#-32768,d3
				add.l	d3,d3
				swap	d3

				; v4 = add(v4, 127)
				add.w	#127,d3
				bvc.s	.AddNoClamp_11_6
				spl		d3
				ext.w	d3
				eor.w	#$7fff,d3
.AddNoClamp_11_6

				; v1 = sv_flt_n(6, v1, v4, 127, 0)
				move.w	AK_OpInstance+AK_BPF+6(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d3,d5
				move.w	AK_OpInstance+AK_LPF+6(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_11_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_11_7
				move.w	d4,AK_OpInstance+AK_LPF+6(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_11_7
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_11_7
.NoClampMaxHPF_11_7
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_11_7
				move.w	#-32768,d5
.NoClampMinHPF_11_7
				move.w	d5,AK_OpInstance+AK_HPF+6(a5)
				asr.w	#7,d5
				muls	d3,d5
				add.w	AK_OpInstance+AK_BPF+6(a5),d5
				bvc.s	.NoClampBPF_11_7
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_11_7
				move.w	d5,AK_OpInstance+AK_BPF+6(a5)
				move.w	AK_OpInstance+AK_LPF+6(a5),d0

				; v4 = envd(7, 32, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d3
				swap	d3
				sub.l	#65024,d5
				bgt.s   .EnvDNoSustain_11_8
				moveq	#0,d5
.EnvDNoSustain_11_8
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v4)
				muls	d3,d0
				add.l	d0,d0
				swap	d0

				; v2 = envd(9, 10, 12, 128)
				move.l	AK_EnvDValue+4(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#645120,d5
				cmp.l	#201326592,d5
				bgt.s   .EnvDNoSustain_11_10
				move.l	#201326592,d5
.EnvDNoSustain_11_10
				move.l	d5,AK_EnvDValue+4(a5)

				; v2 = mul(v2, 128)
				muls	#128,d1
				add.l	d1,d1
				swap	d1

				; v1 = sv_flt_n(11, v1, v2, 32, 0)
				move.w	AK_OpInstance+AK_BPF+12(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d1,d5
				move.w	AK_OpInstance+AK_LPF+12(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_11_12
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_11_12
				move.w	d4,AK_OpInstance+AK_LPF+12(a5)
				asl.w	#5,d6
				ext.l	d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_11_12
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_11_12
.NoClampMaxHPF_11_12
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_11_12
				move.w	#-32768,d5
.NoClampMinHPF_11_12
				move.w	d5,AK_OpInstance+AK_HPF+12(a5)
				asr.w	#7,d5
				muls	d1,d5
				add.w	AK_OpInstance+AK_BPF+12(a5),d5
				bvc.s	.NoClampBPF_11_12
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_11_12
				move.w	d5,AK_OpInstance+AK_BPF+12(a5)
				move.w	AK_OpInstance+AK_LPF+12(a5),d0

				; v1 = add(v1, v3)
				add.w	d2,d0
				bvc.s	.AddNoClamp_11_13
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_11_13

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+40(a5),d7
				blt		.Inst11Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 11 - Loop Generator (Offset: 4096 Length: 4096
;----------------------------------------------------------------------------

				move.l	#4096,d7
				move.l	AK_SmpAddr+40(a5),a0
				lea		4096(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_10
				moveq	#0,d0
.LoopGenVC_10
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_10
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_10

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 12 - colombia_physical_flute
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst12Loop
				; v1 = osc_noise(66)
				move.l	AK_NoiseSeeds+0(a5),d4
				move.l	AK_NoiseSeeds+4(a5),d5
				eor.l	d5,d4
				move.l	d4,AK_NoiseSeeds+0(a5)
				add.l	d5,AK_NoiseSeeds+8(a5)
				add.l	d4,AK_NoiseSeeds+4(a5)
				move.w	AK_NoiseSeeds+10(a5),d0
				muls	#66,d0
				asr.l	#7,d0

				; v4 = add(v1, 0)
				move.w	d0,d3
				add.w	#0,d3
				bvc.s	.AddNoClamp_12_2
				spl		d3
				ext.w	d3
				eor.w	#$7fff,d3
.AddNoClamp_12_2

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_12_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_12_3

				; v2 = dly_cyc(3, v1, 76, 127)
				move.w	d0,d4
				muls	#127,d4
				asr.l	#7,d4
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#76<<1,d5
				blt.s	.NoDelayReset_12_4
				moveq	#0,d5
.NoDelayReset_12_4
				move.w  d5,AK_OpInstance+0(a5)
				move.w	(a4,d5.w),d1

				; v2 = mul(v2, -32768)
				muls	#-32768,d1
				add.l	d1,d1
				swap	d1

				; v3 = envd(5, 26, 0, 127)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d2
				swap	d2
				sub.l	#98560,d5
				bgt.s   .EnvDNoSustain_12_6
				moveq	#0,d5
.EnvDNoSustain_12_6
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#127,d2
				asr.l	#7,d2

				; v3 = ctrl(v3)
				moveq	#9,d4
				asr.w	d4,d2
				add.w	#64,d2

				; v3 = mul(v3, -32768)
				muls	#-32768,d2
				add.l	d2,d2
				swap	d2

				; v3 = add(v3, 128)
				add.w	#128,d2
				bvc.s	.AddNoClamp_12_9
				spl		d2
				ext.w	d2
				eor.w	#$7fff,d2
.AddNoClamp_12_9

				; v2 = sv_flt_n(9, v2, v3, 21, 0)
				move.w	AK_OpInstance+AK_BPF+2(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d2,d5
				move.w	AK_OpInstance+AK_LPF+2(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_12_10
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_12_10
				move.w	d4,AK_OpInstance+AK_LPF+2(a5)
				muls	#21,d6
				move.w	d1,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_12_10
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_12_10
.NoClampMaxHPF_12_10
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_12_10
				move.w	#-32768,d5
.NoClampMinHPF_12_10
				move.w	d5,AK_OpInstance+AK_HPF+2(a5)
				asr.w	#7,d5
				muls	d2,d5
				add.w	AK_OpInstance+AK_BPF+2(a5),d5
				bvc.s	.NoClampBPF_12_10
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_12_10
				move.w	d5,AK_OpInstance+AK_BPF+2(a5)
				move.w	AK_OpInstance+AK_LPF+2(a5),d1

				; v4 = mul(v4, -16384)
				muls	#-16384,d3
				add.l	d3,d3
				swap	d3

				; v1 = add(v1, v4)
				add.w	d3,d0
				bvc.s	.AddNoClamp_12_12
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_12_12

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+44(a5),d7
				blt		.Inst12Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 12 - Loop Generator (Offset: 27268 Length: 5500
;----------------------------------------------------------------------------

				move.l	#5500,d7
				move.l	AK_SmpAddr+44(a5),a0
				lea		27268(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_11
				moveq	#0,d0
.LoopGenVC_11
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_11
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_11

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 13 - colombia_pling1
;----------------------------------------------------------------------------

				moveq	#1,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst13Loop
				; v1 = clone(smp,9, 1024)
				move.l	d7,d6
				add.l	#1024,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+36(a5),d6
				bge.s	.NoClone_13_1
				move.l	AK_SmpAddr+36(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_13_1

				; v1 = sv_flt_n(1, v1, 48, 0, 1)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#48,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_13_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_13_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#0,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_13_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_13_2
.NoClampMaxHPF_13_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_13_2
				move.w	#-32768,d5
.NoClampMinHPF_13_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#48,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_13_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_13_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_HPF+0(a5),d0

				; v2 = envd(2, 8, 16, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#931840,d5
				cmp.l	#268435456,d5
				bgt.s   .EnvDNoSustain_13_3
				move.l	#268435456,d5
.EnvDNoSustain_13_3
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+48(a5),d7
				blt		.Inst13Loop

;----------------------------------------------------------------------------
; Instrument 14 - colombia_lead
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst14Loop
				; v1 = osc_tri(0, 1000, 92)
				add.w	#1000,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				bge.s	.TriNoInvert_14_1
				not.w	d0
.TriNoInvert_14_1
				sub.w	#16384,d0
				add.w	d0,d0
				muls	#92,d0
				asr.l	#7,d0

				; v2 = osc_tri(1, 1007, 92)
				add.w	#1007,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d1
				bge.s	.TriNoInvert_14_2
				not.w	d1
.TriNoInvert_14_2
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#92,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_14_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_14_3

				; v2 = osc_saw(3, 2000, 72)
				add.w	#2000,AK_OpInstance+4(a5)
				move.w	AK_OpInstance+4(a5),d1
				muls	#72,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_14_5
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_14_5

				; v2 = envd(5, 8, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#931840,d5
				bgt.s   .EnvDNoSustain_14_6
				moveq	#0,d5
.EnvDNoSustain_14_6
				move.l	d5,AK_EnvDValue+0(a5)

				; v2 = mul(v2, 64)
				muls	#64,d1
				add.l	d1,d1
				swap	d1

				; v2 = add(v2, 10)
				add.w	#10,d1
				bvc.s	.AddNoClamp_14_8
				spl		d1
				ext.w	d1
				eor.w	#$7fff,d1
.AddNoClamp_14_8

				; v1 = sv_flt_n(8, v1, v2, 127, 2)
				move.w	AK_OpInstance+AK_BPF+6(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d1,d5
				move.w	AK_OpInstance+AK_LPF+6(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_14_9
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_14_9
				move.w	d4,AK_OpInstance+AK_LPF+6(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_14_9
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_14_9
.NoClampMaxHPF_14_9
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_14_9
				move.w	#-32768,d5
.NoClampMinHPF_14_9
				move.w	d5,AK_OpInstance+AK_HPF+6(a5)
				asr.w	#7,d5
				muls	d1,d5
				add.w	AK_OpInstance+AK_BPF+6(a5),d5
				bvc.s	.NoClampBPF_14_9
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_14_9
				move.w	d5,AK_OpInstance+AK_BPF+6(a5)
				move.w	d5,d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+52(a5),d7
				blt		.Inst14Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 14 - Loop Generator (Offset: 4096 Length: 4096
;----------------------------------------------------------------------------

				move.l	#4096,d7
				move.l	AK_SmpAddr+52(a5),a0
				lea		4096(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_13
				moveq	#0,d0
.LoopGenVC_13
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_13
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_13

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 15 - colombia_chord1
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst15Loop
				; v2 = osc_sine(0, 4, 64)
				add.w	#4,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_15_1
				neg.w	d5
.SineNoAbs_15_1
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				asr.w	#1,d1

				; v2 = ctrl(v2)
				moveq	#9,d4
				asr.w	d4,d1
				add.w	#64,d1

				; v1 = chordgen(2, 13, 3, 7, 10, v2)
				move.l	AK_SmpAddr+52(a5),a4
				move.b	(a4,d7.l),d6
				ext.w	d6
				move.w	d1,d4
				and.w	#255,d4
				add.w	d4,a4
				moveq	#0,d4
				move.w	AK_OpInstance+AK_CHORD1+2(a5),d4
				add.l	#77824,AK_OpInstance+AK_CHORD1+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD2+2(a5),d4
				add.l	#98048,AK_OpInstance+AK_CHORD2+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD3+2(a5),d4
				add.l	#116736,AK_OpInstance+AK_CHORD3+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	#255,d5
				cmp.w	d5,d6
				blt.s	.NoClampMaxChord_15_3
				move.w	d5,d6
				bra.s	.NoClampMinChord_15_3
.NoClampMaxChord_15_3
				not.w	d5
				cmp.w	d5,d6
				bge.s	.NoClampMinChord_15_3
				move.w	d5,d6
.NoClampMinChord_15_3
				asl.w	#7,d6
				move.w	d6,d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+56(a5),d7
				blt		.Inst15Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 15 - Loop Generator (Offset: 3894 Length: 2250
;----------------------------------------------------------------------------

				move.l	#2250,d7
				move.l	AK_SmpAddr+56(a5),a0
				lea		3894(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_14
				moveq	#0,d0
.LoopGenVC_14
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_14
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_14

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 16 - colombia_chord2
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst16Loop
				; v2 = osc_sine(0, 4, 64)
				add.w	#4,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_16_1
				neg.w	d5
.SineNoAbs_16_1
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				asr.w	#1,d1

				; v2 = ctrl(v2)
				moveq	#9,d4
				asr.w	d4,d1
				add.w	#64,d1

				; v1 = chordgen(2, 13, 3, 7, 9, v2)
				move.l	AK_SmpAddr+52(a5),a4
				move.b	(a4,d7.l),d6
				ext.w	d6
				move.w	d1,d4
				and.w	#255,d4
				add.w	d4,a4
				moveq	#0,d4
				move.w	AK_OpInstance+AK_CHORD1+2(a5),d4
				add.l	#77824,AK_OpInstance+AK_CHORD1+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD2+2(a5),d4
				add.l	#98048,AK_OpInstance+AK_CHORD2+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD3+2(a5),d4
				add.l	#110080,AK_OpInstance+AK_CHORD3+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	#255,d5
				cmp.w	d5,d6
				blt.s	.NoClampMaxChord_16_3
				move.w	d5,d6
				bra.s	.NoClampMinChord_16_3
.NoClampMaxChord_16_3
				not.w	d5
				cmp.w	d5,d6
				bge.s	.NoClampMinChord_16_3
				move.w	d5,d6
.NoClampMinChord_16_3
				asl.w	#7,d6
				move.w	d6,d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+60(a5),d7
				blt		.Inst16Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 16 - Loop Generator (Offset: 3894 Length: 2250
;----------------------------------------------------------------------------

				move.l	#2250,d7
				move.l	AK_SmpAddr+60(a5),a0
				lea		3894(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_15
				moveq	#0,d0
.LoopGenVC_15
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_15
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_15

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 17 - colombia_lead_high
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst17Loop
				; v1 = osc_tri(0, 2000, 92)
				add.w	#2000,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				bge.s	.TriNoInvert_17_1
				not.w	d0
.TriNoInvert_17_1
				sub.w	#16384,d0
				add.w	d0,d0
				muls	#92,d0
				asr.l	#7,d0

				; v2 = osc_tri(1, 2014, 92)
				add.w	#2014,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d1
				bge.s	.TriNoInvert_17_2
				not.w	d1
.TriNoInvert_17_2
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#92,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_17_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_17_3

				; v2 = osc_saw(3, 4000, 72)
				add.w	#4000,AK_OpInstance+4(a5)
				move.w	AK_OpInstance+4(a5),d1
				muls	#72,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_17_5
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_17_5

				; v2 = envd(5, 8, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#931840,d5
				bgt.s   .EnvDNoSustain_17_6
				moveq	#0,d5
.EnvDNoSustain_17_6
				move.l	d5,AK_EnvDValue+0(a5)

				; v2 = mul(v2, 64)
				muls	#64,d1
				add.l	d1,d1
				swap	d1

				; v2 = add(v2, 10)
				add.w	#10,d1
				bvc.s	.AddNoClamp_17_8
				spl		d1
				ext.w	d1
				eor.w	#$7fff,d1
.AddNoClamp_17_8

				; v1 = sv_flt_n(8, v1, v2, 127, 2)
				move.w	AK_OpInstance+AK_BPF+6(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d1,d5
				move.w	AK_OpInstance+AK_LPF+6(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_17_9
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_17_9
				move.w	d4,AK_OpInstance+AK_LPF+6(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_17_9
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_17_9
.NoClampMaxHPF_17_9
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_17_9
				move.w	#-32768,d5
.NoClampMinHPF_17_9
				move.w	d5,AK_OpInstance+AK_HPF+6(a5)
				asr.w	#7,d5
				muls	d1,d5
				add.w	AK_OpInstance+AK_BPF+6(a5),d5
				bvc.s	.NoClampBPF_17_9
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_17_9
				move.w	d5,AK_OpInstance+AK_BPF+6(a5)
				move.w	d5,d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+64(a5),d7
				blt		.Inst17Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 17 - Loop Generator (Offset: 4096 Length: 4096
;----------------------------------------------------------------------------

				move.l	#4096,d7
				move.l	AK_SmpAddr+64(a5),a0
				lea		4096(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_16
				moveq	#0,d0
.LoopGenVC_16
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_16
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_16

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 18 - colombia_chordstab1
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst18Loop
				; v1 = clone(smp,14, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+56(a5),d7
				bge.s	.NoClone_18_1
				move.l	AK_SmpAddr+56(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_18_1

				; v1 = add(v1, v1)
				add.w	d0,d0
				bvc.s	.AddNoClamp_18_2
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_18_2

				; v2 = envd(2, 8, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#931840,d5
				bgt.s   .EnvDNoSustain_18_3
				moveq	#0,d5
.EnvDNoSustain_18_3
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				; v1 = reverb(v1, 100, 16)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_18_5_0
				moveq	#0,d5
.NoReverbReset_18_5_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_18_5_1
				moveq	#0,d5
.NoReverbReset_18_5_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_18_5_2
				moveq	#0,d5
.NoReverbReset_18_5_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_18_5_3
				moveq	#0,d5
.NoReverbReset_18_5_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_18_5_4
				moveq	#0,d5
.NoReverbReset_18_5_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_18_5_5
				moveq	#0,d5
.NoReverbReset_18_5_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_18_5_6
				moveq	#0,d5
.NoReverbReset_18_5_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_18_5_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_18_5_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_18_5_7
				moveq	#0,d5
.NoReverbReset_18_5_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_18_5
				move.w	#32767,d7
				bra.s	.NoReverbMin_18_5
.NoReverbMax_18_5
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_18_5
				move.w	#-32768,d7
.NoReverbMin_18_5
				move.w	d7,d0
				move.l	(sp)+,d7

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+68(a5),d7
				blt		.Inst18Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 18 - Loop Generator (Offset: 2048 Length: 2048
;----------------------------------------------------------------------------

				move.l	#2048,d7
				move.l	AK_SmpAddr+68(a5),a0
				lea		2048(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_17
				moveq	#0,d0
.LoopGenVC_17
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_17
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_17

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 19 - colombia_chordstab2
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst19Loop
				; v1 = clone(smp,15, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+60(a5),d7
				bge.s	.NoClone_19_1
				move.l	AK_SmpAddr+60(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_19_1

				; v1 = add(v1, v1)
				add.w	d0,d0
				bvc.s	.AddNoClamp_19_2
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_19_2

				; v2 = envd(2, 8, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#931840,d5
				bgt.s   .EnvDNoSustain_19_3
				moveq	#0,d5
.EnvDNoSustain_19_3
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				; v1 = reverb(v1, 100, 16)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_19_5_0
				moveq	#0,d5
.NoReverbReset_19_5_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_19_5_1
				moveq	#0,d5
.NoReverbReset_19_5_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_19_5_2
				moveq	#0,d5
.NoReverbReset_19_5_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_19_5_3
				moveq	#0,d5
.NoReverbReset_19_5_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_19_5_4
				moveq	#0,d5
.NoReverbReset_19_5_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_19_5_5
				moveq	#0,d5
.NoReverbReset_19_5_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_19_5_6
				moveq	#0,d5
.NoReverbReset_19_5_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#100,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_19_5_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_19_5_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_19_5_7
				moveq	#0,d5
.NoReverbReset_19_5_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_19_5
				move.w	#32767,d7
				bra.s	.NoReverbMin_19_5
.NoReverbMax_19_5
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_19_5
				move.w	#-32768,d7
.NoReverbMin_19_5
				move.w	d7,d0
				move.l	(sp)+,d7

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+72(a5),d7
				blt		.Inst19Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 19 - Loop Generator (Offset: 2048 Length: 2048
;----------------------------------------------------------------------------

				move.l	#2048,d7
				move.l	AK_SmpAddr+72(a5),a0
				lea		2048(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_18
				moveq	#0,d0
.LoopGenVC_18
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_18
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_18

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 20 - colombia_leadstab
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst20Loop
				; v1 = clone(smp,16, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+64(a5),d7
				bge.s	.NoClone_20_1
				move.l	AK_SmpAddr+64(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_20_1

				; v2 = envd(1, 6, 0, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#1677568,d5
				bgt.s   .EnvDNoSustain_20_2
				moveq	#0,d5
.EnvDNoSustain_20_2
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				; v1 = reverb(v1, 120, 16)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_20_4_0
				moveq	#0,d5
.NoReverbReset_20_4_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_20_4_1
				moveq	#0,d5
.NoReverbReset_20_4_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_20_4_2
				moveq	#0,d5
.NoReverbReset_20_4_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_20_4_3
				moveq	#0,d5
.NoReverbReset_20_4_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_20_4_4
				moveq	#0,d5
.NoReverbReset_20_4_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_20_4_5
				moveq	#0,d5
.NoReverbReset_20_4_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_20_4_6
				moveq	#0,d5
.NoReverbReset_20_4_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#120,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_20_4_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_20_4_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_20_4_7
				moveq	#0,d5
.NoReverbReset_20_4_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_20_4
				move.w	#32767,d7
				bra.s	.NoReverbMin_20_4
.NoReverbMax_20_4
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_20_4
				move.w	#-32768,d7
.NoReverbMin_20_4
				move.w	d7,d0
				move.l	(sp)+,d7

				; v2 = mul(v1, 16384)
				move.w	d0,d1
				muls	#16384,d1
				add.l	d1,d1
				swap	d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_20_6
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_20_6

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+76(a5),d7
				blt		.Inst20Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 20 - Loop Generator (Offset: 2184 Length: 1912
;----------------------------------------------------------------------------

				move.l	#1912,d7
				move.l	AK_SmpAddr+76(a5),a0
				lea		2184(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_19
				moveq	#0,d0
.LoopGenVC_19
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_19
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_19

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 21 - colombia_pling2
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst21Loop
				; v1 = clone(smp,9, 1024)
				move.l	d7,d6
				add.l	#1024,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+36(a5),d6
				bge.s	.NoClone_21_1
				move.l	AK_SmpAddr+36(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_21_1

				; v1 = sv_flt_n(1, v1, 32, 0, 2)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				asl.w	#5,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_21_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_21_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#0,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_21_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_21_2
.NoClampMaxHPF_21_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_21_2
				move.w	#-32768,d5
.NoClampMinHPF_21_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				asl.w	#5,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_21_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_21_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	d5,d0

				; v2 = envd(2, 7, 16, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#1198336,d5
				cmp.l	#268435456,d5
				bgt.s   .EnvDNoSustain_21_3
				move.l	#268435456,d5
.EnvDNoSustain_21_3
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+80(a5),d7
				blt		.Inst21Loop

;----------------------------------------------------------------------------
; Instrument 22 - colombia_pling3
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst22Loop
				; v1 = clone(smp,9, 1024)
				move.l	d7,d6
				add.l	#1024,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+36(a5),d6
				bge.s	.NoClone_22_1
				move.l	AK_SmpAddr+36(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_22_1

				; v1 = sv_flt_n(1, v1, 58, 0, 0)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#58,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_22_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_22_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#0,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_22_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_22_2
.NoClampMaxHPF_22_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_22_2
				move.w	#-32768,d5
.NoClampMinHPF_22_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#58,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_22_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_22_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_LPF+0(a5),d0

				; v2 = envd(2, 7, 16, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#1198336,d5
				cmp.l	#268435456,d5
				bgt.s   .EnvDNoSustain_22_3
				move.l	#268435456,d5
.EnvDNoSustain_22_3
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+84(a5),d7
				blt		.Inst22Loop

;----------------------------------------------------------------------------
; Instrument 23 - colombia_pling4
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst23Loop
				; v1 = clone(smp,9, 1024)
				move.l	d7,d6
				add.l	#1024,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+36(a5),d6
				bge.s	.NoClone_23_1
				move.l	AK_SmpAddr+36(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_23_1

				; v1 = sv_flt_n(1, v1, 10, 1, 0)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#10,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_23_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_23_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#1,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_23_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_23_2
.NoClampMaxHPF_23_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_23_2
				move.w	#-32768,d5
.NoClampMinHPF_23_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#10,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_23_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_23_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_LPF+0(a5),d0

				; v2 = envd(2, 7, 16, 128)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#1198336,d5
				cmp.l	#268435456,d5
				bgt.s   .EnvDNoSustain_23_3
				move.l	#268435456,d5
.EnvDNoSustain_23_3
				move.l	d5,AK_EnvDValue+0(a5)

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+88(a5),d7
				blt		.Inst23Loop

;----------------------------------------------------------------------------
; Instrument 24 - colombia_hat_low
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst24Loop
				; v2 = envd(0, 4, 8, 80)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#2796032,d5
				cmp.l	#134217728,d5
				bgt.s   .EnvDNoSustain_24_1
				move.l	#134217728,d5
.EnvDNoSustain_24_1
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#80,d1
				asr.l	#7,d1

				; v1 = clone(smp,3, 32)
				move.l	d7,d6
				add.l	#32,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+12(a5),d6
				bge.s	.NoClone_24_2
				move.l	AK_SmpAddr+12(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_24_2

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+92(a5),d7
				blt		.Inst24Loop

;----------------------------------------------------------------------------
; Instrument 25 - colombia_missing_chord
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst25Loop
				; v1 = osc_saw(0, 2000, 16)
				add.w	#2000,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				asr.w	#3,d0

				; v1 = clone_reverse(smp,13, 0)
				move.l	d7,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+52(a5),d6
				bge.s	.NoClone_25_2
				move.l	AK_SmpAddr+52+4(a5),a4
				neg.l	d6
				move.b	-1(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_25_2

				asr.w	#8,d0
				move.b	d0,(a0)+
				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif
				addq.l	#1,d7
				cmp.l	AK_SmpLen+96(a5),d7
				blt		.Inst25Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 25 - Loop Generator (Offset: 5119 Length: 5121
;----------------------------------------------------------------------------

				move.l	#5121,d7
				move.l	AK_SmpAddr+96(a5),a0
				lea		5119(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_24
				moveq	#0,d0
.LoopGenVC_24
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_24
				move.l	d4,d2
				asr.l	#8,d2
				move.l	d5,d3
				asr.l	#8,d3
				move.b	(a0),d0
				move.b	(a1)+,d1
				ext.w	d0
				ext.w	d1
				muls	d3,d0
				muls	d2,d1
				add.l	d1,d0
				add.l	d0,d0
				swap	d0
				move.b	d0,(a0)+
				add.l	d6,d4
				sub.l	d6,d5

				ifne	AK_USE_PROGRESS
					ifne	AK_FINE_PROGRESS
						addq.l	#1,(a3)
					endif
				endif

				subq.l	#1,d7
				bne.s	.LoopGen_24

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator


;----------------------------------------------------------------------------

				; Clear first 2 bytes of each sample
				lea		AK_SmpAddr(a5),a6
				moveq	#0,d0
				moveq	#31-1,d7
.SmpClrLoop		move.l	(a6)+,a4
				move.b	d0,(a4)+
				move.b	d0,(a4)+
				dbra	d7,.SmpClrLoop

				rts

;----------------------------------------------------------------------------

AK_ResetVars:
				moveq   #0,d1
				moveq   #0,d2
				moveq   #0,d3
				move.w  d0,d7
				beq.s	.NoClearDelay
				lsl.w	#8,d7
				subq.w	#1,d7
				move.l  a1,a6
.ClearDelayLoop
				move.l  d1,(a6)+
				move.l  d1,(a6)+
				move.l  d1,(a6)+
				move.l  d1,(a6)+
				dbra	d7,.ClearDelayLoop
.NoClearDelay
				moveq   #0,d0
				lea		AK_OpInstance(a5),a6
				move.l	d0,(a6)+
				move.l	d0,(a6)+
				move.l	d0,(a6)+
				move.l	d0,(a6)+
				move.l	d0,(a6)+
				move.l  #32767<<16,d6
				move.l	d6,(a6)+
				move.l	d6,(a6)+
				rts

;----------------------------------------------------------------------------

				rsreset
AK_LPF			rs.w	1
AK_HPF			rs.w	1
AK_BPF			rs.w	1
				rsreset
AK_CHORD1		rs.l	1
AK_CHORD2		rs.l	1
AK_CHORD3		rs.l	1
				rsreset
AK_SmpLen		rs.l	31
AK_ExtSmpLen	rs.l	8
AK_NoiseSeeds	rs.l	3
AK_SmpAddr		rs.l	31
AK_ExtSmpAddr	rs.l	8
AK_OpInstance	rs.w    10
AK_EnvDValue	rs.l	2
AK_VarSize		rs.w	0

AK_Vars:
				dc.l	$00000880		; Instrument 1 Length 
				dc.l	$00002000		; Instrument 2 Length 
				dc.l	$00002000		; Instrument 3 Length 
				dc.l	$00001000		; Instrument 4 Length 
				dc.l	$00000900		; Instrument 5 Length 
				dc.l	$00000900		; Instrument 6 Length 
				dc.l	$00000b00		; Instrument 7 Length 
				dc.l	$00000880		; Instrument 8 Length 
				dc.l	$00000900		; Instrument 9 Length 
				dc.l	$00004000		; Instrument 10 Length 
				dc.l	$00002000		; Instrument 11 Length 
				dc.l	$00008000		; Instrument 12 Length 
				dc.l	$00000b00		; Instrument 13 Length 
				dc.l	$00002000		; Instrument 14 Length 
				dc.l	$00001800		; Instrument 15 Length 
				dc.l	$00001800		; Instrument 16 Length 
				dc.l	$00002000		; Instrument 17 Length 
				dc.l	$00001000		; Instrument 18 Length 
				dc.l	$00001000		; Instrument 19 Length 
				dc.l	$00001000		; Instrument 20 Length 
				dc.l	$00000b00		; Instrument 21 Length 
				dc.l	$00000b00		; Instrument 22 Length 
				dc.l	$00000b00		; Instrument 23 Length 
				dc.l	$000002a0		; Instrument 24 Length 
				dc.l	$00002800		; Instrument 25 Length 
				dc.l	$00000000		; Instrument 26 Length 
				dc.l	$00000000		; Instrument 27 Length 
				dc.l	$00000000		; Instrument 28 Length 
				dc.l	$00000000		; Instrument 29 Length 
				dc.l	$00000000		; Instrument 30 Length 
				dc.l	$00000000		; Instrument 31 Length 
				dc.l	$00000000		; External Sample 1 Length 
				dc.l	$00000000		; External Sample 2 Length 
				dc.l	$00000000		; External Sample 3 Length 
				dc.l	$00000000		; External Sample 4 Length 
				dc.l	$00000000		; External Sample 5 Length 
				dc.l	$00000000		; External Sample 6 Length 
				dc.l	$00000000		; External Sample 7 Length 
				dc.l	$00000000		; External Sample 8 Length 
				dc.l	$67452301		; AK_NoiseSeed1
				dc.l	$efcdab89		; AK_NoiseSeed2
				dc.l	$00000000		; AK_NoiseSeed3
				ds.b	AK_VarSize-AK_SmpAddr

;----------------------------------------------------------------------------
