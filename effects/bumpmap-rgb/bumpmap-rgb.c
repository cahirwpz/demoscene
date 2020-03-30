#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "color.h"
#include "interrupts.h"
#include "memory.h"
#include "pixmap.h"
#include "fx.h"

#define WIDTH 80
#define HEIGHT 64
#define DEPTH 4

static BitmapT *screen[2];
static u_short active = 0;
static u_short *lightmap;
static u_short *shademap;
static u_short *colormap;
static u_short *chunky[2];
static CopListT *cp;
static CopInsT *bplptr[DEPTH];

#include "data/dragon.c"
#include "data/light.c"
#include "data/bumpmap.c"

static u_short bluetab[16] = {
  0x0000, 0x0003, 0x0030, 0x0033, 0x0300, 0x0303, 0x0330, 0x0333,
  0x3000, 0x3003, 0x3030, 0x3033, 0x3300, 0x3303, 0x3330, 0x3333,
};

static u_short greentab[16] = {
  0x0000, 0x0004, 0x0040, 0x0044, 0x0400, 0x0404, 0x0440, 0x0444,
  0x4000, 0x4004, 0x4040, 0x4044, 0x4400, 0x4404, 0x4440, 0x4444,
};

static u_short redtab[16] = {
  0x0000, 0x0008, 0x0080, 0x0088, 0x0800, 0x0808, 0x0880, 0x0888,
  0x8000, 0x8008, 0x8080, 0x8088, 0x8800, 0x8808, 0x8880, 0x8888,
};

static void DataScramble(u_short *data, short n) {
  u_char *in = (u_char *)data;
  u_short *out = data;

  while (--n >= 0) {
    short ri = *in++;
    short gi = *in++;
    short bi = gi;

    /* [-- -- -- -- 11 10  9  8  7  6  5  4  3  2  1  0] */
    /* [-- -- -- -- r0 r1 r2 r3 g0 g1 g2 g3 b0 b1 b2 b3] */
    /* [11  7  3  3 10  6  2  2  9  5  1  1  8  4  0  0] */
    /* [r0 g0 b0 b0 r1 g1 b1 b1 r2 g2 b2 b2 r3 g3 b3 b3] */

    gi >>= 4;
    bi &= 15;

    *out++ = getword(redtab, ri) + getword(greentab, gi) + getword(bluetab, bi);
  }
}

static void Load(void) {
  int lightSize = light_w * light_h;

  lightmap = MemAlloc(lightSize * sizeof(u_short) * 2, MEMF_PUBLIC);
  {
    u_char *src = light;
    u_short *dst0 = lightmap;
    u_short *dst1 = lightmap + lightSize;
    short n = lightSize;

    while (--n >= 0) {
      short v = ((*src++) >> 2) & 0x3e;
      *dst0++ = v;
      *dst1++ = v;
    }
  }

  colormap = MemAlloc(WIDTH * HEIGHT * sizeof(u_short), MEMF_PUBLIC);
  {
    u_char *src = dragon.pixels;
    u_short *dst = colormap;
    short n = WIDTH * HEIGHT;

    while (--n >= 0)
      *dst++ = *src++ << 6;
  }

  shademap = MemAlloc(32 * sizeof(u_short) * dragon_pal.count, MEMF_PUBLIC);
  {
    ColorT *c = dragon_pal.colors;
    u_short *dst = shademap;
    short n = dragon_pal.count;
    short i;

    while (--n >= 0) {
      for (i = 0; i < 16; i++)
        *dst++ = ColorTransitionRGB(0, 0, 0, c->r, c->g, c->b, i);
      for (i = 0; i < 16; i++)
        *dst++ = ColorTransitionRGB(c->r, c->g, c->b, 255, 255, 255, i);
      c++;
    }
  }

  DataScramble(shademap, dragon_pal.count * 32);

  {
    short n = WIDTH * HEIGHT;
    u_short *src = bumpmap;
    u_short *dst = bumpmap;

    while (--n >= 0)
      *dst++ = *src++ << 1;
  }
}

static void UnLoad(void) {
  MemFree(colormap);
  MemFree(shademap);
  MemFree(lightmap);
}

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
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC) | (8 << ASHIFTSHIFT);
      custom->bltcon1 = 0;
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 1:
      custom->color[0] = 0xf00;
      custom->bltsize = 2 | ((BLTSIZE / 16) << 6);
      break;

    case 2:
      /* Swap 8x4, pass 2. */
      custom->bltapt = src + BLTSIZE - 6;
      custom->bltbpt = src + BLTSIZE - 2;
      custom->bltdpt = dst + BLTSIZE - 2;

      /* ((a << 8) & ~0x00FF) | (b & 0x00FF) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABNC | ANBNC | ABC | NABC) | (8 << ASHIFTSHIFT);
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
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ANBC | ABNC | NABNC) | (4 << ASHIFTSHIFT);
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
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABNC | ANBNC | ABC | NABC) | (4 << ASHIFTSHIFT);
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
      custom->color[0] = 0x0f0;
      CopInsSet32(bplptr[0], bpl[3]);
      CopInsSet32(bplptr[1], bpl[2]);
      CopInsSet32(bplptr[2], bpl[1]);
      CopInsSet32(bplptr[3], bpl[0]);
      break;

    default:
      break;
  }

  c2p.phase++;

  custom->intreq = INTF_BLIT;
}

INTERRUPT(ChunkyToPlanarInterrupt, 0, ChunkyToPlanar, NULL);

static struct Interrupt *oldBlitInt;

static void MakeCopperList(CopListT *cp) {
  short i;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_HAM, 7, X(0), Y(0), WIDTH * 4 + 2, HEIGHT * 4);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  CopMove16(cp, bpldat[4], 0x7777); // rgbb: 0111
  CopMove16(cp, bpldat[5], 0xcccc); // rgbb: 1100
  CopLoadColor(cp, 0, 15, 0);
  for (i = 0; i < HEIGHT * 4; i++) {
    CopWaitSafe(cp, Y(i), 0);
    /* Line quadrupling. */
    CopMove16(cp, bpl1mod, ((i & 3) != 3) ? -40 : 0);
    CopMove16(cp, bpl2mod, ((i & 3) != 3) ? -40 : 0);
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0022 : 0x0000);
  }
  CopEnd(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH);

  screen[0]->flags |= BM_HAM;
  screen[1]->flags |= BM_HAM;

  chunky[0] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP);
  chunky[1] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP);

  EnableDMA(DMAF_BLITTER);

  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  cp = NewCopList(1200);
  MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  oldBlitInt = SetIntVector(INTB_BLIT, ChunkyToPlanarInterrupt);
  ClearIRQ(INTF_BLIT);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER);

  DisableINT(INTF_BLIT);
  SetIntVector(INTB_BLIT, oldBlitInt);

  DeleteCopList(cp);

  MemFree(chunky[0]);
  MemFree(chunky[1]);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

#define OPTIMIZED 1

static __regargs void BumpMapRender(void *lmap) {
  void *smap = shademap;
  u_short *cmap = colormap;
  u_short *bmap = bumpmap;
  u_short *dst = chunky[active];
  short n = HEIGHT * WIDTH / 2 - 1;

  do {
#if OPTIMIZED
    asm volatile("movew %1@+,d0\n"
                 "movew %3@(d0:w),d0\n"
                 "orw   %2@+,d0\n"
                 "movew %4@(d0:w),%0@+\n"
                 "movew %1@+,d0\n"
                 "movew %3@(d0:w),d0\n"
                 "orw   %2@+,d0\n"
                 "movew %4@(d0:w),%0@+\n"
                 : "+a" (dst), "+a" (bmap), "+a" (cmap)
                 : "a" (lmap), "a" (smap)
                 : "d0");
#else
    {
      short s = *(short *)(lmap + (*bmap++)) | *cmap++;
      *dst++ = *(u_short *)(smap + s);
    }
    {
      short s = *(short *)(lmap + (*bmap++)) | *cmap++;
      *dst++ = *(u_short *)(smap + s);
    }
#endif
  } while (--n >= 0);
}

static void Render(void) {
  int lines = ReadLineCounter();
  {
    short xo = normfx(SIN(frameCount * 16) * HEIGHT / 2) + 24;
    short yo = normfx(COS(frameCount * 16) * HEIGHT / 2) + 32;
    BumpMapRender(&lightmap[(yo * 128 + xo) & 16383]);
  }
  Log("bumpmap: %d\n", ReadLineCounter() - lines);

  c2p.phase = 0;
  c2p.chunky = chunky[active];
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render, NULL };
