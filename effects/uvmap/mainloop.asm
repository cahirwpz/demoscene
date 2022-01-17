        section ".text"

                                ; cpu/bus cycles
        move.w  $1111(a1),d0    ; 12/3
        or.w    $2222(a2),d0    ; 12/3
        move.b  $3333(a1),d0    ; 12/3
        or.b    $4444(a2),d0    ; 12/3
        move.w  d0,(a0)+        ; 8/2
                                ; 14/3.5 per pixel  
