#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"

#include "gfx/blit.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/gradient.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *map1;
static PixBufT *map2;
static PixBufT *shade;
static PixBufT *lighten;
static PixBufT *darken;
static PixBufT *image;
static PaletteT *imagePal;

void AcquireResources() {
  LoadPngImage(&image, &imagePal, "data/samkaat-absinthe.png");
  LoadPngImage(&darken, NULL, "data/samkaat-absinthe-darken.png");
  LoadPngImage(&lighten, NULL, "data/samkaat-absinthe-lighten.png");
}

void ReleaseResources() {
  MemUnref(lighten);
  MemUnref(darken);
  MemUnref(image);
  MemUnref(imagePal);
}

void SetupEffect() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  map1 = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  map2 = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  shade = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);

  {
    FLineT line;
    FPointT pa = { 0, 0 };
    FPointT pb = { WIDTH, HEIGHT };

    FLineInitFromPoints(&line, &pa, &pb);
    LinearGradient(map1, &line);
  }

  {
    FPointT center = { WIDTH / 2, HEIGHT / 2 };
    CircularGradient(map2, &center);
  }

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(imagePal);
}

void TearDownEffect() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(map1);
  MemUnref(map2);
  MemUnref(shade);
}

static int Effect = 0;
static const int LastEffect = 4;

void RenderChunky(int frameNumber) {
  PixBufT *map;

  int change = (frameNumber * 2) % 256;

  switch (Effect) {
    case 0:
      map = map1;
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change);
      PixBufSetColorMap(shade, lighten);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 1:
      map = map1;
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change - 64);
      PixBufSetColorMap(shade, darken);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 2:
      map = map2;
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change - 64);
      PixBufSetColorMap(shade, lighten);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 3:
      map = map2;
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map, change - 64);
      PixBufSetColorMap(shade, darken);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    default:
      break;
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      Effect = (Effect + 1) % LastEffect;
    if (event == LOOP_PREV) {
      Effect--;
      if (Effect < 0)
        Effect += LastEffect;
    }

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
