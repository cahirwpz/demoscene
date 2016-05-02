#include <math.h>

#include "std/debug.h"
#include "std/memory.h"

#include "gfx/blit.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "uvmap/generate.h"
#include "uvmap/render.h"

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

void AcquireResources() {
  LoadPngImage(&texture, &texturePal, "data/texture-01.png");
  LoadPngImage(&credits, &creditsPal, "data/code.png");
  LoadPngImage(&whelpz, &whelpzPal, "data/whelpz.png");
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  static TunnelPetalsT petals = { 3, 0.333333f, 0.33333f };

  tunnelMap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  UVMapGenerateTunnel(tunnelMap, 32.0f, 1, 16.0f / 9.0f, 0.5f, 0.5f, &petals);

  LinkPalettes(texturePal, whelpzPal, creditsPal, NULL);
  LoadPalette(texturePal);

  effectPal = MemClone(texturePal);

  PixBufRemap(credits, creditsPal);
  PixBufRemap(whelpz, whelpzPal);
  PixBufSetBlitMode(credits, BLIT_TRANSPARENT);
  PixBufSetBlitMode(whelpz, BLIT_TRANSPARENT);
}

void TearDownEffect() {
  UnlinkPalettes(texturePal);
}

typedef void (*PaletteFunctorT)(int frameNumber, HSL *hsl);

void CyclicHue(int frameNumber, HSL *hsl) {
  hsl->h += (float)(frameNumber & 255) / 256.0f;

  if (hsl->h > 1.0f)
    hsl->h -= 1.0f;
}

void PulsingSaturation(int frameNumber, HSL *hsl) {
  float s = sin(frameNumber * M_PI / 50.0f) * 1.00f;
  float change = (s > 0.0f) ? (1.0f - hsl->s) : (hsl->s);

  hsl->s += change * s;
}

void PulsingLuminosity(int frameNumber, HSL *hsl) {
  float s = sin(frameNumber * M_PI / 12.5f) * 0.66f;
  float change = (s > 0.0f) ? (1.0f - hsl->l) : (hsl->l);

  hsl->l += change * s;
}

static PaletteFunctorT PalEffects[] = {
  CyclicHue,
  PulsingSaturation,
  PulsingLuminosity
};

void PaletteEffect(int frameNumber, PaletteT *src, PaletteT *dst, PaletteFunctorT *func) {
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

void RenderTunnel(int frameNumber) {
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

/*
 * Main loop.
 */
void MainLoop() {
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
