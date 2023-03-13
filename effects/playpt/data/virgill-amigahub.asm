        section '.bsschip', bss
        xdef _Samples

_Samples:
        ds.w AK_SMP_LEN/2

        section '.data', data
        xdef _ExtSamples

_ExtSamples:
        incbin  'data/virgill-amigahub.raw'

        section '.text', code
        xdef _AK_Generate
        xdef _AK_Progress

_AK_Generate:
        movem.l d2-d7/a2-a6,-(sp)
        lea     _Samples,a0
        lea     _ExtSamples,a2
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
AK_FINE_PROGRESS_LEN	equ 333632
AK_SMP_LEN				equ 242678
AK_EXT_SMP_LEN			equ 30896

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

				; Convert external samples from stored deltas
				move.l	a2,a6
				move.w	#AK_EXT_SMP_LEN-1,d7
				moveq	#0,d0
.DeltaLoop		add.b	(a6),d0
				move.b	d0,(a6)+
				dbra	d7,.DeltaLoop

;----------------------------------------------------------------------------
; Instrument 1 - amigahub-hat
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
				; v1 = imported_sample(smp,0)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+0(a5),d7
				bge.s	.NoClone_1_1
				move.l	AK_ExtSmpAddr+0(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_1_1

				; v1 = sv_flt_n(1, v1, 79, 127, 1)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#79,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_1_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_1_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_1_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_1_2
.NoClampMaxHPF_1_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_1_2
				move.w	#-32768,d5
.NoClampMinHPF_1_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#79,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_1_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_1_2
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
				cmp.l	AK_SmpLen+0(a5),d7
				blt		.Inst1Loop

;----------------------------------------------------------------------------
; Instrument 2 - amigahub-hat2
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
				; v1 = clone(smp,0, 336)
				move.l	d7,d6
				add.l	#336,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+0(a5),d6
				bge.s	.NoClone_2_1
				move.l	AK_SmpAddr+0(a5),a4
				move.b	(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_2_1

				; v2 = enva(1, 4, 0, 127)
				move.l	AK_OpInstance+0(a5),d5
				move.l	d5,d1
				swap	d1
				add.l	#2796032,d5
				bvc.s   .EnvANoMax_2_2
				move.l	#32767<<16,d5
.EnvANoMax_2_2
				move.l	d5,AK_OpInstance+0(a5)
				muls	#127,d1
				asr.l	#7,d1

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
				cmp.l	AK_SmpLen+4(a5),d7
				blt		.Inst2Loop

;----------------------------------------------------------------------------
; Instrument 3 - amigahub-hatopen
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst3Loop
				; v1 = imported_sample(smp,1)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+4(a5),d7
				bge.s	.NoClone_3_1
				move.l	AK_ExtSmpAddr+4(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_3_1

				; v1 = sv_flt_n(1, v1, 40, 127, 1)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#40,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_3_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_3_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_3_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_3_2
.NoClampMaxHPF_3_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_3_2
				move.w	#-32768,d5
.NoClampMinHPF_3_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#40,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_3_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_3_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_HPF+0(a5),d0

				; v1 = reverb(v1, 107, 13)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_3_3_0
				moveq	#0,d5
.NoReverbReset_3_3_0
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_3_3_1
				moveq	#0,d5
.NoReverbReset_3_3_1
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_3_3_2
				moveq	#0,d5
.NoReverbReset_3_3_2
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_3_3_3
				moveq	#0,d5
.NoReverbReset_3_3_3
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_3_3_4
				moveq	#0,d5
.NoReverbReset_3_3_4
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+16(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_3_3_5
				moveq	#0,d5
.NoReverbReset_3_3_5
				move.w  d5,AK_OpInstance+16(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+18(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_3_3_6
				moveq	#0,d5
.NoReverbReset_3_3_6
				move.w  d5,AK_OpInstance+18(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+20(a5),d5
				move.w	(a4,d5.w),d4
				muls	#107,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_3_3_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_3_3_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_3_3_7
				moveq	#0,d5
.NoReverbReset_3_3_7
				move.w  d5,AK_OpInstance+20(a5)
				move.w	d4,d7
				muls	#13,d7
				asr.l	#7,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_3_3
				move.w	#32767,d7
				bra.s	.NoReverbMin_3_3
.NoReverbMax_3_3
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_3_3
				move.w	#-32768,d7
.NoReverbMin_3_3
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
				cmp.l	AK_SmpLen+8(a5),d7
				blt		.Inst3Loop

;----------------------------------------------------------------------------
; Instrument 4 - amigahub-kick
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst4Loop
				; v1 = imported_sample(smp,2)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+8(a5),d7
				bge.s	.NoClone_4_1
				move.l	AK_ExtSmpAddr+8(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_4_1

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
; Instrument 5 - amigahub_hat+bell
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
				; v1 = clone(smp,0, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+0(a5),d7
				bge.s	.NoClone_5_1
				move.l	AK_SmpAddr+0(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_5_1

				; v1 = cmb_flt_n(1, v1, 73, 96, 99)
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#96,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.CombAddNoClamp_5_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.CombAddNoClamp_5_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#73<<1,d5
				blt.s	.NoCombReset_5_2
				moveq	#0,d5
.NoCombReset_5_2
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d0
				muls	#99,d0
				asr.l	#7,d0

				; v1 = sv_flt_n(2, v1, 17, 0, 1)
				move.w	AK_OpInstance+AK_BPF+2(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#17,d5
				move.w	AK_OpInstance+AK_LPF+2(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_5_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_5_3
				move.w	d4,AK_OpInstance+AK_LPF+2(a5)
				muls	#0,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_5_3
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_5_3
.NoClampMaxHPF_5_3
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_5_3
				move.w	#-32768,d5
.NoClampMinHPF_5_3
				move.w	d5,AK_OpInstance+AK_HPF+2(a5)
				asr.w	#7,d5
				muls	#17,d5
				add.w	AK_OpInstance+AK_BPF+2(a5),d5
				bvc.s	.NoClampBPF_5_3
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_5_3
				move.w	d5,AK_OpInstance+AK_BPF+2(a5)
				move.w	AK_OpInstance+AK_HPF+2(a5),d0

				; v1 = sv_flt_n(3, v1, 17, 0, 1)
				move.w	AK_OpInstance+AK_BPF+8(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#17,d5
				move.w	AK_OpInstance+AK_LPF+8(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_5_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_5_4
				move.w	d4,AK_OpInstance+AK_LPF+8(a5)
				muls	#0,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_5_4
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_5_4
.NoClampMaxHPF_5_4
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_5_4
				move.w	#-32768,d5
.NoClampMinHPF_5_4
				move.w	d5,AK_OpInstance+AK_HPF+8(a5)
				asr.w	#7,d5
				muls	#17,d5
				add.w	AK_OpInstance+AK_BPF+8(a5),d5
				bvc.s	.NoClampBPF_5_4
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_5_4
				move.w	d5,AK_OpInstance+AK_BPF+8(a5)
				move.w	AK_OpInstance+AK_HPF+8(a5),d0

				; v2 = envd(4, 6, 0, 50)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#1677568,d5
				bgt.s   .EnvDNoSustain_5_5
				moveq	#0,d5
.EnvDNoSustain_5_5
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#50,d1
				asr.l	#7,d1

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
; Instrument 6 - amigahub-snare1
;----------------------------------------------------------------------------

				moveq	#1,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst6Loop
				; v1 = imported_sample(smp,3)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+12(a5),d7
				bge.s	.NoClone_6_1
				move.l	AK_ExtSmpAddr+12(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_6_1

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
; Instrument 7 - amigahub-snare2
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
				; v1 = imported_sample(smp,4)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+16(a5),d7
				bge.s	.NoClone_7_1
				move.l	AK_ExtSmpAddr+16(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_7_1

				; v1 = reverb(v1, 82, 16)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_7_2_0
				moveq	#0,d5
.NoReverbReset_7_2_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_7_2_1
				moveq	#0,d5
.NoReverbReset_7_2_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_7_2_2
				moveq	#0,d5
.NoReverbReset_7_2_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_7_2_3
				moveq	#0,d5
.NoReverbReset_7_2_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_7_2_4
				moveq	#0,d5
.NoReverbReset_7_2_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_7_2_5
				moveq	#0,d5
.NoReverbReset_7_2_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_7_2_6
				moveq	#0,d5
.NoReverbReset_7_2_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#82,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_7_2_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_7_2_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_7_2_7
				moveq	#0,d5
.NoReverbReset_7_2_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				asr.w	#3,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_7_2
				move.w	#32767,d7
				bra.s	.NoReverbMin_7_2
.NoReverbMax_7_2
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_7_2
				move.w	#-32768,d7
.NoReverbMin_7_2
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
				cmp.l	AK_SmpLen+24(a5),d7
				blt		.Inst7Loop

;----------------------------------------------------------------------------
; Instrument 8 - amigahub_snare3
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst8Loop
				; v1 = imported_sample(smp,4)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+16(a5),d7
				bge.s	.NoClone_8_1
				move.l	AK_ExtSmpAddr+16(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_8_1

				; v1 = sv_flt_n(1, v1, 37, 55, 2)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#37,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_8_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_8_2
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				muls	#55,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_8_2
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_8_2
.NoClampMaxHPF_8_2
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_8_2
				move.w	#-32768,d5
.NoClampMinHPF_8_2
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	#37,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_8_2
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_8_2
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	d5,d0

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
; Instrument 9 - amigahub_snare_attack
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
				; v1 = imported_sample(smp,3)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+12(a5),d7
				bge.s	.NoClone_9_1
				move.l	AK_ExtSmpAddr+12(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_9_1

				; v1 = vol(v1, 70)
				muls	#70,d0
				asr.l	#7,d0

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
; Instrument 10 - amigahub_hat+bell+reverb
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
				; v1 = clone(smp,4, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+16(a5),d7
				bge.s	.NoClone_10_1
				move.l	AK_SmpAddr+16(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_10_1

				; v1 = reverb(v1, 99, 24)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_10_2_0
				moveq	#0,d5
.NoReverbReset_10_2_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_10_2_1
				moveq	#0,d5
.NoReverbReset_10_2_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_10_2_2
				moveq	#0,d5
.NoReverbReset_10_2_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_10_2_3
				moveq	#0,d5
.NoReverbReset_10_2_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_10_2_4
				moveq	#0,d5
.NoReverbReset_10_2_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_10_2_5
				moveq	#0,d5
.NoReverbReset_10_2_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_10_2_6
				moveq	#0,d5
.NoReverbReset_10_2_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#99,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_10_2_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_10_2_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_10_2_7
				moveq	#0,d5
.NoReverbReset_10_2_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				muls	#24,d7
				asr.l	#7,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_10_2
				move.w	#32767,d7
				bra.s	.NoReverbMin_10_2
.NoReverbMax_10_2
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_10_2
				move.w	#-32768,d7
.NoReverbMin_10_2
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
				cmp.l	AK_SmpLen+36(a5),d7
				blt		.Inst10Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 10 - Loop Generator (Offset: 3952 Length: 3952
;----------------------------------------------------------------------------

				move.l	#3952,d7
				move.l	AK_SmpAddr+36(a5),a0
				lea		3952(a0),a0
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
; Instrument 11 - amigahub_organ_lead
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst11Loop
				; v1 = imported_sample(smp,6)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+24(a5),d7
				bge.s	.NoClone_11_1
				move.l	AK_ExtSmpAddr+24(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_11_1

				; v2 = envd(1, 11, 0, 127)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#524288,d5
				bgt.s   .EnvDNoSustain_11_2
				moveq	#0,d5
.EnvDNoSustain_11_2
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#127,d1
				asr.l	#7,d1

				; v1 = mul(v1, v2)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				; v1 = reverb(v1, 126, 60)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_11_4_0
				moveq	#0,d5
.NoReverbReset_11_4_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_11_4_1
				moveq	#0,d5
.NoReverbReset_11_4_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_11_4_2
				moveq	#0,d5
.NoReverbReset_11_4_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_11_4_3
				moveq	#0,d5
.NoReverbReset_11_4_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_11_4_4
				moveq	#0,d5
.NoReverbReset_11_4_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_11_4_5
				moveq	#0,d5
.NoReverbReset_11_4_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_11_4_6
				moveq	#0,d5
.NoReverbReset_11_4_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#126,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_11_4_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_11_4_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_11_4_7
				moveq	#0,d5
.NoReverbReset_11_4_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				muls	#60,d7
				asr.l	#7,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_11_4
				move.w	#32767,d7
				bra.s	.NoReverbMin_11_4
.NoReverbMax_11_4
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_11_4
				move.w	#-32768,d7
.NoReverbMin_11_4
				move.w	d7,d0
				move.l	(sp)+,d7

				; v2 = osc_sine(4, 900, 24)
				add.w	#900,AK_OpInstance+16(a5)
				move.w	AK_OpInstance+16(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_11_5
				neg.w	d5
.SineNoAbs_11_5
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				muls	#24,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_11_6
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_11_6

				; v2 = osc_sine(6, 1788, 16)
				add.w	#1788,AK_OpInstance+18(a5)
				move.w	AK_OpInstance+18(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_11_7
				neg.w	d5
.SineNoAbs_11_7
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				asr.w	#3,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_11_8
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_11_8

				; v1 = vol(v1, 110)
				muls	#110,d0
				asr.l	#7,d0

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
; Instrument 11 - Loop Generator (Offset: 14848 Length: 14848
;----------------------------------------------------------------------------

				move.l	#14848,d7
				move.l	AK_SmpAddr+40(a5),a0
				lea		14848(a0),a0
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
; Instrument 12 - amigahub_chord1
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst12Loop
				; v1 = osc_sine(0, 4, 54)
				add.w	#4,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				sub.w	#16384,d0
				move.w	d0,d5
				bge.s	.SineNoAbs_12_1
				neg.w	d5
.SineNoAbs_12_1
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d0
				swap	d0
				asl.w	#3,d0
				muls	#54,d0
				asr.l	#7,d0

				; v1 = ctrl(v1)
				moveq	#9,d4
				asr.w	d4,d0
				add.w	#64,d0

				; v1 = chordgen(2, 10, 3, 5, 10, v1)
				move.l	AK_SmpAddr+40(a5),a4
				move.b	(a4,d7.l),d6
				ext.w	d6
				move.w	d0,d4
				and.w	#255,d4
				add.w	d4,a4
				moveq	#0,d4
				move.w	AK_OpInstance+AK_CHORD1+2(a5),d4
				add.l	#77824,AK_OpInstance+AK_CHORD1+2(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD2+2(a5),d4
				add.l	#87552,AK_OpInstance+AK_CHORD2+2(a5)
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
				blt.s	.NoClampMaxChord_12_3
				move.w	d5,d6
				bra.s	.NoClampMinChord_12_3
.NoClampMaxChord_12_3
				not.w	d5
				cmp.w	d5,d6
				bge.s	.NoClampMinChord_12_3
				move.w	d5,d6
.NoClampMinChord_12_3
				asl.w	#7,d6
				move.w	d6,d0

				; v2 = osc_tri(3, 450, 52)
				add.w	#450,AK_OpInstance+14(a5)
				move.w	AK_OpInstance+14(a5),d1
				bge.s	.TriNoInvert_12_4
				not.w	d1
.TriNoInvert_12_4
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#52,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_12_5
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_12_5

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
; Instrument 12 - Loop Generator (Offset: 8192 Length: 8192
;----------------------------------------------------------------------------

				move.l	#8192,d7
				move.l	AK_SmpAddr+44(a5),a0
				lea		8192(a0),a0
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
; Instrument 13 - amigahub_chord2
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst13Loop
				; v1 = chordgen(0, 10, 3, 7, 10, 0)
				move.l	AK_SmpAddr+40(a5),a4
				move.b	(a4,d7.l),d6
				ext.w	d6
				moveq	#0,d4
				move.w	AK_OpInstance+AK_CHORD1+0(a5),d4
				add.l	#77824,AK_OpInstance+AK_CHORD1+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD2+0(a5),d4
				add.l	#98048,AK_OpInstance+AK_CHORD2+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD3+0(a5),d4
				add.l	#116736,AK_OpInstance+AK_CHORD3+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	#255,d5
				cmp.w	d5,d6
				blt.s	.NoClampMaxChord_13_1
				move.w	d5,d6
				bra.s	.NoClampMinChord_13_1
.NoClampMaxChord_13_1
				not.w	d5
				cmp.w	d5,d6
				bge.s	.NoClampMinChord_13_1
				move.w	d5,d6
.NoClampMinChord_13_1
				asl.w	#7,d6
				move.w	d6,d0

				; v2 = osc_tri(1, 337, 52)
				add.w	#337,AK_OpInstance+12(a5)
				move.w	AK_OpInstance+12(a5),d1
				bge.s	.TriNoInvert_13_2
				not.w	d1
.TriNoInvert_13_2
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#52,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_13_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_13_3

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

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 13 - Loop Generator (Offset: 7560 Length: 6264
;----------------------------------------------------------------------------

				move.l	#6264,d7
				move.l	AK_SmpAddr+48(a5),a0
				lea		7560(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_12
				moveq	#0,d0
.LoopGenVC_12
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_12
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
				bne.s	.LoopGen_12

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 14 - amigahub_chord3
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
				; v1 = chordgen(0, 10, 3, 7, 8, 0)
				move.l	AK_SmpAddr+40(a5),a4
				move.b	(a4,d7.l),d6
				ext.w	d6
				moveq	#0,d4
				move.w	AK_OpInstance+AK_CHORD1+0(a5),d4
				add.l	#77824,AK_OpInstance+AK_CHORD1+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD2+0(a5),d4
				add.l	#98048,AK_OpInstance+AK_CHORD2+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD3+0(a5),d4
				add.l	#103936,AK_OpInstance+AK_CHORD3+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	#255,d5
				cmp.w	d5,d6
				blt.s	.NoClampMaxChord_14_1
				move.w	d5,d6
				bra.s	.NoClampMinChord_14_1
.NoClampMaxChord_14_1
				not.w	d5
				cmp.w	d5,d6
				bge.s	.NoClampMinChord_14_1
				move.w	d5,d6
.NoClampMinChord_14_1
				asl.w	#7,d6
				move.w	d6,d0

				; v2 = osc_tri(1, 360, 52)
				add.w	#360,AK_OpInstance+12(a5)
				move.w	AK_OpInstance+12(a5),d1
				bge.s	.TriNoInvert_14_2
				not.w	d1
.TriNoInvert_14_2
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#52,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_14_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_14_3

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
; Instrument 14 - Loop Generator (Offset: 6720 Length: 6720
;----------------------------------------------------------------------------

				move.l	#6720,d7
				move.l	AK_SmpAddr+52(a5),a0
				lea		6720(a0),a0
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
; Instrument 15 - amigahub_chord4
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
				; v1 = chordgen(0, 10, 2, 5, 10, 0)
				move.l	AK_SmpAddr+40(a5),a4
				move.b	(a4,d7.l),d6
				ext.w	d6
				moveq	#0,d4
				move.w	AK_OpInstance+AK_CHORD1+0(a5),d4
				add.l	#73472,AK_OpInstance+AK_CHORD1+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD2+0(a5),d4
				add.l	#87552,AK_OpInstance+AK_CHORD2+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	AK_OpInstance+AK_CHORD3+0(a5),d4
				add.l	#116736,AK_OpInstance+AK_CHORD3+0(a5)
				move.b	(a4,d4.l),d5
				ext.w	d5
				add.w	d5,d6
				move.w	#255,d5
				cmp.w	d5,d6
				blt.s	.NoClampMaxChord_15_1
				move.w	d5,d6
				bra.s	.NoClampMinChord_15_1
.NoClampMaxChord_15_1
				not.w	d5
				cmp.w	d5,d6
				bge.s	.NoClampMinChord_15_1
				move.w	d5,d6
.NoClampMinChord_15_1
				asl.w	#7,d6
				move.w	d6,d0

				; v2 = osc_tri(1, 400, 52)
				add.w	#400,AK_OpInstance+12(a5)
				move.w	AK_OpInstance+12(a5),d1
				bge.s	.TriNoInvert_15_2
				not.w	d1
.TriNoInvert_15_2
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#52,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_15_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_15_3

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
; Instrument 15 - Loop Generator (Offset: 5248 Length: 5248
;----------------------------------------------------------------------------

				move.l	#5248,d7
				move.l	AK_SmpAddr+56(a5),a0
				lea		5248(a0),a0
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
; Instrument 16 - amigahub_kickbass
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
				; v1 = clone(smp,3, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+12(a5),d7
				bge.s	.NoClone_16_1
				move.l	AK_SmpAddr+12(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_16_1

				; v1 = cmb_flt_n(1, v1, 450, 87, 127)
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#87,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.CombAddNoClamp_16_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.CombAddNoClamp_16_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#450<<1,d5
				blt.s	.NoCombReset_16_2
				moveq	#0,d5
.NoCombReset_16_2
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d0
				muls	#127,d0
				asr.l	#7,d0

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

;----------------------------------------------------------------------------
; Instrument 17 - amigahub_bass1
;----------------------------------------------------------------------------

				moveq	#1,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst17Loop
				; v1 = osc_saw(0, 153, 96)
				add.w	#153,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				muls	#96,d0
				asr.l	#7,d0

				; v2 = osc_saw(1, 147, 83)
				add.w	#147,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d1
				muls	#83,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_17_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_17_3

				; v2 = osc_pulse(3, 150, 68, 63)
				add.w	#150,AK_OpInstance+4(a5)
				slt		d1
				ext.w	d1
				eor.w	#$7fff,d1
				muls	#68,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_17_5
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_17_5

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
; Instrument 17 - Loop Generator (Offset: 3072 Length: 3072
;----------------------------------------------------------------------------

				move.l	#3072,d7
				move.l	AK_SmpAddr+64(a5),a0
				lea		3072(a0),a0
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
; Instrument 18 - amigahub_bass2
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
				; v1 = clone(smp,16, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+64(a5),d7
				bge.s	.NoClone_18_1
				move.l	AK_SmpAddr+64(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_18_1

				; v2 = envd(1, 12, 16, 127)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#441344,d5
				cmp.l	#268435456,d5
				bgt.s   .EnvDNoSustain_18_2
				move.l	#268435456,d5
.EnvDNoSustain_18_2
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#127,d1
				asr.l	#7,d1

				; v3 = mul(v2, 110)
				move.w	d1,d2
				muls	#110,d2
				add.l	d2,d2
				swap	d2

				; v1 = sv_flt_n(3, v1, v3, 16, 0)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d2,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_18_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_18_4
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				asl.w	#4,d6
				ext.l	d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_18_4
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_18_4
.NoClampMaxHPF_18_4
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_18_4
				move.w	#-32768,d5
.NoClampMinHPF_18_4
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	d2,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_18_4
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_18_4
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_LPF+0(a5),d0

				; v1 = mul(v2, v1)
				muls	d1,d0
				add.l	d0,d0
				swap	d0

				; v2 = osc_tri(5, 152, 110)
				add.w	#152,AK_OpInstance+6(a5)
				move.w	AK_OpInstance+6(a5),d1
				bge.s	.TriNoInvert_18_6
				not.w	d1
.TriNoInvert_18_6
				sub.w	#16384,d1
				add.w	d1,d1
				muls	#110,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_18_7
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_18_7

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

;----------------------------------------------------------------------------
; Instrument 19 - amigahub_bass3
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst19Loop
				; v1 = clone(smp,16, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+64(a5),d7
				bge.s	.NoClone_19_1
				move.l	AK_SmpAddr+64(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_19_1

				; v2 = envd(1, 9, 22, 127)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#762368,d5
				cmp.l	#369098752,d5
				bgt.s   .EnvDNoSustain_19_2
				move.l	#369098752,d5
.EnvDNoSustain_19_2
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#127,d1
				asr.l	#7,d1

				; v3 = mul(v2, 110)
				move.w	d1,d2
				muls	#110,d2
				add.l	d2,d2
				swap	d2

				; v1 = sv_flt_n(3, v1, v3, 16, 0)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d2,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_19_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_19_4
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				asl.w	#4,d6
				ext.l	d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_19_4
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_19_4
.NoClampMaxHPF_19_4
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_19_4
				move.w	#-32768,d5
.NoClampMinHPF_19_4
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	d2,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_19_4
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_19_4
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_LPF+0(a5),d0

				; v1 = mul(v2, v1)
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
				cmp.l	AK_SmpLen+72(a5),d7
				blt		.Inst19Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 19 - Loop Generator (Offset: 3584 Length: 2560
;----------------------------------------------------------------------------

				move.l	#2560,d7
				move.l	AK_SmpAddr+72(a5),a0
				lea		3584(a0),a0
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
; Instrument 20 - amigahub_bass4
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst20Loop
				; v1 = clone_reverse(smp,16, 0)
				move.l	d7,d6
				moveq	#0,d0
				cmp.l	AK_SmpLen+64(a5),d6
				bge.s	.NoClone_20_1
				move.l	AK_SmpAddr+64+4(a5),a4
				neg.l	d6
				move.b	-1(a4,d6.l),d0
				asl.w	#8,d0
.NoClone_20_1

				; v2 = envd(1, 6, 16, 127)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#1677568,d5
				cmp.l	#268435456,d5
				bgt.s   .EnvDNoSustain_20_2
				move.l	#268435456,d5
.EnvDNoSustain_20_2
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#127,d1
				asr.l	#7,d1

				; v3 = mul(v2, 110)
				move.w	d1,d2
				muls	#110,d2
				add.l	d2,d2
				swap	d2

				; v1 = sv_flt_n(3, v1, v3, 16, 0)
				move.w	AK_OpInstance+AK_BPF+0(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d2,d5
				move.w	AK_OpInstance+AK_LPF+0(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_20_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_20_4
				move.w	d4,AK_OpInstance+AK_LPF+0(a5)
				asl.w	#4,d6
				ext.l	d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_20_4
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_20_4
.NoClampMaxHPF_20_4
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_20_4
				move.w	#-32768,d5
.NoClampMinHPF_20_4
				move.w	d5,AK_OpInstance+AK_HPF+0(a5)
				asr.w	#7,d5
				muls	d2,d5
				add.w	AK_OpInstance+AK_BPF+0(a5),d5
				bvc.s	.NoClampBPF_20_4
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_20_4
				move.w	d5,AK_OpInstance+AK_BPF+0(a5)
				move.w	AK_OpInstance+AK_LPF+0(a5),d0

				; v1 = mul(v2, v1)
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
				cmp.l	AK_SmpLen+76(a5),d7
				blt		.Inst20Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 20 - Loop Generator (Offset: 3584 Length: 2560
;----------------------------------------------------------------------------

				move.l	#2560,d7
				move.l	AK_SmpAddr+76(a5),a0
				lea		3584(a0),a0
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
; Instrument 21 - amigahub_bass+hat
;----------------------------------------------------------------------------

				moveq	#0,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst21Loop
				; v1 = clone(smp,0, 0)
				moveq	#0,d0
				cmp.l	AK_SmpLen+0(a5),d7
				bge.s	.NoClone_21_1
				move.l	AK_SmpAddr+0(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_21_1

				; v2 = clone(smp,17, 0)
				moveq	#0,d1
				cmp.l	AK_SmpLen+68(a5),d7
				bge.s	.NoClone_21_2
				move.l	AK_SmpAddr+68(a5),a4
				move.b	(a4,d7.l),d1
				asl.w	#8,d1
.NoClone_21_2

				; v1 = add(v2, v1)
				add.w	d1,d0
				bvc.s	.AddNoClamp_21_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_21_3

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
; Instrument 22 - amigahub_vocal
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
				; v1 = imported_sample(smp,5)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+20(a5),d7
				bge.s	.NoClone_22_1
				move.l	AK_ExtSmpAddr+20(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_22_1

				; v1 = reverb(v1, 118, 15)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_22_2_0
				moveq	#0,d5
.NoReverbReset_22_2_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_22_2_1
				moveq	#0,d5
.NoReverbReset_22_2_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_22_2_2
				moveq	#0,d5
.NoReverbReset_22_2_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_22_2_3
				moveq	#0,d5
.NoReverbReset_22_2_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_22_2_4
				moveq	#0,d5
.NoReverbReset_22_2_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_22_2_5
				moveq	#0,d5
.NoReverbReset_22_2_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_22_2_6
				moveq	#0,d5
.NoReverbReset_22_2_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#118,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_22_2_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_22_2_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_22_2_7
				moveq	#0,d5
.NoReverbReset_22_2_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				muls	#15,d7
				asr.l	#7,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_22_2
				move.w	#32767,d7
				bra.s	.NoReverbMin_22_2
.NoReverbMax_22_2
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_22_2
				move.w	#-32768,d7
.NoReverbMin_22_2
				move.w	d7,d0
				move.l	(sp)+,d7

				; v1 = vol(v1, 138)
				muls	#138,d0
				asr.l	#7,d0

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

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 22 - Loop Generator (Offset: 14216 Length: 14218
;----------------------------------------------------------------------------

				move.l	#14218,d7
				move.l	AK_SmpAddr+84(a5),a0
				lea		14216(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_21
				moveq	#0,d0
.LoopGenVC_21
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_21
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
				bne.s	.LoopGen_21

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 23 - amigahub_vocal2
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst23Loop
				; v1 = imported_sample(smp,7)
				moveq	#0,d0
				cmp.l	AK_ExtSmpLen+28(a5),d7
				bge.s	.NoClone_23_1
				move.l	AK_ExtSmpAddr+28(a5),a4
				move.b	(a4,d7.l),d0
				asl.w	#8,d0
.NoClone_23_1

				; v1 = reverb(v1, 112, 26)
				move.l	d7,-(sp)
				sub.l	a6,a6
				move.l	a1,a4
				move.w	AK_OpInstance+0(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_0
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_0
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#557<<1,d5
				ble.s	.NoReverbReset_23_2_0
				moveq	#0,d5
.NoReverbReset_23_2_0
				move.w  d5,AK_OpInstance+0(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		4096(a1),a4
				move.w	AK_OpInstance+2(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_1
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_1
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#593<<1,d5
				ble.s	.NoReverbReset_23_2_1
				moveq	#0,d5
.NoReverbReset_23_2_1
				move.w  d5,AK_OpInstance+2(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		8192(a1),a4
				move.w	AK_OpInstance+4(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_2
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_2
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#641<<1,d5
				ble.s	.NoReverbReset_23_2_2
				moveq	#0,d5
.NoReverbReset_23_2_2
				move.w  d5,AK_OpInstance+4(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		12288(a1),a4
				move.w	AK_OpInstance+6(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_3
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_3
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#677<<1,d5
				ble.s	.NoReverbReset_23_2_3
				moveq	#0,d5
.NoReverbReset_23_2_3
				move.w  d5,AK_OpInstance+6(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		16384(a1),a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_4
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_4
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#709<<1,d5
				ble.s	.NoReverbReset_23_2_4
				moveq	#0,d5
.NoReverbReset_23_2_4
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		20480(a1),a4
				move.w	AK_OpInstance+10(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_5
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_5
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#743<<1,d5
				ble.s	.NoReverbReset_23_2_5
				moveq	#0,d5
.NoReverbReset_23_2_5
				move.w  d5,AK_OpInstance+10(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		24576(a1),a4
				move.w	AK_OpInstance+12(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_6
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_6
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#787<<1,d5
				ble.s	.NoReverbReset_23_2_6
				moveq	#0,d5
.NoReverbReset_23_2_6
				move.w  d5,AK_OpInstance+12(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				lea		28672(a1),a4
				move.w	AK_OpInstance+14(a5),d5
				move.w	(a4,d5.w),d4
				muls	#112,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.ReverbAddNoClamp_23_2_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.ReverbAddNoClamp_23_2_7
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#809<<1,d5
				ble.s	.NoReverbReset_23_2_7
				moveq	#0,d5
.NoReverbReset_23_2_7
				move.w  d5,AK_OpInstance+14(a5)
				move.w	d4,d7
				muls	#26,d7
				asr.l	#7,d7
				add.w	d7,a6
				move.l	a6,d7
				cmp.l	#32767,d7
				ble.s	.NoReverbMax_23_2
				move.w	#32767,d7
				bra.s	.NoReverbMin_23_2
.NoReverbMax_23_2
				cmp.l	#-32768,d7
				bge.s	.NoReverbMin_23_2
				move.w	#-32768,d7
.NoReverbMin_23_2
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
				cmp.l	AK_SmpLen+88(a5),d7
				blt		.Inst23Loop

				movem.l a0-a1,-(sp)	;Stash sample base address & large buffer address for loop generator

;----------------------------------------------------------------------------
; Instrument 23 - Loop Generator (Offset: 8698 Length: 6918
;----------------------------------------------------------------------------

				move.l	#6918,d7
				move.l	AK_SmpAddr+88(a5),a0
				lea		8698(a0),a0
				move.l	a0,a1
				sub.l	d7,a1
				moveq	#0,d4
				move.l	#32767<<8,d5
				move.l	d5,d0
				divs	d7,d0
				bvc.s	.LoopGenVC_22
				moveq	#0,d0
.LoopGenVC_22
				moveq	#0,d6
				move.w	d0,d6
.LoopGen_22
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
				bne.s	.LoopGen_22

				movem.l (sp)+,a0-a1	;Restore sample base address & large buffer address after loop generator

;----------------------------------------------------------------------------
; Instrument 24 - amigahub_tom
;----------------------------------------------------------------------------

				moveq	#8,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst24Loop
				; v2 = envd(0, 19, 0, 7)
				move.l	AK_EnvDValue+0(a5),d5
				move.l	d5,d1
				swap	d1
				sub.l	#182272,d5
				bgt.s   .EnvDNoSustain_24_1
				moveq	#0,d5
.EnvDNoSustain_24_1
				move.l	d5,AK_EnvDValue+0(a5)
				muls	#7,d1
				asr.l	#7,d1

				; v3 = envd(1, 15, 0, 127)
				move.l	AK_EnvDValue+4(a5),d5
				move.l	d5,d2
				swap	d2
				sub.l	#289024,d5
				bgt.s   .EnvDNoSustain_24_2
				moveq	#0,d5
.EnvDNoSustain_24_2
				move.l	d5,AK_EnvDValue+4(a5)
				muls	#127,d2
				asr.l	#7,d2

				; v4 = envd(2, 15, 0, 127)
				move.l	AK_EnvDValue+8(a5),d5
				move.l	d5,d3
				swap	d3
				sub.l	#289024,d5
				bgt.s   .EnvDNoSustain_24_3
				moveq	#0,d5
.EnvDNoSustain_24_3
				move.l	d5,AK_EnvDValue+8(a5)
				muls	#127,d3
				asr.l	#7,d3

				; v3 = mul(v3, v4)
				muls	d3,d2
				add.l	d2,d2
				swap	d2

				; v1 = osc_sine(4, v2, 79)
				add.w	d1,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				sub.w	#16384,d0
				move.w	d0,d5
				bge.s	.SineNoAbs_24_5
				neg.w	d5
.SineNoAbs_24_5
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d0
				swap	d0
				asl.w	#3,d0
				muls	#79,d0
				asr.l	#7,d0

				; v2 = clone(smp,6, 0)
				moveq	#0,d1
				cmp.l	AK_SmpLen+24(a5),d7
				bge.s	.NoClone_24_6
				move.l	AK_SmpAddr+24(a5),a4
				move.b	(a4,d7.l),d1
				asl.w	#8,d1
.NoClone_24_6

				; v2 = sv_flt_n(6, v2, 20, 28, 1)
				move.w	AK_OpInstance+AK_BPF+2(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#20,d5
				move.w	AK_OpInstance+AK_LPF+2(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_24_7
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_24_7
				move.w	d4,AK_OpInstance+AK_LPF+2(a5)
				muls	#28,d6
				move.w	d1,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_24_7
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_24_7
.NoClampMaxHPF_24_7
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_24_7
				move.w	#-32768,d5
.NoClampMinHPF_24_7
				move.w	d5,AK_OpInstance+AK_HPF+2(a5)
				asr.w	#7,d5
				muls	#20,d5
				add.w	AK_OpInstance+AK_BPF+2(a5),d5
				bvc.s	.NoClampBPF_24_7
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_24_7
				move.w	d5,AK_OpInstance+AK_BPF+2(a5)
				move.w	AK_OpInstance+AK_HPF+2(a5),d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_24_8
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_24_8

				; v1 = mul(v1, v3)
				muls	d2,d0
				add.l	d0,d0
				swap	d0

				; v1 = cmb_flt_n(9, v1, 267, 59, 127)
				move.l	a1,a4
				move.w	AK_OpInstance+8(a5),d5
				move.w	(a4,d5.w),d4
				muls	#59,d4
				asr.l	#7,d4
				add.w	d0,d4
				bvc.s	.CombAddNoClamp_24_10
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.CombAddNoClamp_24_10
				move.w	d4,(a4,d5.w)
				addq.w	#2,d5
				cmp.w	#267<<1,d5
				blt.s	.NoCombReset_24_10
				moveq	#0,d5
.NoCombReset_24_10
				move.w  d5,AK_OpInstance+8(a5)
				move.w	d4,d0
				muls	#127,d0
				asr.l	#7,d0

				; v1 = sv_flt_n(10, v1, 24, 42, 1)
				move.w	AK_OpInstance+AK_BPF+10(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#24,d5
				move.w	AK_OpInstance+AK_LPF+10(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_24_11
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_24_11
				move.w	d4,AK_OpInstance+AK_LPF+10(a5)
				muls	#42,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_24_11
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_24_11
.NoClampMaxHPF_24_11
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_24_11
				move.w	#-32768,d5
.NoClampMinHPF_24_11
				move.w	d5,AK_OpInstance+AK_HPF+10(a5)
				asr.w	#7,d5
				muls	#24,d5
				add.w	AK_OpInstance+AK_BPF+10(a5),d5
				bvc.s	.NoClampBPF_24_11
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_24_11
				move.w	d5,AK_OpInstance+AK_BPF+10(a5)
				move.w	AK_OpInstance+AK_HPF+10(a5),d0

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
; Instrument 25 - amigahub_lead
;----------------------------------------------------------------------------

				moveq	#1,d0
				bsr		AK_ResetVars
				moveq	#0,d7
				ifne	AK_USE_PROGRESS
					ifeq	AK_FINE_PROGRESS
						addq.b	#1,(a3)
					endif
				endif
.Inst25Loop
				; v1 = osc_saw(0, 1800, 63)
				add.w	#1800,AK_OpInstance+0(a5)
				move.w	AK_OpInstance+0(a5),d0
				muls	#63,d0
				asr.l	#7,d0

				; v2 = osc_saw(1, 1810, 50)
				add.w	#1810,AK_OpInstance+2(a5)
				move.w	AK_OpInstance+2(a5),d1
				muls	#50,d1
				asr.l	#7,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_25_3
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_25_3

				; v2 = osc_pulse(3, 900, 64, v3)
				add.w	#900,AK_OpInstance+4(a5)
				move.w	d2,d4
				and.w	#255,d4
				sub.w	#63,d4
				asl.w	#8,d4
				add.w	d4,d4
				cmp.w	AK_OpInstance+4(a5),d4
				slt		d1
				ext.w	d1
				eor.w	#$7fff,d1
				asr.w	#1,d1

				; v1 = add(v1, v2)
				add.w	d1,d0
				bvc.s	.AddNoClamp_25_5
				spl		d0
				ext.w	d0
				eor.w	#$7fff,d0
.AddNoClamp_25_5

				; v2 = osc_sine(5, 3, 112)
				add.w	#3,AK_OpInstance+6(a5)
				move.w	AK_OpInstance+6(a5),d1
				sub.w	#16384,d1
				move.w	d1,d5
				bge.s	.SineNoAbs_25_6
				neg.w	d5
.SineNoAbs_25_6
				move.w	#32767,d6
				sub.w	d5,d6
				muls	d6,d1
				swap	d1
				asl.w	#3,d1
				muls	#112,d1
				asr.l	#7,d1

				; v3 = ctrl(v2)
				move.w	d1,d2
				moveq	#9,d4
				asr.w	d4,d2
				add.w	#64,d2

				; v1 = sv_flt_n(7, v1, v3, 46, 2)
				move.w	AK_OpInstance+AK_BPF+8(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	d2,d5
				move.w	AK_OpInstance+AK_LPF+8(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_25_8
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_25_8
				move.w	d4,AK_OpInstance+AK_LPF+8(a5)
				muls	#46,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_25_8
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_25_8
.NoClampMaxHPF_25_8
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_25_8
				move.w	#-32768,d5
.NoClampMinHPF_25_8
				move.w	d5,AK_OpInstance+AK_HPF+8(a5)
				asr.w	#7,d5
				muls	d2,d5
				add.w	AK_OpInstance+AK_BPF+8(a5),d5
				bvc.s	.NoClampBPF_25_8
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_25_8
				move.w	d5,AK_OpInstance+AK_BPF+8(a5)
				move.w	d5,d0

				; v1 = sv_flt_n(8, v1, 37, 127, 2)
				move.w	AK_OpInstance+AK_BPF+14(a5),d5
				asr.w	#7,d5
				move.w	d5,d6
				muls	#37,d5
				move.w	AK_OpInstance+AK_LPF+14(a5),d4
				add.w	d5,d4
				bvc.s	.NoClampLPF_25_9
				spl		d4
				ext.w	d4
				eor.w	#$7fff,d4
.NoClampLPF_25_9
				move.w	d4,AK_OpInstance+AK_LPF+14(a5)
				muls	#127,d6
				move.w	d0,d5
				ext.l	d5
				ext.l	d4
				sub.l	d4,d5
				sub.l	d6,d5
				cmp.l	#32767,d5
				ble.s	.NoClampMaxHPF_25_9
				move.w	#32767,d5
				bra.s	.NoClampMinHPF_25_9
.NoClampMaxHPF_25_9
				cmp.l	#-32768,d5
				bge.s	.NoClampMinHPF_25_9
				move.w	#-32768,d5
.NoClampMinHPF_25_9
				move.w	d5,AK_OpInstance+AK_HPF+14(a5)
				asr.w	#7,d5
				muls	#37,d5
				add.w	AK_OpInstance+AK_BPF+14(a5),d5
				bvc.s	.NoClampBPF_25_9
				spl		d5
				ext.w	d5
				eor.w	#$7fff,d5
.NoClampBPF_25_9
				move.w	d5,AK_OpInstance+AK_BPF+14(a5)
				move.w	d5,d0

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
; Instrument 25 - Loop Generator (Offset: 16400 Length: 16402
;----------------------------------------------------------------------------

				move.l	#16402,d7
				move.l	AK_SmpAddr+96(a5),a0
				lea		16400(a0),a0
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
				move.l	d0,(a6)+
				move.l  #32767<<16,d6
				move.l	d6,(a6)+
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
AK_SmpAddr		rs.l	31
AK_ExtSmpAddr	rs.l	8
AK_OpInstance	rs.w    12
AK_EnvDValue	rs.l	3
AK_VarSize		rs.w	0

AK_Vars:
				dc.l	$00000740		; Instrument 1 Length 
				dc.l	$00000740		; Instrument 2 Length 
				dc.l	$00002500		; Instrument 3 Length 
				dc.l	$00000e48		; Instrument 4 Length 
				dc.l	$00000510		; Instrument 5 Length 
				dc.l	$00001010		; Instrument 6 Length 
				dc.l	$00001200		; Instrument 7 Length 
				dc.l	$000008d4		; Instrument 8 Length 
				dc.l	$00000200		; Instrument 9 Length 
				dc.l	$00001ee0		; Instrument 10 Length 
				dc.l	$00007400		; Instrument 11 Length 
				dc.l	$00004000		; Instrument 12 Length 
				dc.l	$00003600		; Instrument 13 Length 
				dc.l	$00003480		; Instrument 14 Length 
				dc.l	$00002900		; Instrument 15 Length 
				dc.l	$00001de6		; Instrument 16 Length 
				dc.l	$00001800		; Instrument 17 Length 
				dc.l	$00001800		; Instrument 18 Length 
				dc.l	$00001800		; Instrument 19 Length 
				dc.l	$00001800		; Instrument 20 Length 
				dc.l	$00001800		; Instrument 21 Length 
				dc.l	$00006f12		; Instrument 22 Length 
				dc.l	$00003d00		; Instrument 23 Length 
				dc.l	$000017c0		; Instrument 24 Length 
				dc.l	$00008022		; Instrument 25 Length 
				dc.l	$00000000		; Instrument 26 Length 
				dc.l	$00000000		; Instrument 27 Length 
				dc.l	$00000000		; Instrument 28 Length 
				dc.l	$00000000		; Instrument 29 Length 
				dc.l	$00000000		; Instrument 30 Length 
				dc.l	$00000000		; Instrument 31 Length 
				dc.l	$00000776		; External Sample 1 Length 
				dc.l	$00000e7c		; External Sample 2 Length 
				dc.l	$00000e48		; External Sample 3 Length 
				dc.l	$00001010		; External Sample 4 Length 
				dc.l	$0000090e		; External Sample 5 Length 
				dc.l	$00001674		; External Sample 6 Length 
				dc.l	$00000fa8		; External Sample 7 Length 
				dc.l	$0000153c		; External Sample 8 Length 
				ds.b	AK_VarSize-AK_SmpAddr

;----------------------------------------------------------------------------
