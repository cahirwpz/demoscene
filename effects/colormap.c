#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "gfx/blit.h"
#include "gfx/canvas.h"
#include "tools/frame.h"
#include "tools/gradient.h"
#include "tools/loopevent.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));
  ResAdd("Map1", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Map2", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("Image", NewPixBufFromFile("data/samkaat-absinthe.8"));
  ResAdd("ImagePal", NewPaletteFromFile("data/samkaat-absinthe.pal"));
  ResAdd("Darken", NewPixBufFromFile("data/samkaat-absinthe-darken.8"));
  ResAdd("Lighten", NewPixBufFromFile("data/samkaat-absinthe-lighten.8"));
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
  {
    FLineT line;
    FPointT pa = { 0, 0 };
    FPointT pb = { WIDTH, HEIGHT };

    FLineInitFromPoints(&line, &pa, &pb);
    LinearGradient(R_("Map1"), &line);
  }

  {
    FPointT center = { WIDTH / 2, HEIGHT / 2 };
    CircularGradient(R_("Map2"), &center);
  }

  LoadPalette(R_("ImagePal"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
void Emerge(PixBufT *dst, PixBufT *image, PixBufT *map, int change, int threshold) {
  uint8_t *d = dst->data;
  uint8_t *s = image->data;
  uint8_t *m = map->data;
  uint8_t bgcol = image->baseColor;

  int pixels = WIDTH * HEIGHT;

  do {
    int shade = (*m++) + change;

    if (shade > threshold) {
      *d++ = *s++;
    } else {
      *d++ = bgcol;
      s++;
    }
  } while (pixels-- > 0);
}

void Shade(PixBufT *dst, PixBufT *image, PixBufT *map, PixBufT *colorMap, int change) {
  uint8_t *s = image->data;
  uint8_t *d = dst->data;
  uint8_t *m = map->data;
  uint8_t *c = colorMap->data;

  int pixels = WIDTH * HEIGHT;

  do {
    int pixel = *s++;
    int shade = (*m++) + change;

    if (shade < 0)
      shade = 0;
    if (shade > 255)
      shade = 255;

    *d++ = c[pixel * 256 + shade];
  } while (pixels-- > 0);
}

static int Effect = 0;
static const int LastEffect = 6;

void RenderChunky(int frameNumber) {
  CanvasT *canvas = R_("Canvas");
  PixBufT *image = R_("Image");

  int change = (frameNumber * 2) % 256;

  switch (Effect) {
    case 0:
      Emerge(canvas->pixbuf, image, R_("Map1"), change, 192);
      break;
    
    case 1:
      Shade(canvas->pixbuf, image, R_("Map1"), R_("Lighten"), change);
      break;

    case 2:
      Shade(canvas->pixbuf, image, R_("Map1"), R_("Darken"), change - 64);
      break;

    case 3:
      Emerge(canvas->pixbuf, image, R_("Map2"), change, 192);
      break;
    
    case 4:
      Shade(canvas->pixbuf, image, R_("Map2"), R_("Lighten"), change);
      break;

    case 5:
      Shade(canvas->pixbuf, image, R_("Map2"), R_("Darken"), change - 64);
      break;

    default:
      break;
  }

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
      Effect = (Effect + 1) % LastEffect;
    if (event == LOOP_PREV) {
      Effect--;
      if (Effect < 0)
        Effect += LastEffect;
    }

    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
