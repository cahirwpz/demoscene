#include "std/fastmath.h"
#include "std/math.h"
#include "gfx/blit.h"
#include "gfx/png.h"
#include "tools/gradient.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *map[2];
static PixBufT *shade;
static PixBufT *lighten;
static PixBufT *darken;
static PixBufT *image;
static PaletteT *imagePal;

static void Load() {
  LoadPngImage(&image, &imagePal, "data/samkaat-absinthe.png");
  LoadPngImage(&darken, NULL, "data/samkaat-absinthe-darken.png");
  LoadPngImage(&lighten, NULL, "data/samkaat-absinthe-lighten.png");
}

static void UnLoad() {
  MemUnref(lighten);
  MemUnref(darken);
  MemUnref(image);
  MemUnref(imagePal);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  map[0] = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  map[1] = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  shade = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);

  {
    FLineT line;
    FPointT pa = { 0, 0 };
    FPointT pb = { WIDTH, HEIGHT };

    FLineInitFromPoints(&line, &pa, &pb);
    LinearGradient(map[0], &line);
  }

  {
    FPointT center = { WIDTH / 2, HEIGHT / 2 };
    CircularGradient(map[1], &center);
  }

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(imagePal);
}

static void Kill() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(map[0]);
  MemUnref(map[1]);
  MemUnref(shade);
}

static int effect = 0;
static const int lastEffect = 4;

static void Render(int frameNumber) {
  int change = (frameNumber * 2) % 256;

  switch (effect) {
    case 0:
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map[0], change);
      PixBufSetColorMap(shade, lighten);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 1:
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map[0], change - 64);
      PixBufSetColorMap(shade, darken);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 2:
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map[1], change - 64);
      PixBufSetColorMap(shade, lighten);
      PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
      PROFILE(PixBufBlit)
        PixBufBlit(canvas, 0, 0, shade, NULL);
      break;

    case 3:
      PROFILE(PixBufCopy)
        PixBufCopy(canvas, image);
      PROFILE(PixBufAddAndClamp)
        PixBufAddAndClamp(shade, map[1], change - 64);
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

static void HandleEvent(InputEventT *event) {
  if (KEY_RELEASED(event, KEY_RIGHT))
    effect = (effect + 1) % lastEffect;
  if (KEY_RELEASED(event, KEY_LEFT)) {
    effect--;
    if (effect < 0)
      effect += lastEffect;
  }
}

EffectT Effect = { "ColorMap", Load, UnLoad, Init, Kill, Render, HandleEvent };
