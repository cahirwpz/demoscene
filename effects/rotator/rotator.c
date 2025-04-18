#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <system/interrupt.h>
#include <system/memory.h>

#define WIDTH 160
#define HEIGHT 100
#define DEPTH 4

static __code BitmapT *screen[2];
static __code CopInsPairT *bplptr[2];
static __code CopListT *cp[2];
static __code short active = 0;
static __code volatile short c2p_phase;
static __code short c2p_active;
static __code void **c2p_bpl;
static __code u_short *textureHi, *textureLo;

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

static void PixmapToTexture(u_short *imageHi, u_short *imageLo) {
  u_char *data = texture_pixels;
  short n = texture_width * texture_height;
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

#define BLTSIZE ((WIDTH / 2) * HEIGHT) /* 8000 bytes */

/* If you think you can speed it up (I doubt it) please first look into
 * `c2p_2x1_4bpl_mangled_fast_blitter.py` in `prototypes/c2p`. */

#define C2P_LAST 7

static void ChunkyToPlanar(CustomPtrT custom_) {
  register void **bpl = c2p_bpl;

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

  custom_->intreq_ = INTF_BLIT;

  switch (c2p_phase++) {
    case 0:
      /* Initialize chunky to planar. */
      custom_->bltamod = 2;
      custom_->bltbmod = 2;
      custom_->bltdmod = 0;
      custom_->bltcdat = 0xF0F0;
      custom_->bltafwm = -1;
      custom_->bltalwm = -1;

      /* Swap 4x2, pass 1, high-bits. */
      custom_->bltapt = bpl[0];
      custom_->bltbpt = bpl[0] + 2;
      custom_->bltdpt = bpl[1] + BLTSIZE / 2;

      /* (a & 0xF0F0) | ((b >> 4) & ~0xF0F0) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom_->bltcon1 = BSHIFT(4);

    case 1:
      /* overall size: BLTSIZE / 2 bytes */
      custom_->bltsize = 1 | ((BLTSIZE / 8) << 6);
      break;

    case 2:
      /* Swap 4x2, pass 2, low-bits. */
      custom_->bltapt = bpl[1] - 4;
      custom_->bltbpt = bpl[1] - 2;
      custom_->bltdpt = bpl[1] + BLTSIZE / 2 - 2;

      /* ((a << 4) & 0xF0F0) | (b & ~0xF0F0) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(4);
      custom_->bltcon1 = BLITREVERSE;

    case 3:
      /* overall size: BLTSIZE / 2 bytes */
      custom_->bltsize = 1 | ((BLTSIZE / 8) << 6);
      break;

    case 4:
      custom_->bltamod = 0;
      custom_->bltbmod = 0;
      custom_->bltdmod = 0;
      custom_->bltcdat = 0xAAAA;

      custom_->bltapt = bpl[1];
      custom_->bltbpt = bpl[1];
      custom_->bltdpt = bpl[3];

      /* (a & 0xAAAA) | ((b >> 1) & ~0xAAAA) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC);
      custom_->bltcon1 = BSHIFT(1);
      /* overall size: BLTSIZE bytes */
      custom_->bltsize = 4 | ((BLTSIZE / 8) << 6);
      break;

    case 5:
      custom_->bltapt = bpl[1] + BLTSIZE - 2;
      custom_->bltbpt = bpl[1] + BLTSIZE - 2;
      custom_->bltdpt = bpl[2] + BLTSIZE - 2;
      custom_->bltcdat = 0xAAAA;

      /* ((a << 1) & 0xAAAA) | (b & ~0xAAAA) */
      custom_->bltcon0 = (SRCA | SRCB | DEST) | (ABC | ABNC | ANBC | NABNC) | ASHIFT(1);
      custom_->bltcon1 = BLITREVERSE;
      /* overall size: BLTSIZE bytes */
      custom_->bltsize = 4 | ((BLTSIZE / 8) << 6);
      break;

    case 6:
      {
        CopInsPairT *ins = bplptr[c2p_active];
        CopInsSet32(ins++, bpl[2]);
        CopInsSet32(ins++, bpl[3]);
        CopInsSet32(ins++, bpl[2] + BLTSIZE / 2);
        CopInsSet32(ins++, bpl[3] + BLTSIZE / 2);
        CopListRun(cp[c2p_active]);
      }
      break;

    default:
      break;
  }
}

static void ChunkyToPlanarStart(void) {
  c2p_active = active;
  c2p_phase = 0;
  c2p_bpl = screen[active]->planes;
  ChunkyToPlanar(custom);
  active ^= 1;
}

static void ChunkyToPlanarWait(void) {
  while (BlitterBusy() || c2p_phase < C2P_LAST)
    continue;
}

static CopListT *MakeCopperList(short active) {
  CopListT *cp = NewCopList(HEIGHT * 2 * 3 + 50);
  short i;

  bplptr[active] = CopSetupBitplanes(cp, screen[active], DEPTH);
  for (i = 0; i < HEIGHT * 2; i++) {
    CopWaitSafe(cp, Y(i + 28), 0);
    /* Line doubling. */
    CopMove16(cp, bpl1mod, (i & 1) ? 0 : -40);
    CopMove16(cp, bpl2mod, (i & 1) ? 0 : -40);
  }
  return CopListFinish(cp);
}

static void Load(void) {
  textureHi = MemAlloc(texture_width * texture_height * 4, MEMF_PUBLIC);
  textureLo = MemAlloc(texture_width * texture_height * 4, MEMF_PUBLIC);
  PixmapToTexture(textureHi, textureLo);
}

static void UnLoad(void) {
  MemFree(textureHi);
  MemFree(textureLo);
}

static void Init(void) {
  screen[0] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH, 0);
  screen[1] = NewBitmap(WIDTH * 2, HEIGHT * 2, DEPTH, 0);

  EnableDMA(DMAF_BLITTER | DMAF_BLITHOG);
  BitmapClear(screen[0]);
  BitmapClear(screen[1]);
  WaitBlitter();

  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(28), WIDTH * 2, HEIGHT * 2);
  LoadColors(texture_colors, 0);

  cp[0] = MakeCopperList(0);
  cp[1] = MakeCopperList(1);
  CopListActivate(cp[1]);

  SetIntVector(INTB_BLIT, (IntHandlerT)ChunkyToPlanar, (void *)custom);
  ClearIRQ(INTF_BLIT);
  EnableINT(INTF_BLIT);

  active = 0;
  ChunkyToPlanarStart();
  ChunkyToPlanarWait();

  CopListActivate(cp[0]);
  EnableDMA(DMAF_RASTER);
}

static void Kill(void) {
  CopperStop();

  DisableINT(INTF_BLIT);
  ResetIntVector(INTB_BLIT);

  BlitterStop();
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);

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
  short radius = 128 + 32 + (COS(frameCount * 17) >> 7);
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

  ChunkyToPlanarWait();
  TaskWaitVBlank();
  ChunkyToPlanarStart();
}

EFFECT(Rotator, Load, UnLoad, Init, Kill, Render, NULL);
