#ifndef BLTOP_NAME
#error "BLTOP: function name not defined!"
#endif

#ifndef BLTOP_SRC_BM
#error "BLTOP: source bitmap not defined!"
#endif

#ifndef BLTOP_SRC_WIDTH
#error "BLTOP: source width not defined!"
#endif

#ifndef BLTOP_CARRY_BM
#error "BLTOP: carry bit bitmap not defined!"
#endif

#ifndef BLTOP_DST_BM
#error "BLTOP: destination bitmap not defined!"
#endif

#ifndef BLTOP_DST_WIDTH
#error "BLTOP: destination width not defined!"
#endif

#ifndef BLTOP_BPLS
#error "BLTOP: number of bitplanes not defined!"
#endif

#if !defined(BLTOP_HSIZE) || !defined(BLTOP_VSIZE)
#error "BLTOP: blit size not defined!"
#else
#define BLTOP_SIZE (BLTOP_HSIZE << 6) | ((BLTOP_VSIZE + 16) >> 4)
#endif

#ifndef BLTOP_WAIT
#define BLTOP_WAIT WaitBlitter()
#endif

/* Bitplane adder with saturation. */
static __regargs void BLTOP_NAME(WORD dx, WORD dy, WORD sx, WORD sy) {
  LONG dst_begin = ((dx & ~15) >> 3) + ((WORD)dy * BLTOP_DST_WIDTH / 8);
  WORD dst_modulo = (BLTOP_DST_WIDTH - (BLTOP_HSIZE + 16)) / 8;
  LONG src_begin = ((sx & ~15) >> 3) + ((WORD)sy * BLTOP_SRC_WIDTH / 8);
  WORD src_shift = (dx & 15) << ASHIFTSHIFT;
  APTR carry0 = (BLTOP_CARRY_BM)->planes[0];
  APTR carry1 = (BLTOP_CARRY_BM)->planes[1];
  APTR *src = (BLTOP_SRC_BM)->planes;
  APTR *dst = (BLTOP_DST_BM)->planes;

  {
    APTR aptr = (*src++) + src_begin;
    APTR bptr = (*dst++) + dst_begin;

    BLTOP_WAIT;

    /* Initialize blitter */
    custom->bltamod = -2;
    custom->bltbmod = dst_modulo;
    custom->bltcmod = 0;
    custom->bltcon1 = 0;
    custom->bltafwm = -1;
    custom->bltalwm = 0;

    /* Bitplane 0: half adder with carry. */
    custom->bltapt = aptr;
    custom->bltbpt = bptr;
    custom->bltdpt = carry0;
    custom->bltdmod = 0;
    custom->bltcon0 = HALF_ADDER_CARRY | src_shift;
    custom->bltsize = BLTOP_SIZE;

    BLTOP_WAIT;
    custom->bltapt = aptr;
    custom->bltbpt = bptr;
    custom->bltdpt = bptr;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = HALF_ADDER | src_shift;
    custom->bltsize = BLTOP_SIZE;
  }

  {
    WORD n = BLTOP_BPLS - 1;

    /* Bitplane 1-5: full adder with carry. */
    while (--n >= 0) {
      APTR aptr = (*src++) + src_begin;
      APTR bptr = (*dst++) + dst_begin;

      BLTOP_WAIT;
      custom->bltapt = aptr;
      custom->bltbpt = bptr;
      custom->bltcpt = carry0;
      custom->bltdpt = carry1;
      custom->bltdmod = 0;
      custom->bltcon0 = FULL_ADDER_CARRY | src_shift;
      custom->bltsize = BLTOP_SIZE;

      BLTOP_WAIT;
      custom->bltapt = aptr;
      custom->bltbpt = bptr;
      custom->bltcpt = carry0;
      custom->bltdpt = bptr;
      custom->bltdmod = dst_modulo;
      custom->bltcon0 = FULL_ADDER | src_shift;
      custom->bltsize = BLTOP_SIZE;

      swapr(carry0, carry1);
    }
  }

  /* Apply saturation bits. */
  {
    WORD n = BLTOP_BPLS;

    dst = (BLTOP_DST_BM)->planes;

    BLTOP_WAIT;
    custom->bltamod = dst_modulo;
    custom->bltbmod = 0;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
    custom->bltalwm = -1;

    while (--n >= 0) {
      APTR bptr = (*dst++) + dst_begin;

      BLTOP_WAIT;
      custom->bltapt = bptr;
      custom->bltbpt = carry0;
      custom->bltdpt = bptr;
      custom->bltsize = BLTOP_SIZE;
    }
  }
}

#undef BLTOP_NAME
#undef BLTOP_SRC_BM
#undef BLTOP_SRC_WIDTH
#undef BLTOP_CARRY_BM
#undef BLTOP_DST_BM
#undef BLTOP_DST_WIDTH
#undef BLTOP_BPLS
#undef BLTOP_HSIZE
#undef BLTOP_VSIZE
#undef BLTOP_SIZE
#undef BLTOP_WAIT
