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
  APTR borrow0 = (BLTOP_BORROW_BM)->planes[0];
  APTR borrow1 = (BLTOP_BORROW_BM)->planes[1];
  APTR *dst = (BLTOP_DST_BM)->planes;
  APTR ptr = *dst++;
  WORD n = BLTOP_BPLS - 1;

  BLTOP_WAIT;
  custom->bltcon1 = 0;
  custom->bltamod = 0;
  custom->bltbdat = -1;
  custom->bltbmod = 0;
  custom->bltdmod = 0;
  custom->bltafwm = -1;
  custom->bltalwm = -1;

  custom->bltapt = ptr;
  custom->bltdpt = borrow0;
  custom->bltcon0 = HALF_SUB_BORROW & ~SRCB;
  custom->bltsize = BLTOP_SIZE;

  BLTOP_WAIT;
  custom->bltapt = ptr;
  custom->bltdpt = ptr;
  custom->bltcon0 = HALF_SUB & ~SRCB;
  custom->bltsize = BLTOP_SIZE;

  while (--n >= 0) {
    ptr = *dst++;

    BLTOP_WAIT;
    custom->bltapt = ptr;
    custom->bltbpt = borrow0;
    custom->bltdpt = borrow1;
    custom->bltcon0 = HALF_SUB_BORROW;
    custom->bltsize = BLTOP_SIZE;

    BLTOP_WAIT;
    custom->bltapt = ptr;
    custom->bltbpt = borrow0;
    custom->bltdpt = ptr;
    custom->bltcon0 = HALF_SUB;
    custom->bltsize = BLTOP_SIZE;

    swapr(borrow0, borrow1);
  }

  dst = (BLTOP_DST_BM)->planes;
  n = BLTOP_BPLS;
  while (--n >= 0) {
    ptr = *dst++;

    BLTOP_WAIT;
    custom->bltapt = ptr;
    custom->bltbpt = borrow0;
    custom->bltdpt = ptr;
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
