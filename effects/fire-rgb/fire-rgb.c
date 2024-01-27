#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <pixmap.h>
#include <system/interrupt.h>
#include <system/memory.h>
#include <common.h>

#define WIDTH 80
#define HEIGHT 64
#define DEPTH 4

static short *chunky[2];
static short *fire;
static BitmapT *screen[2];
static short active = 0;
static CopListT *cp;
static CopInsPairT *bplptr;

#include "data/dualtab.c"

static struct {
  short phase;
  void **bpl;
  void *chunky;
} c2p = { 256, NULL, NULL };

#define BPLSIZE ((WIDTH * 4) * HEIGHT / 8) /* 2560 bytes */
#define BLTSIZE ((WIDTH * 4) * HEIGHT / 2) /* 10240 bytes */

static void ChunkyToPlanar(void) {
  void *src = c2p.chunky;
  void *dst = c2p.chunky + BLTSIZE;
  void **bpl = c2p.bpl;

  switch (c2p.phase) {
    case 0:
      /* Initialize chunky to planar. */
      custom->bltamod = 4;
      custom->bltbmod = 4;
      custom->bltdmod = 4;
      custom->bltcdat = 0x00FF;
      custom->bltafwm = -1;
      custom->bltalwm = -1;

      /* Swap 8x4, pass 1. */
      custom->bltapt = src + 4;
      custom->bltbpt = src;
      custom->bltdpt = dst;

      /* ((a >> 8) & 0x00FF) | (b & ~0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC) | ASHIFT(8);
      custom->bltcon1 = 0;
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 1:
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 2:
      /* Swap 8x4, pass 2. */
      custom->bltapt = src + BLTSIZE - 6;
      custom->bltbpt = src + BLTSIZE - 2;
      custom->bltdpt = dst + BLTSIZE - 2;

      /* ((a << 8) & ~0x00FF) | (b & 0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABNC | ANBNC | ABC | NABC) | ASHIFT(8);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 3:
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 4:
      custom->bltamod = 6;
      custom->bltbmod = 6;
      custom->bltdmod = 0;
      custom->bltcdat = 0x0F0F;

      custom->bltapt = dst + 2;
      custom->bltbpt = dst;
      custom->bltdpt = bpl[0];

      /* ((a >> 4) & 0x0F0F) | (b & ~0x0F0F) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC) | ASHIFT(4);
      custom->bltcon1 = 0;
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 5:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 6:
      custom->bltapt = dst + 6;
      custom->bltbpt = dst + 4;
      custom->bltdpt = bpl[2];
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 7:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 8:
      custom->bltapt = dst + BLTSIZE - 8;
      custom->bltbpt = dst + BLTSIZE - 6;
      custom->bltdpt = bpl[1] + BPLSIZE - 2;

      /* ((a << 8) & ~0x0F0F) | (b & 0x0F0F) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABNC | ANBNC | ABC | NABC) | ASHIFT(4);
      custom->bltcon1 = BLITREVERSE;
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 9:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 10:
      custom->bltapt = dst + BLTSIZE - 4;
      custom->bltbpt = dst + BLTSIZE - 2;
      custom->bltdpt = bpl[3] + BPLSIZE - 2;
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 11:
      custom->bltsize = 1 | ((BLTSIZE / 16) << 6);
      break;

    case 12:
      CopInsSet32(&bplptr[0], bpl[3]);
      CopInsSet32(&bplptr[1], bpl[2]);
      CopInsSet32(&bplptr[2], bpl[1]);
      CopInsSet32(&bplptr[3], bpl[0]);
      break;

    default:
      break;
  }

  c2p.phase++;

  ClearIRQ(INTF_BLIT);
}

static CopListT *MakeCopperList(void) {
  CopListT *cp = NewCopList(1200);
  short i;

  bplptr = CopSetupBitplanes(cp, screen[0], DEPTH);
  CopLoadColor(cp, 0, 15, 0);
  for (i = 0; i < HEIGHT * 4; i++) {
    CopWaitSafe(cp, Y(i), 0);
    /* Line quadrupling. */
    CopMove16(cp, bpl1mod, ((i & 3) != 3) ? -40 : 0);
    CopMove16(cp, bpl2mod, ((i & 3) != 3) ? -40 : 0);
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0022 : 0x0000);
  }
  return CopListFinish(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH, BM_CLEAR);
  screen[1] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH, BM_CLEAR);
  chunky[0] = MemAlloc(WIDTH * 4 * HEIGHT, MEMF_CHIP);
  chunky[1] = MemAlloc(WIDTH * 4 * HEIGHT, MEMF_CHIP);
  fire = MemAlloc(WIDTH * HEIGHT * 2, MEMF_CHIP);

  EnableDMA(DMAF_BLITTER);
  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  SetupPlayfield(MODE_HAM, 7, X(0), Y(0), WIDTH * 4 + 2, HEIGHT * 4);

  custom->bpldat[4] = 0x7777; // rgbb: 0111
  custom->bpldat[5] = 0xcccc; // rgbb: 1100

  cp = MakeCopperList();
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  SetIntVector(INTB_BLIT, (IntHandlerT)ChunkyToPlanar, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);

  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);

  DeleteCopList(cp);

  MemFree(chunky[0]);
  MemFree(chunky[1]);
  MemFree(fire);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static inline int fastrand(void) {
  static int m[2] = { 0x3E50B28C, 0xD461A7F9 };

  int a, b;

  // https://www.atari-forum.com/viewtopic.php?p=188000#p188000
  asm volatile("move.l (%2)+,%0\n"
               "move.l (%2),%1\n"
               "swap   %1\n"
               "add.l  %0,(%2)\n"
               "add.l  %1,-(%2)\n"
               : "=d" (a), "=d" (b)
               : "a" (m));
  
  return a;
}

static void RandomizeBottom(void) {
  int r;
  short i;
  short *bufPtr;

  /*
   * Bottom two lines are randomized every frame and not displayed.
   * This loop takes 34 raster lines (on average) to execute.
   */
  bufPtr = &(fire[WIDTH * HEIGHT - 1]);
  for (i = 1; i <= WIDTH * 2; i += 5) {
    r = fastrand();
    *bufPtr-- = (r & 0x3F) * 4;
    r >>= 6;
    *bufPtr-- = (r & 0x3F) * 4;
    r >>= 6; 
    *bufPtr-- = (r & 0x3F) * 4;
    r >>= 6; 
    *bufPtr-- = (r & 0x3F) * 4;
    r >>= 6; 
    *bufPtr-- = (r & 0x3F) * 4;
  }
}

static void MainLoop(void) {
  /*
   * Right now this effect takes 788-968-976 (min-avg-max)
   * raster lines to render.
   */
  short i;

  /*
   *   A
   * B C D
   *   E
   */
  register uint16_t *chunkyPtr asm("a0") = (uint16_t *)chunky[active];
  register uint32_t *Aptr      asm("a1") = (uint32_t *)fire;
  register uint32_t *Bptr      asm("a2") = (uint32_t *)&fire[WIDTH - 1];
  register uint32_t *Cptr      asm("a3") = (uint32_t *)&fire[WIDTH];
  register uint32_t *Dptr      asm("a4") = (uint32_t *)&fire[WIDTH + 1];
  register uint32_t *Eptr      asm("a6") = (uint32_t *)&fire[WIDTH * 2];
  register uint32_t *dt = dualtab;

  for (i = 0; i < (WIDTH * HEIGHT - 2 * WIDTH) / 8; ++i) {
    uint32_t vl, hi, lo;

#define FIREITER()                                                             \
    vl = (*Eptr++) + (*Bptr++) + (*Dptr++) + (*Cptr++);                        \
                                                                               \
    asm volatile(                                                              \
      "movel (%3,%2:w),%1\n"                                                   \
      "swap  %2\n"                                                             \
      "movel (%3,%2:w),%0\n"                                                   \
      : "=r" (hi), "=r" (lo)                                                   \
      : "d" (vl), "a" (dt));                                                   \
                                                                               \
    *chunkyPtr++ = hi;                                                         \
    *chunkyPtr++ = lo;                                                         \
                                                                               \
    asm volatile(                                                              \
      "swap   %1\n"                                                            \
      "move.w %1,%0\n"                                                         \
      : "+d" (hi), "+d" (lo));                                                 \
                                                                               \
    *Aptr++ = hi;

    FIREITER();
    FIREITER();
    FIREITER();
    FIREITER();
  }
}

PROFILE(FireRGB);

static void Render(void) {
  ProfilerStart(FireRGB);
  {
    RandomizeBottom();
    MainLoop();
  }
  ProfilerStop(FireRGB);

  c2p.phase = 0;
  c2p.chunky = chunky[active];
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();

  active ^= 1;
}

EFFECT(FireRGB, NULL, NULL, Init, Kill, Render, NULL);
