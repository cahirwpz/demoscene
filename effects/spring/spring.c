#include "effect.h"

#include "bitmap.h"
#include "blitter.h"
#include "copper.h"
#include "custom.h"
#include "fx.h"
#include "gfx.h"
#include "sprite.h"
#include "stdio.h"

#define WIDTH 320
#define HEIGHT 256
#define DEPTH 4

#define X_OFFSET 40 

#define AMPL 32
#define Y0 130
#define START_DRAW 48

#include "data/background.c"
#include "data/spiders.c"

static CopListT *cp;
static CopInsT *sprptr[8];

/* Palette and references for spider sprites. */
static PaletteT sprite_pal = {.count = 15,
                              .colors = {0x0b1, 0xb4f, 0x10b, 0x0, 0x04b, 0xb70,
                                         0x09b, 0x0, 0xbb0, 0xb01, 0x0b3, 0x0,
                                         0x04b, 0xb70, 0x09b}};

static SpriteT *sprite[] = {
    &spiders_sprite0, &spiders_sprite1, &spiders_sprite2, &spiders_sprite3,
    &spiders_sprite4, &spiders_sprite5, &spiders_sprite6, &spiders_sprite7};

/* Offsets for diverging spiders movement. */
static short factors[8] = {-32, 128, -96, 64, -160, 0, -224, -64};

/* Draws lines with blitter to create a spring.
 * It takes position of a spider, numbers of points in a spring
 * and distance between each 2 in the line. */
static void DrawSpring(int y, int x, short pts_num, short pts_dist) {
  short x0, x1;
  short y0, y1;
  short temp;
  int len;
  short delta_x, delta_y;
  short i;

  /* Length od the spring. */
  len = y - START_DRAW;

  /* Calculate vertical delta_y between points and horizontal offset from
   * center. */
  delta_y = div16(len, pts_num << 1);
  delta_x = ((pts_dist * pts_dist) - (delta_y * delta_y)) >> 2;
  if (delta_x > 0)
    delta_x = isqrt(delta_x);
  else
    delta_x = 0;

  x0 = x + delta_x;
  x1 = x - delta_x;

  y0 = START_DRAW;
  y1 = y0 + delta_y;

  BlitterLineSetup(&background_bmp, 0, LINE_SOLID);
  BlitterLine(x, y0, x0, y1);

  for (i = 0; i < pts_num - 1; i++) {
    temp = x0;
    x0 = x1;
    x1 = temp;
    y0 = y1;
    y1 = y0 + 2 * delta_y;
    BlitterLine(x1, y0, x0, y1);
  }
  y0 = y1;
  y1 = y0 + delta_y;
  BlitterLine(x0, y0, x, y1);
  BlitterLine(x, y1, x, y);
}

static void MakeCopperList(CopListT *cp) {
  CopInit(cp);
  CopSetupSprites(cp, sprptr);
  ITER(i, 0, 8, CopInsSet32(sprptr[i], sprite[i]));
  CopSetupBitplanes(cp, NULL, &background_bmp, DEPTH);

  CopMove16(cp, bplcon2, 0x20);
  CopLoadPal(cp, &back_pal, 0);
  CopLoadPal(cp, &sprite_pal, 17);
  /* Make sure that drawn strings are white. */
  CopWait(cp, Y(START_DRAW), 0);
  CopSetColor(cp, 1, 0xfff);
  CopWait(cp, Y(Y0 + AMPL), 0);
  CopSetColor(cp, 1, back_pal.colors[1]);
  CopEnd(cp);
}

static void Init(void) {
  SetupPlayfield(MODE_LORES, DEPTH, X(0), Y(0), WIDTH, HEIGHT);

  cp = NewCopList(200);
  MakeCopperList(cp);
  CopListActivate(cp);
  CopListRun(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_SPRITE);
}

static void Kill(void) {
  DisableDMA(DMAF_BLITTER | DMAF_RASTER | DMAF_SPRITE);
  DeleteCopList(cp);
}

PROFILE(Spring);

static void Render(void) {
  short i, y_offset, k;
  /* Draw one half of springs and sprites every second frame. */
  k = (frameCount & 1) ? 4 : 0;

  ProfilerStart(Spring);
  for (i = 0 + k; i < 4 + k; i++) {
    y_offset = normfx(AMPL * COS(frameCount * 48 + factors[i]));
    SpriteUpdatePos(sprite[i], 16, X(X_OFFSET * i), Y(Y0 + y_offset));
    {
      Area2D area = {.x = X_OFFSET * i + 1, .y = START_DRAW, .w = 16, .h = 130};
      BlitterClearArea(&background_bmp, 0, &area);
    }
    DrawSpring(Y0 + y_offset, X_OFFSET * i + 9, 6, 8);
  }
  ProfilerStop(Spring);

  TaskWaitVBlank();
}

EFFECT(spring, NULL, NULL, Init, Kill, Render);
