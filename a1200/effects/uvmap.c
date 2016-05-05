#include "gfx/palette.h"
#include "gfx/png.h"
#include "uvmap/misc.h"
#include "uvmap/render.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static UVMapT *uvmap;
static PixBufT *texture;
static PixBufT *canvas;
static PixBufT *texture;
static PaletteT *texturePal;

static const int maps = 11;
static int lastMap = -1;
static void (*generate[])(UVMapT *) = {
  UVMapGenerate0, UVMapGenerate1, UVMapGenerate2, UVMapGenerate3,
  UVMapGenerate4, UVMapGenerate5, UVMapGenerate6, UVMapGenerate7,
  UVMapGenerate8, UVMapGenerate9, UVMapGenerate10
};

static void ChangeMap(int newMap) {
  while (newMap < 0)
    newMap += maps;

  newMap = newMap % maps;

  if (newMap != lastMap) {
    generate[newMap](uvmap);

    UVMapSetTexture(uvmap, texture);

    lastMap = newMap;
  }
}

static void Load() {
  LoadPngImage(&texture, &texturePal, "data/texture.png");
}

static void UnLoad() {
  MemUnref(texture);
  MemUnref(texturePal);
}

static void Init() {
  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  ChangeMap(0);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(texturePal);
}

static void Kill() {
  KillDisplay();

  MemUnref(uvmap);
  MemUnref(canvas);
}

static void Render(int frameNumber) {
  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  UVMapSetOffset(uvmap, du, dv);
  PROFILE(UVMapRender)
    UVMapRender(uvmap, canvas);
  PROFILE(C2P)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void HandleEvent(InputEventT *event) {
  if (KEY_RELEASED(event, KEY_RIGHT))
    ChangeMap(lastMap + 1);
  if (KEY_RELEASED(event, KEY_LEFT))
    ChangeMap(lastMap - 1);
}

EffectT Effect = { "UVMap", Load, UnLoad, Init, Kill, Render, HandleEvent };
