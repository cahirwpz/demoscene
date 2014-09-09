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

__regargs static void BLTOP_NAME(WORD dx, WORD dy, WORD sx, WORD sy) {
  ULONG dst_begin = ((dx & ~15) >> 3) + ((WORD)dy * BLTOP_DST_WIDTH / 8);
  UWORD dst_modulo = (BLTOP_DST_WIDTH - (BLTOP_HSIZE + 16)) / 8;
  ULONG src_begin = ((sx & ~15) >> 3) + ((WORD)sy * BLTOP_SRC_WIDTH / 8);
  UWORD src_shift = (dx & 15) << ASHIFTSHIFT;
  APTR *__src = (BLTOP_SRC_BM)->planes;
  APTR *__dst = (BLTOP_DST_BM)->planes;
  APTR *__carry = (BLTOP_CARRY_BM)->planes;
  LONG i, k;

  /* Bitplane adder with saturation. */
  custom->bltamod = -2;
  custom->bltbmod = dst_modulo;
  custom->bltcmod = 0;
  custom->bltcon1 = 0;
  custom->bltafwm = -1;
  custom->bltalwm = 0;

  /* Bitplane 0: half adder with carry. */
  custom->bltapt = __src[0] + src_begin;
  custom->bltbpt = __dst[0] + dst_begin;
  custom->bltdpt = __carry[0];
  custom->bltdmod = 0;
  custom->bltcon0 = HALF_ADDER_CARRY | src_shift;
  custom->bltsize = BLTOP_SIZE;
  BLTOP_WAIT;

  custom->bltapt = __src[0] + src_begin;
  custom->bltbpt = __dst[0] + dst_begin;
  custom->bltdpt = __dst[0] + dst_begin;
  custom->bltdmod = dst_modulo;
  custom->bltcon0 = HALF_ADDER | src_shift;
  custom->bltsize = BLTOP_SIZE;
  BLTOP_WAIT;

  /* Bitplane 1-5: full adder with carry. */
  for (i = 1, k = 0; i < BLTOP_BPLS; i++, k ^= 1) {
    custom->bltapt = __src[i] + src_begin;
    custom->bltbpt = __dst[i] + dst_begin;
    custom->bltcpt = __carry[k];
    custom->bltdpt = __carry[k ^ 1];
    custom->bltdmod = 0;
    custom->bltcon0 = FULL_ADDER_CARRY | src_shift;
    custom->bltsize = BLTOP_SIZE;
    BLTOP_WAIT;

    custom->bltapt = __src[i] + src_begin;
    custom->bltbpt = __dst[i] + dst_begin;
    custom->bltcpt = __carry[k];
    custom->bltdpt = __dst[i] + dst_begin;
    custom->bltdmod = dst_modulo;
    custom->bltcon0 = FULL_ADDER | src_shift;
    custom->bltsize = BLTOP_SIZE;
    BLTOP_WAIT;
  }

  /* Apply saturation bits. */
  custom->bltamod = dst_modulo;
  custom->bltbmod = 0;
  custom->bltdmod = dst_modulo;
  custom->bltcon0 = (SRCA | SRCB | DEST) | A_OR_B;
  custom->bltalwm = -1;

  for (i = 0; i < BLTOP_BPLS; i++) {
    custom->bltapt = __dst[i] + dst_begin;
    custom->bltbpt = __carry[k];
    custom->bltdpt = __dst[i] + dst_begin;
    custom->bltsize = BLTOP_SIZE;
    BLTOP_WAIT;
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
