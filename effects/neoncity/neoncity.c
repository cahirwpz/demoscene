#include "effect.h"
#include "hardware.h"
#include "copper.h"
#include "pixmap.h"
#include "blitter.h"
#include "gfx.h"
#include "tasks.h"

#define WIDTH 320
#define HEIGHT 176
#define DEPTH 3

#include "data/gradient.c"
#include "data/layer0.c"
#include "data/layer1-map.c"
#include "data/layer1-tiles.c"
#include "data/layer2-map.c"
#include "data/layer2-tiles.c"

static BitmapT *background_bm, *foreground_bm;
static BitmapT background[1], foreground[1];
static CopListT *cp;
static CopInsT *bplcon1;
static CopInsT *bplptr[DEPTH + DEPTH];

__regargs void UpdateLayer1(short dstX, short srcX, short width) {
  Area2D area = {.x = 0, .y = 0, .w = 16, .h = 16};
  short x, y;

  for (y = 0; y < HEIGHT / 16; y++) {
    for (x = 0; x < width; x++) {
      u_short tile = layer1_map[y][srcX + x];
      area.y = tile * 16;
      BlitterCopyAreaSetup(background, (dstX + x) * 16, y * 16,
                           &layer1_tiles, &area);
      BlitterCopyAreaStart(1, 0);
      BlitterCopyAreaStart(2, 1);
    }
  }
}

__regargs void UpdateLayer2(short dstX, short srcX, short width) {
  Area2D area = {.x = 0, .y = 0, .w = 16, .h = 16};
  short x, y;

  for (y = 0; y < HEIGHT / 16; y++) {
    for (x = 0; x < width; x++) {
      u_short tile = layer2_map[y][srcX + x];
      area.y = tile * 16;
      BlitterCopyAreaSetup(foreground, (dstX + x) * 16, y * 16,
                           &layer2_tiles, &area);
      BlitterCopyAreaStart(0, 0);
      BlitterCopyAreaStart(1, 1);
      BlitterCopyAreaStart(2, 2);
    }
  }
}


static void Init(void) {
  const short ys = (256 - layer0.height) / 2;
  short i;

  background_bm = NewBitmapCustom(WIDTH + 16, HEIGHT,
                                  DEPTH, BM_DISPLAYABLE);
  foreground_bm = NewBitmapCustom(WIDTH + 16, HEIGHT + 6, DEPTH,
                                  BM_DISPLAYABLE);

  InitSharedBitmap(background, WIDTH + 16, HEIGHT, DEPTH, background_bm);
  InitSharedBitmap(foreground, WIDTH + 16, HEIGHT, DEPTH, foreground_bm);

  cp = NewCopList(100 + HEIGHT * 5);

  CopInit(cp);
  CopSetupMode(cp, MODE_LORES|MODE_DUALPF, 2 * DEPTH);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0) + 1, Y(ys), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0) + 1 - 16, WIDTH + 16);
  CopSetupDualPlayfield(cp, bplptr, foreground, background);

  bplcon1 = CopMove16(cp, bplcon1, 0);

  /* Palette */
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0x000);
  for (i = 1; i < layer1_pal.count; i++) {
    CopSetColor(cp, 2*i, layer1_pal.colors[i]);
    CopSetColor(cp, 2*i+1, layer1_pal.colors[i]);
  }
  CopLoadPal(cp, &layer2_pal, 8);

  {
    u_short *color = gradient.pixels;

    for (i = 0; i < HEIGHT; i++) {
      u_short c0 = color[HEIGHT - i - 1];
      u_short c1 = ~color[i];
      CopWait(cp, VP(ys + i), HP(0) - 12);
      CopSetColor(cp, 1, c1);
      CopSetColor(cp, 0, c0);
      CopWait(cp, VP(ys + i), HP(WIDTH) - 8);
      CopSetColor(cp, 0, 0);
    }
  }

  CopSetColor(cp, 0, 0x000);
  CopEnd(cp);

  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp);
  DeleteBitmap(foreground_bm);
  DeleteBitmap(background_bm);
}

static short ScrollLayer1(short t) {
  short shift = 15 - (t % 16);

  if (shift == 15) {
    short k = (t >> 4) % (layer1_map_width - WIDTH / 16);
    short i;

    for (i = 1; i < DEPTH; i++)
      background->planes[i] = background_bm->planes[i] + k * 2;

    if (k == 0)
      UpdateLayer1(0, 0, WIDTH/16 + 1);
    else
      UpdateLayer1(WIDTH/16, WIDTH/16 + k, 1);

    CopInsSet32(bplptr[2], background->planes[1]);
    CopInsSet32(bplptr[4], background->planes[2]);
  }

  return shift;
}

static short ScrollLayer2(short t) {
  short shift = 15 - (t % 16);

  if (shift == 15) {
    short k = (t >> 4) % (layer2_map_width - WIDTH / 16);
    short i;

    for (i = 0; i < DEPTH; i++)
      foreground->planes[i] = foreground_bm->planes[i] + k * 2;

    if (k == 0)
      UpdateLayer2(0, 0, WIDTH/16 + 1);
    else
      UpdateLayer2(WIDTH/16, WIDTH/16 + k, 1);

    CopInsSet32(bplptr[1], foreground->planes[0]);
    CopInsSet32(bplptr[3], foreground->planes[1]);
    CopInsSet32(bplptr[5], foreground->planes[2]);
  }

  return shift;
}

static void Render(void) {
  int lines = ReadLineCounter();
  {
    short l1_shift = ScrollLayer1(frameCount / 2);
    short l2_shift = ScrollLayer2(frameCount);


    if (0)
      BitmapCopyFast(background_bm, 15 - l1_shift, 0, &layer0);

    CopInsSet16(bplcon1, (l2_shift << 4) | l1_shift);
  }
  Log("neoncity: %d\n", ReadLineCounter() - lines);

  TaskWaitVBlank();
}

EFFECT(neoncity, NULL, NULL, Init, Kill, Render);
