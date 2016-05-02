#include <stdlib.h>

#include "std/fastmath.h"
#include "std/math.h"
#include "gfx/ellipse.h"
#include "gfx/layers.h"
#include "gfx/rectangle.h"
#include "gfx/png.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 200;
const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *layerMap;
static PixBufT *image[3];
static PaletteT *imagePal[3];

static void Load() {
  LoadPngImage(&image[0], &imagePal[0], "data/last-hope-64.png");
  LoadPngImage(&image[1], &imagePal[1], "data/bus-stop-64.png");
  LoadPngImage(&image[2], &imagePal[2], "data/dragon-128.png");
}

static void UnLoad() {
  int i;

  for (i = 0; i < 3; i++) {
    MemUnref(image[i]);
    MemUnref(imagePal[i]);
  }
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  layerMap = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);

  PixBufRemap(image[1], imagePal[1]);
  PixBufRemap(image[2], imagePal[2]);

  LinkPalettes(imagePal[0], imagePal[1], imagePal[2], NULL);
  LoadPalette(imagePal[0]);
  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

static void Kill() {
  KillDisplay();

  UnlinkPalettes(imagePal[0]);

  MemUnref(canvas);
  MemUnref(layerMap);
}

static int effect = 0;
static const int lastEffect = 1;

static void RenderChunky(int frameNumber) {
  int x = 160.0 * sin(M_PI * frameNumber / 100);
  int y = 100.0 * sin(M_PI * frameNumber / 100);

  PROFILE(DrawComposeMap)
    switch (effect) {
      case 0:
        PixBufClear(layerMap);
        layerMap->fgColor = 2;
        DrawRectangle(layerMap, 0, 0, 160 + x, 100 + y);
        DrawRectangle(layerMap, 160 - x, 100 - y, 320, 200);
        layerMap->fgColor = 1;
        DrawEllipse(layerMap, 160, 100, -x, -y);
        break;
      
      default:
        break;
    }

  PROFILE(LayersCompose)
    LayersCompose(canvas, layerMap, image, 3);

  PROFILE(ChunkyToPlanar)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void Loop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      effect = (effect + 1) % lastEffect;
    if (event == LOOP_PREV) {
      effect--;
      if (effect < 0)
        effect += lastEffect;
    }

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}

EffectT Effect = { "Layers", Load, UnLoad, Init, Kill, Loop };
