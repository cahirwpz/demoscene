        IFND PLAYER61_I
PLAYER61_I SET 1

**      $Filename: Player61.i $
**      $Release: 6.1A $
**      $Revision: 610.6 $
**      $Date: 04/07/25 $
**
**      The Player 6.1A definitions
**
**      (C) Copyright 1992-94 Jarno Paananen
**      All Rights Reserved
**      Fixed 1998-04 by NoName, Platon42, Tolkien and The Dark Coder
**

        IFND EXEC_TYPES_I
        include exec/types.i
        ENDC

**************
* The header *
**************

  STRUCTURE Player_Header,0
** Instructions to jump to P61_Init
        ULONG   P61_InitOffset
** ... to P61_Music (rts, if CIA-Version)
        ULONG   P61_MusicOffset
** ... to P61_End
        ULONG   P61_EndOffset
** ... to P61_SetRepeat (if present, otherwise rts)
        ULONG   P61_SetRepeatOffset
** ... to P61_SetPosition
        ULONG   P61_SetPositionOffset
** Master volume (used if told to...)
        UWORD   P61_MasterVolume
** If non-zero, tempo will be used
        UWORD   P61_UseTempo
** If zero, playing is stopped
        UWORD   P61_PlayFlag
** Info nybble after command E8
        UWORD   P61_E8_info
** Vector Base Register VBR passed to the player (default 0)
        APTR    P61_UseVBR
** Current song position
        UWORD   P61_Position
** Current pattern
        UWORD   P61_Pattern
** Current row
        UWORD   P61_Row
** Offset to channel 0 block from the beginning
        APTR    P61_Cha0Offset
** Offset to channel 1 block from the beginning
        APTR    P61_Cha1Offset
** Offset to channel 2 block from the beginning
        APTR    P61_Cha2Offset
** Offset to channel 3 block from the beginning
        APTR    P61_Cha3Offset

        LABEL Player_Header_SIZE


*********************************************************
** The structure of the channel blocks (P61_Temp[0-3]) **
*********************************************************

  STRUCTURE Channel_Block,0

** Note and the MSB of the sample number
        UBYTE   P61_SN_Note
** Lower nybble of the sample number and the command
        UBYTE   P61_Command
** Info byte
        UBYTE   P61_Info
** Packing info
        UBYTE   P61_Pack
** Pointer to the sample block of the current sample
        APTR    P61_Sample
** Current note (offset to the period table)
        UWORD   P61_Note
** Period
        UWORD   P61_Period
** Volume (NOT updated in tremolo!)
        UWORD   P61_Volume
** Current finetune
        UWORD   P61_Fine
** Sample offset
        UWORD   P61_Offset
** Last sample Offset
        UWORD   P61_LOffset
** To period for tone portamento
        UWORD   P61_ToPeriod
** Speed for tone portamento
        UWORD   P61_TPSpeed
** Vibrato command
        UBYTE   P61_VibCmd
** Vibrato position
        UBYTE   P61_VibPos
** Tremolo command
        UBYTE   P61_TreCmd
** Tremolo position
        UBYTE   P61_TrePos
** Retrig note counter
        UWORD   P61_RetrigCount

** Invert loop speed
        UBYTE   P61_Funkspd
** Invert loop offset
        UBYTE   P61_Funkoff
** Invert loop offset
        APTR    P61_Wave

** Internal switch to the packing
        UWORD   P61_OnOff
** Pointer to the current pattern data
        APTR    P61_ChaPos
** A packing pointer to data elsewhere in the pattern data
        APTR    P61_TempPos
** Lenght of the temporary positions
        UWORD   P61_TempLen
** Temp pointers for patternloop
        UWORD   P61_TData
        APTR    P61_TChaPos
        APTR    P61_TTempPos
        UWORD   P61_TTempLen

** Shadow address for fading (updated also in tremolo!)
        UWORD   P61_Shadow

** Bit in DMACON ($DFF096)
        UWORD   P61_DMABit

        LABEL Channel_Block_SIZE



************************************************
** The structure of the sample block that     **
** the Player does at the init to P61_Samples **
************************************************

  STRUCTURE Sample_Block,0

** Pointer to the beginning of the sample
        APTR    P61_SampleOffset
** Lenght of the sample
        UWORD   P61_SampleLength
** Pointer to the repeat
        APTR    P61_RepeatOffset
** Lenght of the repeat
        UWORD   P61_RepeatLength
** Volume of the sample
        UWORD   P61_SampleVolume
** Finetune (offset to the period table)
        UWORD   P61_FineTune

        LABEL Sample_Block_SIZE

************************************************
** Some internal stuff for the Usecode-system **
************************************************


** if finetune is used
P61_ft = use&1
** portamento up
P61_pu = use&2
** portamento down
P61_pd = use&4
** tone portamento
P61_tp = use&40
** vibrato
P61_vib = use&80
** tone portamento and volume slide
P61_tpvs = use&32
** vibrato and volume slide
P61_vbvs = use&64
** tremolo
P61_tre = use&$80
** arpeggio
P61_arp = use&$100
** sample offset
P61_sof = use&$200
** volume slide
P61_vs = use&$400
** position jump
P61_pj = use&$800
** set volume
P61_vl = use&$1000
** pattern break
P61_pb = use&$2800
** set speed
P61_sd = use&$8000

** E-commands
P61_ec = use&$ffff0000

** filter
P61_fi = use&$10000
** fine slide up
P61_fsu = use&$20000
** fine slide down
P61_fsd = use&$40000
** set finetune
P61_sft = use&$200000
** pattern loop
P61_pl = use&$400000
** E8 for timing purposes
P61_timing = use&$1000000
** retrig note
P61_rt = use&$2000000
** fine volume slide up
P61_fvu = use&$4000000
** fine volume slide down
P61_fvd = use&$8000000
** note cut
P61_nc = use&$10000000
** note delay
P61_nd = use&$20000000
** pattern delay
P61_pde = use&$40000000
** invert loop
P61_il = use&$80000000

   ENDC ; PLAYER61_I
