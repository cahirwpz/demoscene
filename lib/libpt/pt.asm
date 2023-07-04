        xdef    _PtInstallCIA
        xdef    _PtRemoveCIA
        xdef    _PtInit
        xdef    _PtEnd
        xdef    _PtData
        xdef    _PtEnable
        xdef    _PtE8Trigger

        section '.text',code

_PtData         set     mt_data+mt_chan1
_PtEnable       set     _mt_Enable
_PtE8Trigger    set     _mt_E8Trigger

_PtInstallCIA:
        movem.l d2-d7/a2-a6,-(sp)
        st.b    d0
        move.l  _ExcVecBase,a0
        lea     $dff000,a6
        bsr     _mt_install_cia
        movem.l (sp)+,d2-d7/a2-a6
        rts

_PtRemoveCIA:
        movem.l d2-d7/a2-a6,-(sp)
        lea     $dff000,a6
        bsr     _mt_remove_cia
        movem.l (sp)+,d2-d7/a2-a6
        rts

_PtInit:
        movem.l d2-d7/a2-a6,-(sp)
        lea     $dff000,a6
        bsr     _mt_init
        movem.l (sp)+,d2-d7/a2-a6
        rts

_PtEnd:
        movem.l d2-d7/a2-a6,-(sp)
        lea     $dff000,a6
        bsr     _mt_end
        movem.l (sp)+,d2-d7/a2-a6
        rts

        include 'ptplayer.asm'
