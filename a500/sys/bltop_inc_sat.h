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
  APTR *__carry = (BLTOP_CARRY_BM)->planes;
  APTR *__dst = (BLTOP_DST_BM)->planes;
  WORD i, k;

  /* Only pixels set to one in carry[0] will be incremented. */
  
  BLTOP_WAIT;
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  for (i = 0, k = 0; i < 4; i++, k ^= 1) {
    BLTOP_WAIT;
    custom->bltapt = __dst[i];
    custom->bltbpt = __carry[k];
    custom->bltdpt = __carry[k ^ 1];
    custom->bltcon0 = HALF_ADDER_CARRY;
    custom->bltsize = BLTOP_SIZE;

    custom->bltapt = __dst[i];
    custom->bltbpt = __carry[k];
    custom->bltdpt = __dst[i];
    custom->bltcon0 = HALF_ADDER;
    custom->bltsize = BLTOP_SIZE;
    BLTOP_WAIT;
  }

  for (i = 0; i < BLTOP_BPLS; i++) {
    BLTOP_WAIT;
    custom->bltapt = __dst[i];
    custom->bltbpt = __carry[k];
    custom->bltdpt = __dst[i];
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
