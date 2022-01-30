#include "effect.h"
#include "blitter.h"
#include "copper.h"
#include "interrupt.h"
#include "memory.h"
#include "fx.h"
#include "pixmap.h"

#define WIDTH 160
#define HEIGHT 100
#define DEPTH 4
#define FULLPIXEL 1

static u_short *textureHi, *textureLo;
static BitmapT *screen[2];
static u_short active = 0;
static CopListT *cp;
static CopInsT *bplptr[DEPTH];

#include "data/rork-128.c"

/* [0 0 0 0 a0 a1 a2 a3] => [a0 a1 0 0 a2 a3 0 0] x 2 */
static u_short PixelHi[16] = {
  0x0000, 0x0404, 0x0808, 0x0c0c, 0x4040, 0x4444, 0x4848, 0x4c4c,
  0x8080, 0x8484, 0x8888, 0x8c8c, 0xc0c0, 0xc4c4, 0xc8c8, 0xcccc,
};

/* [0 0 0 0 b0 b1 b2 b3] => [ 0 0 b0 b1 0 0 b2 b3] x 2 */
static u_short PixelLo[16] = {
  0x0000, 0x0101, 0x0202, 0x0303, 0x1010, 0x1111, 0x1212, 0x1313, 
  0x2020, 0x2121, 0x2222, 0x2323, 0x3030, 0x3131, 0x3232, 0x3333, 
};

static void PixmapToTexture(const PixmapT *image,
                            u_short *imageHi, u_short *imageLo) 
{
  u_char *data = image->pixels;
  short n = image->width * image->height;
  /*
   * Since texturing loop may iterate over whole texture (offset = 0...16383),
   * and starting point may be set up at any position (start = 0...16383),
   * we need the texture to be repeated twice in memory (offset + start!).
   */
  u_short *hi0 = imageHi;
  u_short *hi1 = imageHi + n;
  u_short *lo0 = imageLo;
  u_short *lo1 = imageLo + n;

  while (--n >= 0) {
    int c = *data++;
    u_short hi = PixelHi[c];
    u_short lo = PixelLo[c];
    *hi0++ = hi;
    *hi1++ = hi;
    *lo0++ = lo;
    *lo1++ = lo;
  }
}

static struct {
  short phase;
  void **bpl;
} c2p = { 256, NULL };

#define BLTSIZE ((WIDTH / 2) * HEIGHT) /* 8000 bytes */

/* If you think you can speed it up (I doubt it) please first look into
 * `c2p_2x1_4bpl_mangled_fast_blitter.py` in `prototypes/c2p`. */

static void ChunkyToPlanar(void) {
  register void **bpl asm("a0") = c2p.bpl;

  /*
   * Our chunky buffer of size (WIDTH/2, HEIGHT/2) is stored in bpl[0].
   * Each 32-bit long word of chunky buffer contains eight 4-bit pixels
   * [a b c d e f g h] in scrambled format described below.
   * Please note that a_i is the i-th least significant bit of a.
   *
   * [a0 a1 b0 b1 a2 a3 b2 b3 e0 e1 f0 f1 e2 e3 f2 f3
   *  c0 c1 d0 d1 c2 c3 d2 d3 g0 g1 h0 h1 g2 g3 h2 h3]
   *
   * So not only pixels in the texture must be scrambled, but also consecutive
   * bytes of input buffer i.e.: [a b] [e f] [c d] [g h] (see `gen-uvmap.py`).
   *
   * Chunky to planar is divided into two major steps:
   * 
   * Swap 4x2: in two consecutive 16-bit words swap diagonally two bits,
   *           i.e. [b0 b1] <-> [c0 c1], [b2 b3] <-> [c2 c3].
   * Expand 2x1: [x0 x1 ...] is translated into [x0 x0 ...] and [x1 x1 ...]
   *             and copied to corresponding bitplanes, this step effectively
   *             stretches pixels to 2x1.
   *
   * Line doubling is performed using copper. Rendered bitmap will have size
   * (WIDTH, HEIGHT/2, DEPTH) and will be placed in bpl[2] and bpl[3].
   */

  ClearIRQ(INTF_BLIT);

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
      custom->bltcon1 = BSHIFT(4);

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
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(4);
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
      custom->bltdpt = bpl[3];

#if FULLPIXEL
      /* (a & 0xAAAA) | ((b >> 1) & ~0xAAAA) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom->bltcon1 = BSHIFT(1);
#else
      /* (a & 0xAAAA) */
      custom->bltcon0 = (SRCA | DEST) | (ABC | ANBC);
      custom->bltcon1 = 0;
#endif
      /* overall size: BLTSIZE bytes */
      custom->bltsize = 4 | ((BLTSIZE / 8) << 6);
      break;

    case 5:
      custom->bltapt = bpl[1] + BLTSIZE - 2;
      custom->bltbpt = bpl[1] + BLTSIZE - 2;
      custom->bltdpt = bpl[2] + BLTSIZE - 2;
      custom->bltcdat = 0xAAAA;

#if FULLPIXEL
      /* ((a << 1) & 0xAAAA) | (b & ~0xAAAA) */
      custom->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(1);
      custom->bltcon1 = BLITREVERSE;
#else
      /* (a & ~0xAAAA) */
      custom->bltcon0 = (SRCA | DEST) | (ABNC | ANBNC);
      custom->bltcon1 = BLITREVERSE;
#endif
      /* overall size: BLTSIZE bytes */
      custom->bltsize = 4 | ((BLTSIZE / 8) << 6);
      break;

    case 6:
      CopInsSet32(bplptr[0], bpl[2]);
      CopInsSet32(bplptr[1], bpl[3]);
      CopInsSet32(bplptr[2], bpl[2] + BLTSIZE / 2);
      CopInsSet32(bplptr[3], bpl[3] + BLTSIZE / 2);
      break;

    default:
      break;
  }

  c2p.phase++;
}

static void MakeCopperList(CopListT *cp) {
  short i;

  CopInit(cp);
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
  }
  CopEnd(cp);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH);
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH);

  textureHi = MemAlloc(texture.width * texture.height * 4, MEMF_PUBLIC);
  textureLo = MemAlloc(texture.width * texture.height * 4, MEMF_PUBLIC);
  PixmapToTexture(&texture, textureHi, textureLo);

  EnableDMA(DMAF_BLITTER);

  BitmapClear(screen[0]);
  BitmapClear(screen[1]);

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(28), WIDTH * 2, HEIGHT * 2);

  cp = NewCopList(HEIGHT * 2 * (4 - FULLPIXEL) + 50);
  MakeCopperList(cp);
  CopListActivate(cp);

  EnableDMA(DMAF_RASTER);

  SetIntVector(BLIT, (IntHandlerT)ChunkyToPlanar, NULL);
  EnableINT(INTF_BLIT);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER);

  DisableINT(INTF_BLIT);
  ResetIntVector(BLIT);

  DeleteCopList(cp);
  MemFree(textureHi);
  MemFree(textureLo);

  DeleteBitmap(screen[0]);
  DeleteBitmap(screen[1]);
}

void GenDrawSpan(short du asm("d2"), short dv asm("d3"));

void RenderRotator(u_short *chunky asm("a0"),
                   u_short *txtHi asm("a1"),
                   u_short *txtLo asm("a2"),
                   short U asm("d0"), short V asm("d1"),
                   short dU asm("d2"), short dV asm("d3"));

PROFILE(Rotator);

/*
 * Rotator is controlled by parameters that describe rectangle inscribed
 * into a circle. p/q/r points represent top-left / bottom-left / top-right
 * corners of screen (i.e. the rectangle) mapped into texture space. 
 * With radius/alfa/beta it's easy to manipulate the rectange size
 * and ratio between length of sides. radius/alfa/beta uniquely describe
 * position and length of one side, hence other sides can be easily calculated.
 */
static void Rotator(void) {
  short radius = 128 + (COS(frameCount * 17) >> 7);
  short alfa = frameCount * 11;
  short beta = alfa + SIN_HALF_PI + (SIN(frameCount * 13) >> 3);

  int px = mul16(SIN(alfa), radius);
  int py = mul16(COS(alfa), radius);

  {
    int qx = mul16(SIN(beta), radius);
    int qy = mul16(COS(beta), radius);
    short du = div16(qx - px, WIDTH) >> 4;
    short dv = div16(qy - py, HEIGHT) >> 4;

    GenDrawSpan(du, dv);
  }

  {
    int rx = mul16(SIN(beta + SIN_PI), radius);
    int ry = mul16(COS(beta + SIN_PI), radius);

    short dU = div16(rx - px, WIDTH) >> 4;
    short dV = div16(ry - py, HEIGHT) >> 4;
    short U = (px >> 4) & 0x7fff;
    short V = (py >> 4) & 0x7fff;

    RenderRotator(screen[active]->planes[0], textureHi, textureLo,
                  U, V, dU, dV);
  }
}

static void Render(void) {
  /* screen's bitplane #0 is used as a chunky buffer */
  ProfilerStart(Rotator);
  Rotator();
  ProfilerStop(Rotator);

  c2p.phase = 0;
  c2p.bpl = screen[active]->planes;
  ChunkyToPlanar();
  active ^= 1;
}

EFFECT(rotator, NULL, NULL, Init, Kill, Render);
