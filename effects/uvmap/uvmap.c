#include "startup.h"
#include "blitter.h"
#include "coplist.h"
#include "interrupts.h"
#include "memory.h"
#include "pixmap.h"

#define WIDTH 160
#define HEIGHT 100
#define DEPTH 4
#define FULLPIXEL 1

static PixmapT *textureHi, *textureLo;
static BitmapT *screen[2];
static u_short active = 0;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];

#include "data/texture-16-1.c"
#include "data/gradient.c"
#include "data/uvmap.c"

#define UVMapRenderSize (WIDTH * HEIGHT / 2 * 10 + 2)
void (*UVMapRender)(u_char *chunky asm("a0"),
                    u_char *textureHi asm("a1"),
                    u_char *textureLo asm("a2"));

static __regargs void PixmapToTexture(const PixmapT *image,
                                      PixmapT *imageHi, PixmapT *imageLo) 
{
  u_int *data = image->pixels;
  int size = image->width * image->height;
  /* Extra halves for cheap texture motion. */
  u_int *hi0 = imageHi->pixels;
  u_int *hi1 = imageHi->pixels + size;
  u_int *lo0 = imageLo->pixels;
  u_int *lo1 = imageLo->pixels + size;
  short n = size / 4;
  register u_int m1 asm("d6") = 0x0c0c0c0c;
  register u_int m2 asm("d7") = 0x03030303;

  while (--n >= 0) {
    u_int c = *data++;
    /* [0 0 0 0 a0 a1 a2 a3] => [a0 a1 0 0 a2 a3 0 0] */
    u_int hi = ((c & m1) << 4) | ((c & m2) << 2);
    /* [0 0 0 0 b0 b1 b2 b3] => [ 0 0 b0 b1 0 0 b2 b3] */
    u_int lo = ((c & m1) << 2) | (c & m2);
    *hi0++ = hi;
    *hi1++ = hi;
    *lo0++ = lo;
    *lo1++ = lo;
  }
}

static void MakeUVMapRenderCode(void) {
  u_short *code = (void *)UVMapRender;
  u_short *data = uvmap;
  short n = WIDTH * HEIGHT / 2;

  /* The map is pre-scrambled to avoid one c2p pass: [a B C d] => [a C B d] */
  while (n--) {
    *code++ = 0x1029;  /* 1029 xxxx | move.b xxxx(a1),d0 */
    *code++ = *data++;
    *code++ = 0x802a;  /* 802a yyyy | or.b   yyyy(a2),d0 */
    *code++ = *data++;
    *code++ = 0x10c0;  /* 10c0      | move.b d0,(a0)+    */
  }

  *code++ = 0x4e75; /* rts */
}

static struct {
  short phase;
  void **bpl;
} c2p = { 256, NULL };

#define BPLSIZE ((WIDTH * 2) * (HEIGHT * 2) / 8) /* 8000 bytes */
#define BLTSIZE ((WIDTH / 2) * HEIGHT)           /* 8000 bytes */

static void ChunkyToPlanar(void) {
  register void **bpl asm("a0") = c2p.bpl;

  /*
   * Our chunky buffer of size (WIDTH/2, HEIGHT/2) is in bpl[0]. Each 16-bit
   * word of chunky buffer stores four 4-bit pixels [a B c D] in scrambled
   * format described below:
   * 
   * [a3 a2 C3 C2 a1 a0 C1 C0 b3 b2 D3 D2 b1 b0 D1 D0]
   *
   * Using blitter we stretch pixels to 2x1. Line doubling is performed using
   * copper. Rendered bitmap will have size (WIDTH, HEIGHT/2, DEPTH) and will
   * be placed in bpl[2] and bpl[3].
   */

  switch (c2p.phase) {
    case 0:
      /* Initialize chunky to planar. */
      custom->bltamod = 2;
      custom->bltbmod = 2;
      custom->bltdmod = 0;
      custom->bltcdat = 0xF0F0;
      custom->bltafwm = -1;
      custom->bltalwm = -1;

      /* Swap 4x2, pass 1, high-bits. */
      custom->bltapt = bpl[0];
      custom->bltbpt = bpl[0] + 2;
      custom->bltdpt = bpl[1] + BLTSIZE / 2;

      /* (a & 0xF0F0) | ((b >> 4) & ~0xF0F0) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom->bltcon1 = 4 << BSHIFTSHIFT;

    case 1:
      /* overall size: BLTSIZE / 2 bytes */
      custom->bltsize = 1 | ((BLTSIZE / 8) << 6);
      break;

    case 2:
      /* Swap 4x2, pass 2, low-bits. */
      custom->bltapt = bpl[1] - 4;
      custom->bltbpt = bpl[1] - 2;
      custom->bltdpt = bpl[1] + BLTSIZE / 2 - 2;

      /* ((a << 4) & 0xF0F0) | (b & ~0xF0F0) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (4 << ASHIFTSHIFT);
      custom->bltcon1 = BLITREVERSE;

    case 3:
      /* overall size: BLTSIZE / 2 bytes */
      custom->bltsize = 1 | ((BLTSIZE / 8) << 6);
      break;

    case 4:
      custom->bltamod = 0;
      custom->bltbmod = 0;
      custom->bltdmod = 0;
      custom->bltcdat = 0xAAAA;

      custom->bltapt = bpl[1];
      custom->bltbpt = bpl[1];
      custom->bltdpt = bpl[2] + BLTSIZE / 2;

#if FULLPIXEL
      /* (a & 0xAAAA) | ((b >> 1) & ~0xAAAA) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom->bltcon1 = 1 << BSHIFTSHIFT;
#else
      /* (a & 0xAAAA) */
      custom->bltcon0 = (SRCA | DEST) | (ABC | ANBC);
      custom->bltcon1 = 0;
#endif
      custom->bltsize = 2 | ((BLTSIZE / 8) << 6);
      break;

    case 5:
      custom->bltapt = bpl[1] + BLTSIZE / 2;
      custom->bltbpt = bpl[1] + BLTSIZE / 2;
      custom->bltdpt = bpl[3] + BLTSIZE / 2;

      /* overall size: BLTSIZE bytes */
      custom->bltsize = 2 | ((BLTSIZE / 8) << 6);
      break;

    case 6:
      custom->bltapt = bpl[1] + BLTSIZE / 2 - 2;
      custom->bltbpt = bpl[1] + BLTSIZE / 2 - 2;
      custom->bltdpt = bpl[2] + BLTSIZE / 2 - 2;
      custom->bltcdat = 0xAAAA;

#if FULLPIXEL
      /* ((a << 1) & 0xAAAA) | (b & ~0xAAAA) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | (1 << ASHIFTSHIFT);
      custom->bltcon1 = BLITREVERSE;
#else
      /* (a & ~0xAAAA) */
      custom->bltcon0 = (SRCA | DEST) | (ABNC | ANBNC);
      custom->bltcon1 = BLITREVERSE;
#endif
      custom->bltsize = 2 | ((BLTSIZE / 8) << 6);
      break;

    case 7:
      custom->bltapt = bpl[1] + BLTSIZE - 2;
      custom->bltbpt = bpl[1] + BLTSIZE - 2;
      custom->bltdpt = bpl[3] + BLTSIZE / 2 - 2;
      custom->bltsize = 2 | ((BLTSIZE / 8) << 6);
      break;

    case 8:
      CopInsSet32(bplptr[0], bpl[2]);
      CopInsSet32(bplptr[1], bpl[2] + BLTSIZE / 2);
      CopInsSet32(bplptr[2], bpl[3]);
      CopInsSet32(bplptr[3], bpl[3] + BLTSIZE / 2);
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
  short *pixels = gradient.pixels;
  short i, j;

  CopInit(cp);
  CopSetupGfxSimple(cp, MODE_LORES, DEPTH, X(0), Y(28), WIDTH * 2, HEIGHT * 2);
  CopSetupBitplanes(cp, bplptr, screen[active], DEPTH);
  CopLoadPal(cp, &texture_pal, 0);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWaitSafe(cp, Y(i + 28), 0);
    /* Line doubling. */
    CopMove16(cp, bpl1mod, (i & 1) ? 0 : -40);
    CopMove16(cp, bpl2mod, (i & 1) ? 0 : -40);
#if !FULLPIXEL
    /* Alternating shift by one for bitplane data. */
    CopMove16(cp, bplcon1, (i & 1) ? 0x0010 : 0x0021);
#endif
    if (i % 12 == 11)
      for (j = 0; j < 16; j++)
        CopSetColor(cp, j, *pixels++);
  }
  CopEnd(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH);
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH);

  UVMapRender = MemAlloc(UVMapRenderSize, MEMF_PUBLIC);
  MakeUVMapRenderCode();

  textureHi = NewPixmap(texture.width, texture.height * 2,
                        PM_CMAP8, MEMF_PUBLIC);
  textureLo = NewPixmap(texture.width, texture.height * 2,
                        PM_CMAP8, MEMF_PUBLIC);
  PixmapToTexture(&texture, textureHi, textureLo);

  EnableDMA(DMAF_BLITTER);

  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  cp = NewCopList(900 + 256);
  MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  oldBlitInt = SetIntVector(INTB_BLIT, ChunkyToPlanarInterrupt);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DisableINT(INTF_BLIT);
  SetIntVector(INTB_BLIT, oldBlitInt);

  DeleteCopList(cp);
  DeletePixmap(textureHi);
  DeletePixmap(textureLo);
  MemFree(UVMapRender);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

static void Render(void) {
  short offset = (frameCount * 127) & 16383;

  /* screen's bitplane #0 is used as a chunky buffer */
  {
    u_char *txtHi = textureHi->pixels + offset;
    u_char *txtLo = textureLo->pixels + offset;

    // int lines = ReadLineCounter();
    (*UVMapRender)(screen[active]->planes[0], txtHi, txtLo);
    // Log("uvmap: %d\n", ReadLineCounter() - lines);
  }

  c2p.phase = 0;
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EffectT Effect = { NULL, NULL, Init, Kill, Render };
