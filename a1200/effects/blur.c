#include "gfx/blit.h"
#include "gfx/filter.h"
#include "gfx/png.h"

#include "startup.h"

static const int WIDTH = 320;
static const int HEIGHT = 256;
static const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *buffer;
static PixBufT *image;

static void Load() {
  LoadPngImage(&image, NULL, "data/samkaat-absinthe.png");
}

static void UnLoad() {
  MemUnref(image);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  buffer = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

static void Kill() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(buffer);
}

static bool init = true;
static int effect = 0;
static const int lastEffect = 2;

static void RenderEffect(int frameNumber) {
  if (init) {
    PixBufCopy(buffer, image);
    init = false;
  }

  if (effect == 0)
    BlurV3(canvas, buffer);
  if (effect == 1)
    BlurH3(canvas, buffer);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  PixBufSwapData(canvas, buffer);
}

static void Loop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT) {
      effect = (effect + 1) % lastEffect;
      init = true;
    }
    if (event == LOOP_PREV) {
      effect--;
      if (effect < 0)
        effect += lastEffect;
      init = true;
    }

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}

EffectT Effect = { "Blur", Load, UnLoad, Init, Kill, Loop };
