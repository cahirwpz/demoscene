#include <effect.h>
#include <blitter.h>
#include <copper.h>
#include <fx.h>
#include <pixmap.h>
#include <sprite.h>
#include <system/memory.h>

#define WIDTH   144
#define HEIGHT  255
#define DEPTH   5
#define STARTX  96

static CopListT *cp[2];
static CopInsPairT *bplptr[2];
static CopInsT *bplmod[2][HEIGHT];
static CopInsT *colors[2][HEIGHT];
static short active = 0;

#include "data/twister-gradient.c"
#include "data/twister-texture.c"
#include "data/twister-left.c"
#include "data/twister-right.c"
#include "data/twister.c"

static CopListT *MakeCopperList(short n) {
  CopListT *cp = NewCopList(100 + HEIGHT * 5 + (31 * HEIGHT / 3));
  CopInsPairT *sprptr = CopSetupSprites(cp);
  short *pixels = texture.pixels;
  short i, j, k;

  bplptr[n] = CopSetupBitplanes(cp, &twister, DEPTH);
  
  CopMove16(cp, dmacon, DMAF_SETCLR|DMAF_RASTER);
  CopSetColor(cp, 0, gradient_colors[0]);

  for (i = 0, k = 0; i < HEIGHT; i++) {
    CopWaitSafe(cp, Y(i), 0);
    bplmod[n][i] = CopMove16(cp, bpl1mod, -32);
    CopMove16(cp, bpl2mod, -32);
    CopMove16(cp, bpldat[0], 0);

    CopSetColor(cp, 0, gradient_colors[i]);

    if ((i % 3) == 0) {
      colors[n][k++] = CopSetColor(cp, 1, *pixels++);
      for (j = 2; j < 32; j++)
        CopSetColor(cp, j, *pixels++);
    }
  }

  CopInsSetSprite(&sprptr[4], &left[0]);
  CopInsSetSprite(&sprptr[5], &left[1]);
  CopInsSetSprite(&sprptr[6], &right[0]);
  CopInsSetSprite(&sprptr[7], &right[1]);

  return CopListFinish(cp);
}

static void Init(void) {
  EnableDMA(DMAF_BLITTER);

  SetupPlayfield(MODE_LORES, DEPTH, X(STARTX), Y(0), WIDTH, HEIGHT);
  custom->diwstrt = 0x2c81;
  custom->diwstop = 0x2bc1;

  cp[0] = MakeCopperList(0);
  cp[1] = MakeCopperList(1);

  SpriteUpdatePos(&left[0], X(0), Y(0));
  SpriteUpdatePos(&left[1], X(16), Y(0));
  SpriteUpdatePos(&right[0], X(320 - 32), Y(0));
  SpriteUpdatePos(&right[1], X(320 - 16), Y(0));

  CopListActivate(cp[1]);
  EnableDMA(DMAF_RASTER | DMAF_SPRITE);
}

static void Kill(void) {
  DeleteCopList(cp[0]);
  DeleteCopList(cp[1]);
}

static inline short rotate(short f) {
  return (COS(f) >> 3) & 255;
}

static void SetupLines(short f) {
  short y0 = rotate(f);

  /* first line */
  {
    int y = (short)twister.bytesPerRow * y0;
    void *const *planes = twister.planes;
    CopInsPairT *_bplptr = bplptr[active];
    short n = DEPTH;

    while (--n >= 0)
      CopInsSet32(_bplptr++, (*planes++) + y);
  }

  /* consecutive lines */
  {
    short **modptr = (short **)bplmod[active];
    short y1, i, m;

    for (i = 1; i < HEIGHT; i++, y0 = y1, f += 2) {
      short *mod = *modptr++;

      y1 = rotate(f);
      m = ((y1 - y0) - 1) * (WIDTH / 8);

      mod[1] = m;
      mod[3] = m;
    }
  }
}

static void SetupTexture(CopInsT **colors, short y) {
  short *pixels = texture.pixels;
  short height = texture.height;
  short width = texture.width;
  short n = height;

  y %= height;

  pixels += y * width;

  while (--n >= 0) {
    short *ins = (short *)(*colors++) + 1;

    ins[0] = *pixels++;
    ins[2] = *pixels++;
    ins[4] = *pixels++;
    ins[6] = *pixels++;
    ins[8] = *pixels++;
    ins[10] = *pixels++;
    ins[12] = *pixels++;
    ins[14] = *pixels++;
    ins[16] = *pixels++;
    ins[18] = *pixels++;
    ins[20] = *pixels++;
    ins[22] = *pixels++;
    ins[24] = *pixels++;
    ins[26] = *pixels++;
    ins[28] = *pixels++;
    ins[30] = *pixels++;
    ins[32] = *pixels++;
    ins[34] = *pixels++;
    ins[36] = *pixels++;
    ins[38] = *pixels++;
    ins[40] = *pixels++;
    ins[42] = *pixels++;
    ins[44] = *pixels++;
    ins[46] = *pixels++;
    ins[48] = *pixels++;
    ins[50] = *pixels++;
    ins[52] = *pixels++;
    ins[54] = *pixels++;
    ins[56] = *pixels++;
    ins[58] = *pixels++;
    ins[60] = *pixels++;

    y++;

    if (y >= height) {
      pixels = texture.pixels;
      y = 0;
    }
  }
}

PROFILE(TwisterRGB);

static void Render(void) {
  ProfilerStart(TwisterRGB);
  {
    SetupLines(frameCount * 16);
    SetupTexture(colors[active], frameCount);
  }
  ProfilerStop(TwisterRGB);

  CopListRun(cp[active]);
  TaskWaitVBlank();
  active ^= 1;
}

EFFECT(TwisterRGB, NULL, NULL, Init, Kill, Render, NULL);
