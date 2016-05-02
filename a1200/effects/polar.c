#include <stdlib.h>

#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/random.h"

#include "gfx/blit.h"
#include "gfx/pixbuf.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"
#include "tools/gradient.h"
#include "uvmap/generate.h"
#include "uvmap/render.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

const int WIDTH = 256;
const int HEIGHT = 256;
const int DEPTH = 8;

static void UVMapGenerateToPolar(UVMapT *map) {
  float dx = 1.0f / (int)map->width;
  float dy = 1.0f / (int)map->height;
  int i, j, k;

  for (i = 0, k = 0; i < map->height; i++)
    for (j = 0; j < map->width; j++, k++) {
      float x = (float)j * dx;
      float y = (float)i * dy;
      float r = x / M_SQRT2;
      float a = y * 2.0f * M_PI;
      float u = cos(a) * r + 0.5f;
      float v = sin(a) * r + 0.5f;

      if (u < 0.0f)
        u = 0.0f;
      if (u >= 0.995f)
        u = 0.995f;

      if (v < 0.0f)
        v = 0.0f;
      if (v >= 0.995f)
        v = 0.995f;

      UVMapSet(map, k, u, v);
    }
}

UVMapGenerate(ToCartesian, a / (2.0f * M_PI), r / M_SQRT2);

static UVMapT *cartesianMap;
static UVMapT *polarMap;
static PixBufT *canvas;
static PixBufT *polarImg;
static PixBufT *polarBuf;
static PixBufT *origU;
static PixBufT *origV;

void AcquireResources() {
  LoadPngImage(&polarImg, NULL, "data/polar-map.png");
}

void ReleaseResources() {
  MemUnref(polarImg);
}

void SetupEffect() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  polarMap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerateToPolar(polarMap);

  cartesianMap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerateToCartesian(cartesianMap);

  if (false)
  {
    FPointT p = {128.0, 128.0};
    CircularGradient(polarImg, &p);
  }

  origU = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  PixBufBlit(origU, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, cartesianMap->map.fast.u), NULL);

  origV = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  PixBufBlit(origV, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, cartesianMap->map.fast.v), NULL);

  polarBuf = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void TearDownEffect() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(polarMap);
  MemUnref(cartesianMap);
  MemUnref(origU);
  MemUnref(origV);
  MemUnref(polarBuf);
}

void RenderPolar(PixBufT *canvas, int frameNumber) {
  UNUSED float a = frameNumber * M_PI / 256;
  UNUSED float u = sin(a) * 32.0f;
  UNUSED float v = cos(a) * 32.0f;

  UVMapSetOffset(polarMap, (int)u, (int)v);
  UVMapSetTexture(polarMap, polarImg);
  UVMapRender(polarMap, polarBuf);

  {
    int x, y;
    uint8_t *pixels = polarBuf->data;

    for (y = 0; y < polarBuf->height; y++) {
      int c = (uint8_t)*pixels++;

      for (x = 1; x < polarBuf->width; x++) {
        int d = (uint8_t)*pixels;

        c -= 4;

        if (c < 0)
          c = 0;

        *pixels++ = max(c, d);

        if (c < d)
          c = d;
      }
    }
  }

  UVMapSetTexture(cartesianMap, polarBuf);
  UVMapRender(cartesianMap, canvas);
}

void RenderEffect(int frameNumber) {
  PROFILE(Polar)
    RenderPolar(canvas, frameNumber);
  PROFILE(C2P)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
