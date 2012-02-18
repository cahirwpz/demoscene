#include <math.h>

#include "p61/p61.h"

#include "gfx/blit.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"
#include "std/resource.h"

#include "system/c2p.h"
#include "system/debug.h"
#include "system/display.h"
#include "system/memory.h"
#include "system/vblank.h"

#include "frame_tools.h"
#include "distortion.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static CanvasT *Canvas;
static DistortionMapT *TunnelMap;
static PixBufT *Texture;
static PixBufT *Credits;
static PixBufT *Whelpz;
static PaletteT *OrigPal;
static PaletteT *EffectPal;

/*
 * Set up display function.
 */
bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  Texture = GetResource("txt_img");
  Credits = GetResource("code_img");
  Whelpz = GetResource("whelpz_img");
  TunnelMap = GetResource("tunnel_map");

  Canvas = NewCanvas(WIDTH, HEIGHT);

  {
    PaletteT *texturePal = GetResource("txt_pal");
    PaletteT *creditsPal = GetResource("code_pal");
    PaletteT *whelpzPal = GetResource("whelpz_pal");

    LinkPalettes(3, texturePal, whelpzPal, creditsPal);
    LoadPalette(texturePal);

    OrigPal = texturePal;
    EffectPal = CopyPalette(OrigPal);

    PixBufRemap(Credits, creditsPal);
    PixBufRemap(Whelpz, whelpzPal);
  }

  P61_Init(GetResource("module"), NULL, NULL);
  P61_ControlBlock.Play = 1;
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  P61_End();

  DeletePalette(EffectPal);
  UnlinkPalettes(OrigPal);
  DeleteCanvas(Canvas);
}

/*
 * Effect rendering functions.
 */
typedef void (*PaletteFunctorT)(int frameNumber, ColorVectorT *hsl);

void CyclicHue(int frameNumber, ColorVectorT *hsl) {
  hsl->h += (float)(frameNumber & 255) / 256.0f;

  if (hsl->h > 1.0f)
    hsl->h -= 1.0f;
}

void PulsingSaturation(int frameNumber, ColorVectorT *hsl) {
  float s = sin(frameNumber * 3.14159265f / 50.0f) * 1.00f;
  float change = (s > 0.0f) ? (1.0f - hsl->s) : (hsl->s);

  hsl->s += change * s;
}

void PulsingLuminosity(int frameNumber, ColorVectorT *hsl) {
  float s = sin(frameNumber * 3.14159265f / 12.5f) * 0.66f;
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
      ColorVectorT hsl;

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
  RenderDistortion(Canvas, TunnelMap, Texture, 0, frameNumber);

  PixBufBlitTransparent(Canvas->pixbuf, 200, 20, Credits);
  PixBufBlitTransparent(Canvas->pixbuf, 0, 137, Whelpz);
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(Canvas), GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  while (GetVBlankCounter() < 50*60*2) {
    int frameNumber = GetVBlankCounter();

    PaletteEffect(frameNumber, OrigPal, EffectPal, PalEffects);
    LoadPalette(EffectPal);

    RenderTunnel(frameNumber);
    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  }
}
