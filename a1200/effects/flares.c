#include <math.h>


#include "gfx/blit.h"
#include "gfx/spline.h"
#include "gfx/line.h"
#include "tools/curves.h"
#include "txtgen/procedural.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const int CYCLEFRAMES = 100;
const int POINTS = 64;
const int SEGMENTS = 16;
const int FLARES = 6;

static PointT *points;
static PixBufT *flare;
static PixBufT *canvas;
static SplineT *splineX;
static SplineT *splineY;

static void Init() {
  float lightRadius = 1.0f;

  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  PixBufClear(canvas);

  flare = NewPixBuf(PIXBUF_GRAY, 32, 32);
  GeneratePixels(flare, (GenPixelFuncT)LightNormalFalloff, &lightRadius);
  PixBufSetBlitMode(flare, BLIT_ADDITIVE_CLIP);

  splineX = NewSpline(4, TRUE);
  splineY = NewSpline(4, TRUE);
  splineX->knots[0].value = 64;
  splineY->knots[0].value = 128;
  splineX->knots[1].value = 128;
  splineY->knots[1].value = 64;
  splineX->knots[2].value = 192;
  splineY->knots[2].value = 192;
  splineX->knots[3].value = 256;
  splineY->knots[3].value = 128;
  SplineAttachCatmullRomTangents(splineX);
  SplineAttachCatmullRomTangents(splineY);

  points = NewTable(PointT, SEGMENTS + 1);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

static void Kill() {
  KillDisplay();

  MemUnref(points);
  MemUnref(flare);
  MemUnref(canvas);
  MemUnref(splineX);
  MemUnref(splineY);
}

static int curve = 8;
static const int lastCurve = 9;

static void RenderFlares(int frameNumber) {
  PixBufClear(canvas);

  {
    float t = (float)frameNumber / CYCLEFRAMES;
    int i;

    /* calculate points */
    for (i = 0; i <= SEGMENTS; i++) {
      float x, y;

      switch (curve) {
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

        case 8: /* Spline */
          {
            float nt = t + (float)i / POINTS;

            nt = fmod(nt, 1.0);
            x = SplineEval(splineX, nt) - WIDTH / 2;
            y = SplineEval(splineY, nt) - HEIGHT / 2;
          }
          break;

        default:
          break;
      }

      points[i].x = (int)x + WIDTH / 2;
      points[i].y = (int)y + HEIGHT / 2;
    }

    /* draw lines */
    for (i = 1; i <= SEGMENTS; i++) {
      canvas->fgColor = i * 255 / SEGMENTS; 
      DrawLine(canvas, points[i-1].x, points[i-1].y, points[i].x, points[i].y);
    }

    for (i = 2; i < FLARES; i++) {
      size_t p = SEGMENTS * i / (FLARES - 1);

      PixBufBlit(canvas, 
                 points[p].x - flare->width / 2,
                 points[p].y - flare->height / 2,
                 flare, NULL);
    }
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void Loop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      curve = (curve + 1) % lastCurve;
    if (event == LOOP_PREV) {
      curve--;
      if (curve < 0)
        curve += lastCurve;
    }

    RenderFlares(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}

EffectT Effect = { "Flares", NULL, NULL, Init, Kill, Loop };
