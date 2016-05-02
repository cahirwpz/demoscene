#include "gfx/blit.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/gradient.h"
#include "uvmap/misc.h"
#include "uvmap/render.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static UVMapT *uvmap;
static PixBufT *canvas;
static PixBufT *shades;
static PixBufT *texture;
static PixBufT *colorMap;
static PixBufT *component;
static PaletteT *texturePal;
static uint8_t *colorFunc;

static void Load() {
  LoadPngImage(&texture, &texturePal, "data/texture-shades.png");
  LoadPngImage(&colorMap, NULL, "data/texture-shades-map.png");
}

static void UnLoad() {
  MemUnref(texture);
  MemUnref(texturePal);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  shades = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  PixBufSetColorMap(shades, colorMap);

  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  uvmap->lightMap = shades;
  UVMapGenerate4(uvmap);
  UVMapSetTexture(uvmap, texture);

  component = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);
  colorFunc = NewTable(uint8_t, 256);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(texturePal);
}

static void Kill() {
  KillDisplay();
  
  MemUnref(canvas);
  MemUnref(shades);
  MemUnref(uvmap);
  MemUnref(component);
  MemUnref(colorFunc);
}

static void RenderEffect(int frameNumber) {
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  {
    int i;

    for (i = 0; i < 256; i++) {
      uint8_t v = i + du;

      if (v >= 128)
        colorFunc[i] = ~v * 2;
      else 
        colorFunc[i] = v * 2;
    }
  }

  PixBufSetBlitMode(component, BLIT_COLOR_FUNC);
  PixBufSetColorFunc(component, colorFunc);
  PROFILE (PixBufBlitWithColorFunc)
    PixBufBlit(shades, 0, 0, component, NULL);

  UVMapSetOffset(uvmap, du, dv);
  PROFILE (UVMapRenderFast)
    UVMapRender(uvmap, canvas);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void Loop() {
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

EffectT Effect = { "UVMapLight", Load, UnLoad, Init, Kill, Loop };
