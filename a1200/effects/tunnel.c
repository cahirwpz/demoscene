#include <math.h>

#include "gfx/blit.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "system/fileio.h"
#include "uvmap/generate.h"
#include "uvmap/render.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *texture;
static PixBufT *credits;
static PixBufT *whelpz;
static PaletteT *texturePal;
static PaletteT *creditsPal;
static PaletteT *whelpzPal;
static PaletteT *effectPal;
static UVMapT *tunnelMap;

static void Load() {
  LoadPngImage(&texture, &texturePal, "data/texture-01.png");
  LoadPngImage(&credits, &creditsPal, "data/code.png");
  LoadPngImage(&whelpz, &whelpzPal, "data/whelpz.png");
}

static void UnLoad() {
  MemUnref(texture);
  MemUnref(texturePal);
  MemUnref(credits);
  MemUnref(creditsPal);
  MemUnref(whelpz);
  MemUnref(whelpzPal);
}

static void Init() {
  static TunnelPetalsT petals = { 3, 0.333333f, 0.33333f };

  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  tunnelMap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerateTunnel(tunnelMap, 32.0f, 1, 16.0f / 9.0f, 0.5f, 0.5f, &petals);

  effectPal = MemClone(texturePal);

  PixBufRemap(credits, creditsPal);
  PixBufSetBlitMode(credits, BLIT_TRANSPARENT);
  PixBufRemap(whelpz, whelpzPal);
  PixBufSetBlitMode(whelpz, BLIT_TRANSPARENT);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LinkPalettes(texturePal, whelpzPal, creditsPal, NULL);
  LoadPalette(texturePal);
}

static void Kill() {
  KillDisplay();

  UnlinkPalettes(texturePal);
  MemUnref(tunnelMap);
  MemUnref(canvas);
  MemUnref(effectPal);
}

typedef void (*PaletteFunctorT)(int frameNumber, HSL *hsl);

static void CyclicHue(int frameNumber, HSL *hsl) {
  hsl->h += (float)(frameNumber & 255) / 256.0f;

  if (hsl->h > 1.0f)
    hsl->h -= 1.0f;
}

static void PulsingSaturation(int frameNumber, HSL *hsl) {
  float s = sin(frameNumber * M_PI / 50.0f) * 1.00f;
  float change = (s > 0.0f) ? (1.0f - hsl->s) : (hsl->s);

  hsl->s += change * s;
}

static void PulsingLuminosity(int frameNumber, HSL *hsl) {
  float s = sin(frameNumber * M_PI / 12.5f) * 0.66f;
  float change = (s > 0.0f) ? (1.0f - hsl->l) : (hsl->l);

  hsl->l += change * s;
}

static PaletteFunctorT PalEffects[] = {
  CyclicHue,
  PulsingSaturation,
  PulsingLuminosity
};

static void PaletteEffect(int frameNumber, PaletteT *src, PaletteT *dst,
                          PaletteFunctorT *func)
{
  while (src) {
    int i;

    if (!(dst && func && src->count == dst->count && src->start == dst->start))
      break;

    for (i = 0; i < src->count; i++) {
      HSL hsl;

      RGB2HSL(&src->colors[i], &hsl);
      (*func)(frameNumber, &hsl);
      HSL2RGB(&hsl, &dst->colors[i]);
    }

    src = src->next;
    dst = dst->next;
    func++;
  }
}

static void RenderTunnel(int frameNumber) {
  UVMapSetOffset(tunnelMap, 0, frameNumber);
  UVMapSetTexture(tunnelMap, texture);
  PROFILE (UVMapRender)
    UVMapRender(tunnelMap, canvas);

  PROFILE (PixBufBlit)
    PixBufBlit(canvas, 0, 137, whelpz, NULL);

  {
    float rad = (float)(frameNumber % 150) / 150 * 2 * M_PI;
    int w = sin(rad) * 80;
    PROFILE (PixBufBlitScaled)
      PixBufBlitScaled(canvas, 200 + (80 - abs(w)) / 2, 20, w, 33, credits);
  }

  PROFILE (c2p)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void Loop() {
  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    PaletteEffect(frameNumber, texturePal, effectPal, PalEffects);
    LoadPalette(effectPal);

    RenderTunnel(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while (ReadLoopEvent() != LOOP_EXIT);
}

EffectT Effect = { "Tunnel", Load, UnLoad, Init, Kill, Loop };
