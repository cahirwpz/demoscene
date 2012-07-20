#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/canvas.h"
#include "gfx/line.h"
#include "tools/curves.h"
#include "tools/frame.h"
#include "txtgen/procedural.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

void PixBufBlitFlare(PixBufT *dstBuf asm("a0"),
                     size_t x asm("d0"), size_t y asm("d1"),
                     PixBufT *srcBuf asm("a1"))
{
  size_t stride = dstBuf->width - srcBuf->width;

  uint8_t *src = srcBuf->data;
  uint8_t *dst;

  x -= srcBuf->width / 2;
  y -= srcBuf->height / 2;
 
  dst = &dstBuf->data[y * dstBuf->width + x];

  y = srcBuf->height;

  do {
    x = srcBuf->width;

    do {
      int v = *dst + *src++;

      if (v > 255)
        v = 255;

      *dst++ = v;
    } while (--x);

    dst += stride;
  } while (--y);
}

const int CYCLEFRAMES = 150;
const int POINTS = 64;
const int SEGMENTS = 16;
const int FLARES = 6;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Points", NewTable(PointT, SEGMENTS + 1));
  ResAdd("Flare", NewPixBuf(PIXBUF_GRAY, 32, 32));
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));
}

/*
 * Set up display function.
 */
bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  float lightRadius = 1.0f;

  CanvasFill(R_("Canvas"), 0);
  GeneratePixels(R_("Flare"),
                 (GenPixelFuncT)LightNormalFalloff, &lightRadius);
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
const int CURVE = 6;

void RenderFlares(int frameNumber) {
  CanvasT *canvas = R_("Canvas");
  PointT *points = R_("Points");

  CanvasFill(canvas, 0);

  {
    float t = (float)frameNumber / CYCLEFRAMES;
    int i;

    /* calculate points */
    for (i = 0; i <= SEGMENTS; i++) {
      float x, y;

      switch (CURVE) {
        case 0: /* LineSegment */
          CurveLineSegment(t + (float)i / POINTS, -80, -64, 80, 64, &x, &y);
          break;

        case 1: /* Limaçon */
          CurveEpitrochoid(t + (float)i / POINTS, 20, 20, 2.0f, &y, &x);
          break;

        case 2: /* Epicycloid */
          CurveEpitrochoid(t + (float)i / POINTS, 64, 16, 1.0f, &x, &y);
          break;

        case 3: /* Epitrochoid 1 */
          CurveEpitrochoid(t + (float)i / POINTS, 45, 15, 2.0f, &x, &y);
          break;

        case 4: /* Epitrochoid 2 */
          CurveEpitrochoid(t + (float)i / POINTS, 80, 20, 0.5f, &x, &y);
          break;

        case 5: /* Hypocycloid */
          CurveHypotrochoid(t + (float)i / POINTS, 80, 20, 1.0f, &x, &y);
          break;

        case 6: /* Hypotrochoid 1 */
          CurveHypotrochoid(t + (float)i / POINTS, 80, 20, 2.0f, &x, &y);
          break;

        case 7: /* Hypotrochoid 2 */
          CurveHypotrochoid(t + (float)i / POINTS, 80, 20, 0.5f, &x, &y);
          break;

        default:
          break;
      }

      points[i].x = (int)x + WIDTH / 2;
      points[i].y = (int)y + HEIGHT / 2;
    }

    /* draw lines */
    for (i = 1; i <= SEGMENTS; i++) {
      canvas->fg_col = i * 255 / SEGMENTS; 
      DrawLine(canvas, points[i-1].x, points[i-1].y, points[i].x, points[i].y);
    }

    for (i = 0; i < FLARES; i++) {
      size_t p = SEGMENTS * i / (FLARES - 1);

      PixBufBlitFlare(canvas->pixbuf, points[p].x, points[p].y, R_("Flare"));
    }
  }
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(R_("Canvas")),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  while (GetVBlankCounter() < CYCLEFRAMES * 4) {
    int frameNumber = GetVBlankCounter();

    EventQueueReset();

    RenderFlares(frameNumber);
    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  }
}
