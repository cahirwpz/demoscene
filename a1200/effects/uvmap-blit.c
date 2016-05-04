#include "gfx/blit.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "txtgen/procedural.h"
#include "uvmap/misc.h"
#include "uvmap/render.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *flare;
static PixBufT *origU;
static PixBufT *origV;
static UVMapT *uvmap;
static PixBufT *texture;
static PaletteT *texturePal;

static void Load() {
  LoadPngImage(&texture, &texturePal, "data/texture-01.png");
}

static void UnLoad() {
  MemUnref(&texture);
  MemUnref(&texturePal);
}

static void Init() {
  float lightRadius = 1.0f;
  int i;

  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  PixBufClear(canvas);

  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerateTunnel(uvmap, 32.0f, 3, 4.0 / 3.0, 0.5, 0.5, NULL);
  UVMapSetTexture(uvmap, texture);

  flare = NewPixBuf(PIXBUF_GRAY, 64, 64);
  GeneratePixels(flare, (GenPixelFuncT)LightLinearFalloff, &lightRadius);
  for (i = 0; i < flare->width * flare->height; i++)
    flare->data[i] /= 4;

  origU = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  PixBufBlit(origU, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u), NULL);

  origV = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  PixBufBlit(origV, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.v), NULL);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(texturePal);
}

static void Kill() {
  KillDisplay();

  MemUnref(origU);
  MemUnref(origU);
  MemUnref(flare);
  MemUnref(uvmap);
  MemUnref(canvas);
}

static void Render(int frameNumber) {
  PixBufT *umap;

  int du = frameNumber;
  int dv = 2 * frameNumber;
  int i;

  umap = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);

  UVMapSetOffset(uvmap, du, dv);
  PixBufBlit(umap, 0, 0, origU, NULL);

  for (i = 0; i < 8; i++) {
    PixBufSetBlitMode(flare, BLIT_ADDITIVE);
    PixBufBlit(umap, 
               (int)(128.0f + sin(-frameNumber * M_PI / 45.0f + M_PI * i / 4) * 80.0f), 
               (int)(96.0f + cos(-frameNumber * M_PI / 45.0f + M_PI * i / 4) * 48.0f),
               flare, NULL);
  }

  UVMapRender(uvmap, canvas);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);

  MemUnref(umap);
}

EffectT Effect = { "UVMapBlit", Load, UnLoad, Init, Kill, Render };
