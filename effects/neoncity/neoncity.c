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

static BitmapT *background, *foreground;
static CopListT *cp;
static CopInsT *bplcon1;

void DrawLayer1(void) {
  Area2D area = {.x = 0, .y = 0, .w = 16, .h = 16};
  short x, y;

  for (y = 0; y < HEIGHT / 16; y++) {
    for (x = 0; x < WIDTH / 16; x++) {
      u_short tile = layer1_map[y][x];
      area.y = tile * 16;
      BlitterCopyAreaSetup(background, x * 16, y * 16, &layer1_tiles, &area);
      BlitterCopyAreaStart(1, 0);
      BlitterCopyAreaStart(2, 1);
    }
  }
}

void DrawLayer2(void) {
  Area2D area = {.x = 0, .y = 0, .w = 16, .h = 16};
  short x, y;

  for (y = 0; y < HEIGHT / 16; y++) {
    for (x = 0; x < WIDTH / 16; x++) {
      u_short tile = layer2_map[y][x+10];
      area.y = tile * 16;
      BlitterCopyAreaSetup(foreground, x * 16, y * 16, &layer2_tiles, &area);
      BlitterCopyAreaStart(0, 0);
      BlitterCopyAreaStart(1, 1);
      BlitterCopyAreaStart(2, 2);
    }
  }
}


static void Init(void) {
  const short ys = (256 - layer0.height) / 2;
  short i;

  background = NewBitmapCustom(WIDTH, HEIGHT, DEPTH, BM_CLEAR | BM_DISPLAYABLE);
  foreground = NewBitmapCustom(WIDTH, HEIGHT, DEPTH, BM_CLEAR | BM_DISPLAYABLE);

  cp = NewCopList(100 + layer0.height * 4);

  CopInit(cp);
  CopSetupMode(cp, MODE_LORES|MODE_DUALPF, 2*DEPTH);
  CopSetupDisplayWindow(cp, MODE_LORES, X(0) + 1, Y(ys), WIDTH, HEIGHT);
  CopSetupBitplaneFetch(cp, MODE_LORES, X(0) + 1, WIDTH);
  CopSetupDualPlayfield(cp, NULL, foreground, background);
  bplcon1 = CopMove16(cp, bplcon1, 0);

  /* Palette */
  CopSetColor(cp, 0, 0x000);
  CopSetColor(cp, 1, 0x000);
  for (i = 1; i < 4; i++) {
    CopSetColor(cp, 2*i, layer1_pal.colors[i]);
    CopSetColor(cp, 2*i+1, layer1_pal.colors[i]);
  }
  CopLoadPal(cp, &layer2_pal, 8);

  {
    u_short *color = gradient.pixels;

    for (i = 0; i < layer0.height; i++) {
      u_short c0 = color[layer0.height - i - 1];
      u_short c1 = ~color[i];
      CopWait(cp, VP(ys + i), HP(0) - 8);
      CopSetColor(cp, 1, c1);
      CopSetColor(cp, 0, c0);
      CopWait(cp, VP(ys + i), HP(layer0.width) - 1);
      CopSetColor(cp, 0, 0);
    }
  }

  CopSetColor(cp, 0, 0x000);
  CopEnd(cp);

  CopListActivate(cp);

  EnableDMA(DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  BitmapCopyFast(background, 0, 0, &layer0);

  DrawLayer1();
  DrawLayer2();
}

static void Kill(void) {
  DisableDMA(DMAF_COPPER | DMAF_RASTER | DMAF_BLITTER | DMAF_BLITHOG);

  DeleteCopList(cp);
  DeleteBitmap(foreground);
  DeleteBitmap(background);
}

static void Render(void) {
  CopInsSet16(bplcon1, 16 * (15 - (frameCount % 16)));
  TaskWaitVBlank();
}

EFFECT(neoncity, NULL, NULL, Init, Kill, Render);
