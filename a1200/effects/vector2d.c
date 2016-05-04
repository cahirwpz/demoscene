#include <math.h>

#include "gfx/ellipse.h"
#include "gfx/line.h"
#include "gfx/ms2d.h"
#include "gfx/triangle.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static MatrixStack2D *ms;

static PointT cross[] = {
  {1, 0}, {2, 0}, {2, 1}, {3, 1},
  {3, 2}, {2, 2}, {2, 3}, {1, 3},
  {1, 2}, {0, 2}, {0, 1}, {1, 1}
};
static PointT crossToDraw[12];

static PointT triangle[] = { {-15, -10}, {10, -5}, {0, 20} };
static PointT triangleToDraw[3];

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  PixBufClear(canvas);

  ms = NewMatrixStack2D();

  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

static void Kill() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(ms);
}

static int effect = 0;
static const int lastEffect = 3;

static void Render(int frameNumber) {
  PointT *toDraw = triangleToDraw;

  float s = sin(frameNumber * 3.14159265f / 22.5f);
  float c = cos(frameNumber * 3.14159265f / 45.0f);

  StackReset(ms);
  PushTranslation2D(ms, -1.5f, -1.5f);
  PushScaling2D(ms, 20.0f + 10.0f * s, 20.0f + 10.0f * s);
  PushRotation2D(ms, (float)(frameNumber * -3));
  PushTranslation2D(ms, (float)(WIDTH/2) + c * (WIDTH/4), (float)(HEIGHT/2));

  Transform2D(crossToDraw, cross, 12, GetMatrix2D(ms, 0));

  if (effect == 0) {
    PixBufClear(canvas);
    DrawPolyLine(canvas, crossToDraw, 12, TRUE);
  }

  StackReset(ms);
  PushTranslation2D(ms, 5.0f, 10.0f);
  PushScaling2D(ms, 2.5f, 2.5f);
  PushRotation2D(ms, (float)(frameNumber*5*c));
  PushTranslation2D(ms, WIDTH/2 + c * 50, HEIGHT/2 + s * 20);

  Transform2D(triangleToDraw, triangle, 3, GetMatrix2D(ms, 0));

  frameNumber &= 255;

  if (frameNumber < 128)
    canvas->fgColor = frameNumber * 2;
  else 
    canvas->fgColor = (255 - frameNumber) * 2;

  if (effect == 1) {
    TriPoint p1 = { toDraw[0].x, toDraw[0].y };
    TriPoint p2 = { toDraw[1].x, toDraw[1].y };
    TriPoint p3 = { toDraw[2].x, toDraw[2].y };
    DrawTriangle(canvas, &p1, &p2, &p3);
  }

  if (effect == 2) {
    DrawEllipse(canvas,
                toDraw[1].x, toDraw[1].y,
                30 + c * 15, 30 + s * 15);
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void HandleEvent(InputEventT *event) {
  PixBufClear(canvas);

  if (KEY_RELEASED(event, KEY_RIGHT))
    effect = (effect + 1) % lastEffect;
  if (KEY_RELEASED(event, KEY_LEFT)) {
    effect--;
    if (effect < 0)
      effect += lastEffect;
  }
}

EffectT Effect = { "Vector2D", NULL, NULL, Init, Kill, Render, HandleEvent };
