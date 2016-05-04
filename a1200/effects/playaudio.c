#include "std/math.h"
#include "audio/stream.h"
#include "gfx/blit.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "system/audio.h"
#include "system/input.h"
#include "uvmap/generate.h"
#include "uvmap/render.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

UVMapGenerate(Polar, (1.0f - r * 0.7f), a);

static PixBufT *texture;
static PixBufT *image;
static PaletteT *imagePal;
static PixBufT *shade;
static UVMapT *uvmap;
static PixBufT *canvas;
static PixBufT *darken;
static AudioStreamT *audio;

static void Load() {
  LoadPngImage(&image, &imagePal, "data/samkaat-absinthe.png");
  LoadPngImage(&darken, NULL, "data/samkaat-absinthe-darken.png");
  audio = AudioStreamOpen("data/chembro.wav");
}

static void UnLoad() {
  MemUnref(audio);
  MemUnref(darken);
  MemUnref(imagePal);
  MemUnref(image);
}

static void Init() {
  texture = NewPixBuf(PIXBUF_GRAY, 256, 256);
  shade = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  UVMapGeneratePolar(uvmap);

  LoadPalette(imagePal);
  InitDisplay(WIDTH, HEIGHT, DEPTH);

  InitAudio();
  AudioStreamPlay(audio);
}

static void Kill() {
  AudioStreamStop(audio);
  KillAudio();
  KillDisplay();

  MemUnref(canvas);
  MemUnref(uvmap);
  MemUnref(shade);
  MemUnref(texture);
}

static void Render(int frameNumber) {
  AudioBufferT *buffer = AudioStreamGetBuffer(audio);
  static int array[256];
  int i;

  PixBufClear(texture);

  for (i = 0; i < min(buffer->length, texture->width); i++) {
    int y1 = abs((int8_t)buffer->left.hi[i]);
    int y2 = abs((int8_t)buffer->right.hi[i]);
    int y = y1 + y2;

    if (y > 255)
      y = 255;

    array[i] = (array[(i - 1) & 0xff] + y) / 2;
  }

  for (i = 0; i < min(buffer->length, texture->width); i++) {
    texture->fgColor = 255;
  }

  PixBufCopy(canvas, image);

  UVMapSetTexture(uvmap, texture);
  UVMapSetOffset(uvmap, 0, 0);
  UVMapRender(uvmap, shade);

  PixBufSetColorMap(shade, darken);
  PixBufSetBlitMode(shade, BLIT_COLOR_MAP);
  PixBufBlit(canvas, 0, 0, shade, NULL);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  AudioStreamFeed(audio);
}

static void HandleEvent(InputEventT *event) {
  int frameCounter = ReadFrameCounter();
  bool changed = false;

  if (KEY_RELEASED(event, KEY_UP)) {
    frameCounter -= 10 * FRAMERATE;
    changed = true;
  } else if (KEY_RELEASED(event, KEY_DOWN)) {
    frameCounter += 10 * FRAMERATE;
    changed = true;
  } else if (KEY_RELEASED(event, KEY_LEFT)) {
    frameCounter -= FRAMERATE;
    changed = true;
  } else if (KEY_RELEASED(event, KEY_RIGHT)) {
    frameCounter += FRAMERATE;
    changed = true;
  }

  if (changed) {
    SetFrameCounter(frameCounter);
    AudioStreamUpdatePos(audio);
  }
}

EffectT Effect = { "PlayAudio", Load, UnLoad, Init, Kill, Render, HandleEvent };
