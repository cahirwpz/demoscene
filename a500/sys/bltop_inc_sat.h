#ifndef BLTOP_NAME
#error "BLTOP: function name not defined!"
#endif

#ifndef BLTOP_CARRY_BM
#error "BLTOP: carry bitmap not defined!"
#endif

#ifndef BLTOP_DST_BM
#error "BLTOP: destination bitmap not defined!"
#endif

#ifndef BLTOP_BPLS
#error "BLTOP: number of bitplanes not defined!"
#endif

#if !defined(BLTOP_HSIZE) || !defined(BLTOP_VSIZE)
#error "BLTOP: blit size not defined!"
#else
#define BLTOP_SIZE (BLTOP_HSIZE << 6) | (BLTOP_VSIZE >> 4)
#endif

#ifndef BLTOP_WAIT
#define BLTOP_WAIT WaitBlitter()
#endif

/* Bitplane incrementer with saturation. */
__regargs static void BLTOP_NAME() {
  APTR carry0 = (BLTOP_CARRY_BM)->planes[0];
  APTR carry1 = (BLTOP_CARRY_BM)->planes[1];
  APTR *dst = (BLTOP_DST_BM)->planes;
  APTR ptr;
  WORD n = BLTOP_BPLS;

  /* Only pixels set to one in carry[0] will be incremented. */
  
  BLTOP_WAIT;
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  while (--n >= 0) {
    ptr = *dst++;

    BLTOP_WAIT;
    custom->bltapt = ptr;
    custom->bltbpt = carry0;
    custom->bltdpt = carry1;
    custom->bltcon0 = HALF_ADDER_CARRY;
    custom->bltsize = BLTOP_SIZE;

    BLTOP_WAIT;
    custom->bltapt = ptr;
    custom->bltbpt = carry0;
    custom->bltdpt = ptr;
    custom->bltcon0 = HALF_ADDER;
    custom->bltsize = BLTOP_SIZE;

    swapr(carry0, carry1);
  }

  dst = (BLTOP_DST_BM)->planes;
  n = BLTOP_BPLS;
  while (--n >= 0) {
    ptr = *dst++;

    BLTOP_WAIT;
    custom->bltapt = ptr;
    custom->bltbpt = carry0;
    custom->bltdpt = ptr;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltsize = BLTOP_SIZE;
  }
}

#undef BLTOP_NAME
#undef BLTOP_CARRY_BM
#undef BLTOP_DST_BM
#undef BLTOP_BPLS
#undef BLTOP_HSIZE
#undef BLTOP_VSIZE
#undef BLTOP_SIZE
#undef BLTOP_WAIT
