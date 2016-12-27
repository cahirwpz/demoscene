#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "color.h"
#include "interrupts.h"
#include "memory.h"
#include "io.h"
#include "png.h"
#include "fx.h"

STRPTR __cwdpath = "data";

#define WIDTH 80
#define HEIGHT 64
#define DEPTH 4

static BitmapT *screen[2];
static UWORD active = 0;
static UWORD *lightmap;
static UWORD *shademap;
static UWORD *colormap;
static UWORD *bumpmap;
static UWORD *chunky[2];
static CopListT *cp;
static CopInsT *bplptr[DEPTH];

static UWORD table[16] = {
  1 << 0, 1 << 0, 1 << 4, 1 << 8,
  1 << 1, 1 << 1, 1 << 5, 1 << 9,
  1 << 2, 1 << 2, 1 << 6, 1 << 10,
  1 << 3, 1 << 3, 1 << 7, 1 << 11
};

static void DataScramble(UWORD *data, WORD n) {
  while (--n >= 0) {
    UWORD c = *data;
    UWORD d = 0;

    /* [-- -- -- -- 11 10  9  8  7  6  5  4  3  2  1  0] */
    /* [-- -- -- -- r0 r1 r2 r3 g0 g1 g2 g3 b0 b1 b2 b3] */
    /* [11  7  3  3 10  6  2  2  9  5  1  1  8  4  0  0] */
    /* [r0 g0 b0 b0 r1 g1 b1 b1 r2 g2 b2 b2 r3 g3 b3 b3] */

    {
      UWORD *mask = table;
      WORD n = 16;
      WORD j = 1;

      while (--n >= 0) {
        if (c & *mask++)
          d |= j;
        j += j;
      }
    }

    *data++ = d;
  }
}

static void Load() {
  {
    PixmapT *image = LoadPNG("light.png", PM_GRAY8, MEMF_PUBLIC);

    lightmap = MemAlloc(65536, MEMF_PUBLIC);
    {
      UBYTE *src = image->pixels;
      UWORD *dst = lightmap;
      WORD n = 16384;

      while (--n >= 0)
        *dst++ = ((*src++) >> 2) & 0x3e;
    }

    memcpy(lightmap + 16384, lightmap, 32768);

    DeletePixmap(image);
  }

  {
    PixmapT *image = LoadPNG("bumpmap.png", PM_CMAP8, MEMF_PUBLIC);

    colormap = MemAlloc(WIDTH * HEIGHT * sizeof(UWORD), MEMF_PUBLIC);
    {
      UWORD *dst = colormap;
      UBYTE *src = image->pixels;
      WORD n = WIDTH * HEIGHT;

      while (--n >= 0)
        *dst++ = *src++ << 6;
    }

    shademap = MemAlloc(32 * sizeof(UWORD) * image->palette->count, MEMF_PUBLIC);
    {
      ColorT *c = image->palette->colors;
      UWORD *dst = shademap;
      WORD n = image->palette->count;
      WORD i;

      while (--n >= 0) {
        for (i = 0; i < 16; i++)
          *dst++ = ColorTransitionRGB(0, 0, 0, c->r, c->g, c->b, i);
        for (i = 0; i < 16; i++)
          *dst++ = ColorTransitionRGB(c->r, c->g, c->b, 255, 255, 255, i);
        c++;
      }
    }
    DataScramble(shademap, image->palette->count * 32);

    DeletePixmap(image);
  }

  bumpmap = LoadFile("bumpmap.bin", MEMF_PUBLIC);
  {
    WORD n = WIDTH * HEIGHT;
    UWORD *src = bumpmap;
    UWORD *dst = bumpmap;

    while (--n >= 0)
      *dst++ = *src++ << 1;
  }
}

static void UnLoad() {
  MemFree(bumpmap);
  MemFree(colormap);
  MemFree(shademap);
  MemFree(lightmap);
}

static struct {
  WORD phase;
  APTR *bpl;
  APTR chunky;
} c2p = { 256, NULL };

#define BPLSIZE ((WIDTH * 4) * HEIGHT / 8) /* 2560 bytes */
#define BLTSIZE ((WIDTH * 4) * HEIGHT / 2) /* 10240 bytes */

static void ChunkyToPlanar() {
  APTR src = c2p.chunky;
  APTR dst = c2p.chunky + BLTSIZE;
  APTR *bpl = c2p.bpl;

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

INTERRUPT(ChunkyToPlanarInterrupt, 0, ChunkyToPlanar);

static struct Interrupt *oldBlitInt;

static void MakeCopperList(CopListT *cp) {
  WORD i;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_HAM, 7, X(0), Y(0), WIDTH * 4 + 2, HEIGHT * 4);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  CopMove16(cp, bpldat[4], 0x7777); // rgbb: 0111
  CopMove16(cp, bpldat[5], 0xcccc); // rgbb: 1100
  CopLoadColor(cp, 0, 15, 0);
  for (i = 0; i < HEIGHT * 4; i++) {
    CopWait(cp, Y(i), 0);
    /* Line quadrupling. */
    CopMove16(cp, bpl1mod, ((i & 3) != 3) ? -40 : 0);
    CopMove16(cp, bpl2mod, ((i & 3) != 3) ? -40 : 0);
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0022 : 0x0000);
  }
  CopEnd(cp);
}

static void Init() {
  screen[0] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH);
  screen[1] = NewBitmap(WIDTH * 4, HEIGHT, DEPTH);

  screen[0]->flags |= BM_HAM;
  screen[1]->flags |= BM_HAM;

  chunky[0] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP);
  chunky[1] = MemAlloc((WIDTH * 4) * HEIGHT, MEMF_CHIP);

  custom->dmacon = DMAF_SETCLR | DMAF_BLITTER;

  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  cp = NewCopList(1200);
  MakeCopperList(cp);
  CopListActivate(cp);

  custom->dmacon = DMAF_SETCLR | DMAF_RASTER;

  oldBlitInt = SetIntVector(INTB_BLIT, &ChunkyToPlanarInterrupt);
  custom->intena = INTF_SETCLR | INTF_BLIT;
}

static void Kill() {
  custom->dmacon = DMAF_COPPER | DMAF_RASTER;

  custom->intena = INTF_BLIT;
  SetIntVector(INTB_BLIT, oldBlitInt);

  DeleteCopList(cp);

  MemFree(chunky[0]);
  MemFree(chunky[1]);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static __regargs void BumpMapRender(APTR lmap) {
  APTR smap = shademap;
  UWORD *cmap = colormap;
  UWORD *bmap = bumpmap;
  UWORD *dst = chunky[active];
  WORD n = HEIGHT * WIDTH / 2;

  while (--n >= 0) {
    {
      WORD s = *(WORD *)(lmap + (*bmap++)) | *cmap++;
      *dst++ = *(UWORD *)(smap + s);
    }
    {
      WORD s = *(WORD *)(lmap + (*bmap++)) | *cmap++;
      *dst++ = *(UWORD *)(smap + s);
    }
  }
}

static void Render() {
  LONG lines = ReadLineCounter();
  {
    WORD xo = normfx(SIN(frameCount * 16) * HEIGHT / 2) + 24;
    WORD yo = normfx(COS(frameCount * 16) * HEIGHT / 2) + 32;
    BumpMapRender(&lightmap[(yo * 128 + xo) & 16383]);
  }
  Log("bumpmap: %ld\n", ReadLineCounter() - lines);

  c2p.phase = 0;
  c2p.chunky = chunky[active];
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EffectT Effect = { Load, UnLoad, Init, Kill, Render };
