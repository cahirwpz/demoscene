        section ".text"

        move.w  $1111(a1),d0   ; hi
        or.w    $2222(a2),d0   ; lo
        move.b  $3333(a1),d0   ; hi
        or.b    $4444(a2),d0   ; lo
        move.w  d0,(a0)+

