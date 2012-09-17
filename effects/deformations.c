#include <math.h>

#include "p61/p61.h"

#include "std/debug.h"
#include "std/memory.h"
#include "std/math.h"
#include "std/resource.h"

#include "distort/generate.h"
#include "gfx/palette.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

float smoothstep(float edge0, float edge1, float x) {
  // Scale, bias and saturate x to 0..1 range
  x = clampf((x - edge0) / (edge1 - edge0)); 
  // Evaluate polynomial
  return x * x * (3.0f - 2.0f * x);
}

GenerateMiscDistortion(0,
                       0.3f / (r + 0.5f * x),
                       3.0f * a / M_PI);

GenerateMiscDistortion(1,
                       x * cos(2.0f * r) - y * sin(2.0f * r),
                       y * cos(2.0f * r) + x * sin(2.0f * r));

GenerateMiscDistortion(2,
                       pow(r, 0.33f),
                       a / M_PI + r);

GenerateMiscDistortion(3,
                       cos(a) / (3 * r),
                       sin(a) / (3 * r));

GenerateMiscDistortion(4,
                       0.04f * y + 0.06f * cos(a * 3) / r,
                       0.04f * x + 0.06f * sin(a * 3) / r);

GenerateMiscDistortion(5,
                       0.1f * y / (0.11f + r * 0.15f),
                       0.1f * x / (0.11f + r * 0.15f));

GenerateMiscDistortion(6,
                       0.5f * a / M_PI + 0.25f * r,
                       pow(r, 0.25f));

GenerateMiscDistortion(7,
                       0.5f * a / M_PI,
                       sin(5.0f * r));

GenerateMiscDistortion(8,
                       3.0f * a / M_PI,
                       sin(6.0f * r) + 0.5f * cos(7.0f * a));

GenerateMiscDistortion(9,
                       x * log(0.5f * r * r),
                       y * log(0.5f * r * r));

GenerateMiscDistortion(10,
                       8 * x * (1.5-r) * (1.5-r),
                       8 * y * (1.5-r) * (1.5-r));

static const int maps = 11;
static int lastMap = -1;

void ChangeMap(int newMap) {
  while (newMap < 0)
    newMap += maps;

  newMap = newMap % maps;

  if (newMap != lastMap) {
    DistortionMapT *map = R_("Map");

    switch (newMap) {
      case 0:
        GenerateMisc0Distortion(map);
        break;
      case 1:
        GenerateMisc1Distortion(map);
        break;
      case 2:
        GenerateMisc2Distortion(map);
        break;
      case 3:
        GenerateMisc3Distortion(map);
        break;
      case 4:
        GenerateMisc4Distortion(map);
        break;
      case 5:
        GenerateMisc5Distortion(map);
        break;
      case 6:
        GenerateMisc6Distortion(map);
        break;
      case 7:
        GenerateMisc7Distortion(map);
        break;
      case 8:
        GenerateMisc8Distortion(map);
        break;
      case 9:
        GenerateMisc9Distortion(map);
        break;
      case 10:
        GenerateMisc10Distortion(map);
        break;
      default:
        break;
    }

    lastMap = newMap;
  }
}

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Texture", NewPixBufFromFile("data/texture.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture.pal"));
  ResAdd("Map", NewDistortionMap(WIDTH, HEIGHT, DMAP_OPTIMIZED, 256, 256));
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));
}

/*
 * Set up display function.
 */
bool SetupDisplay() {
  ChangeMap(0);
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

/*
 * Set up effect function.
 */
void SetupEffect() {
  LoadPalette(R_("TexturePal"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
void RenderChunky(int frameNumber) {
  CanvasT *canvas = R_("Canvas");

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  RenderDistortion(R_("Map"), canvas, R_("Texture"), du, dv);

  c2p1x1_8_c5_bm(GetCanvasPixelData(canvas),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      ChangeMap(lastMap + 1);
    if (event == LOOP_PREV)
      ChangeMap(lastMap - 1);

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
