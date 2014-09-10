#ifndef BLTOP_NAME
#error "BLTOP: function name not defined!"
#endif

#ifndef BLTOP_BORROW_BM
#error "BLTOP: borrow bitmap not defined!"
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

/* Bitplane decrementer with saturation. */
__regargs static void BLTOP_NAME() {
  APTR *__borrow = (BLTOP_BORROW_BM)->planes;
  APTR *__dst = (BLTOP_DST_BM)->planes;
  WORD i, k;
  
  BLTOP_WAIT;
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbdat = -1;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltapt = __dst[0];
  custom->bltdpt = __borrow[0];
  custom->bltcon0 = HALF_SUB_BORROW & ~SRCB;
  custom->bltsize = BLTOP_SIZE;

  BLTOP_WAIT;
  custom->bltapt = __dst[0];
  custom->bltdpt = __dst[0];
  custom->bltcon0 = HALF_SUB & ~SRCB;
  custom->bltsize = BLTOP_SIZE;

  for (i = 1, k = 0; i < BLTOP_BPLS; i++, k ^= 1) {
    BLTOP_WAIT;
    custom->bltapt = __dst[i];
    custom->bltbpt = __borrow[k];
    custom->bltdpt = __borrow[k ^ 1];
    custom->bltcon0 = HALF_SUB_BORROW;
    custom->bltsize = BLTOP_SIZE;

    BLTOP_WAIT;
    custom->bltapt = __dst[i];
    custom->bltbpt = __borrow[k];
    custom->bltdpt = __dst[i];
    custom->bltcon0 = HALF_SUB;
    custom->bltsize = BLTOP_SIZE;
  }

  for (i = 0; i < BLTOP_BPLS; i++) {
    BLTOP_WAIT;
    custom->bltapt = __dst[i];
    custom->bltbpt = __borrow[k];
    custom->bltdpt = __dst[i];
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_AND_NOT_B;
    custom->bltsize = BLTOP_SIZE;
  }
}

#undef BLTOP_NAME
#undef BLTOP_BORROW_BM
#undef BLTOP_DST_BM
#undef BLTOP_BPLS
#undef BLTOP_HSIZE
#undef BLTOP_VSIZE
#undef BLTOP_SIZE
#undef BLTOP_WAIT
