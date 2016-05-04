#include "gfx/blit.h"
#include "gfx/ellipse.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "system/fileio.h"
#include "uvmap/misc.h"
#include "uvmap/render.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *texture[2];
static PaletteT *texturePal[2];
static PixBufT *composeMap;
static PixBufT *component;
static UVMapT *uvmap[2];
static PixBufT *canvas;
static uint8_t *colorFunc;

static void Load() {
  LoadPngImage(&texture[0], &texturePal[0], "data/texture-128-01.png");
  LoadPngImage(&texture[1], &texturePal[1], "data/texture-128-02.png");
}

static void UnLoad() {
  MemUnref(texture[0]);
  MemUnref(texturePal[0]);
  MemUnref(texture[1]);
  MemUnref(texturePal[1]);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  uvmap[0] = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerate3(uvmap[0]);
  UVMapSetTexture(uvmap[0], texture[0]);

  uvmap[1] = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerate4(uvmap[1]);
  UVMapSetTexture(uvmap[1], texture[1]);

  component = NewPixBufWrapper(WIDTH, HEIGHT, uvmap[1]->map.fast.v);
  composeMap = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  colorFunc = NewTable(uint8_t, 256);

  PixBufRemap(texture[1], texturePal[1]);

  LinkPalettes(texturePal[0], texturePal[1], NULL);
  LoadPalette(texturePal[0]);
  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

static void Kill() {
  KillDisplay();

  UnlinkPalettes(texturePal[0]);

  MemUnref(canvas);
  MemUnref(uvmap[0]);
  MemUnref(uvmap[1]);
  MemUnref(component);
  MemUnref(composeMap);
  MemUnref(colorFunc);
}

static int effectNum = 0;

static void Render(int frameNumber) {
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  if (effectNum == 0) {
    int i;

    for (i = 0; i < 256; i++) {
      int v = 128 - ((frameNumber * 2) % 256 + i);
      colorFunc[i] = (v & 0xff) >= 128 ? 1 : 0;
    }

    PixBufSetColorFunc(component, colorFunc);
    PixBufSetBlitMode(component, BLIT_COLOR_FUNC);
    PixBufBlit(composeMap, 0, 0, component, NULL);
  } else {
    PixBufClear(composeMap);
    composeMap->fgColor = 1;
    DrawEllipse(composeMap,
                160, 128,
                40 + sin((float)frameNumber / (4.0f * M_PI)) * 40.0f,
                32 + sin((float)frameNumber / (4.0f * M_PI)) * 32.0f);
  }

  UVMapSetOffset(uvmap[0], du, dv);
  UVMapSetOffset(uvmap[1], -du, -dv);
  PROFILE (UVMapCompose1)
    UVMapComposeAndRender(uvmap[0], canvas, composeMap, 0);
  PROFILE (UVMapCompose2)
    UVMapComposeAndRender(uvmap[1], canvas, composeMap, 1);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void HandleEvent(InputEventT *event) {
  if (KEY_RELEASED(event, KEY_RIGHT))
    effectNum = (effectNum + 1) % 2;
  if (KEY_RELEASED(event, KEY_LEFT))
    effectNum = (effectNum - 1) % 2;
}

EffectT Effect = { "UVMapCompose", Load, UnLoad, Init, Kill, Render, HandleEvent };
