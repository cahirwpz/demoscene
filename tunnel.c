#include <math.h>

#include "p61/p61.h"

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/hsl.h"
#include "gfx/line.h"
#include "gfx/palette.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/input.h"
#include "system/vblank.h"

#include "frame_tools.h"
#include "distortion.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Module", ReadFileToChipMem("data/tempest-acidjazzed_evening.p61"));
  ResAdd("Texture", NewPixBufFromFile("data/texture-01.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture-01.pal"));
  ResAdd("CreditsImg", NewPixBufFromFile("data/code.8"));
  ResAdd("CreditsPal", NewPaletteFromFile("data/code.pal"));
  ResAdd("WhelpzImg", NewPixBufFromFile("data/whelpz.8"));
  ResAdd("WhelpzPal", NewPaletteFromFile("data/whelpz.pal"));
  ResAdd("TunnelMap", NewDistortionMap(WIDTH, HEIGHT));
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));
}

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
  GenerateTunnel(R_("TunnelMap"), 8192, WIDTH/2, HEIGHT/2);

  LinkPalettes(R_("TexturePal"), R_("WhelpzPal"), R_("CreditsPal"), NULL);
  LoadPalette(R_("TexturePal"));

  ResAdd("EffectPal", CopyPalette(R_("TexturePal")));

  PixBufRemap(R_("CreditsImg"), R_("CreditsPal"));
  PixBufRemap(R_("WhelpzImg"), R_("WhelpzPal"));

  P61_Init(R_("Module"), NULL, NULL);
  P61_ControlBlock.Play = 1;
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  P61_End();

  UnlinkPalettes(R_("TexturePal"));
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
  CanvasT *canvas = R_("Canvas");

  RenderDistortion(R_("TunnelMap"), canvas, R_("Texture"), 0, frameNumber);

  PixBufBlitTransparent(canvas->pixbuf, 200, 20, R_("CreditsImg"));
  PixBufBlitTransparent(canvas->pixbuf, 0, 137, R_("WhelpzImg"));
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(R_("Canvas")),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  while (GetVBlankCounter() < 50*60*2) {
    int frameNumber = GetVBlankCounter();

    EventQueueReset();

    PaletteEffect(frameNumber, R_("TexturePal"), R_("EffectPal"), PalEffects);
    LoadPalette(R_("EffectPal"));

    RenderTunnel(frameNumber);
    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  }
}
