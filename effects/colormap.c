#include <stdlib.h>

#include "std/debug.h"
#include "std/fastmath.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/resource.h"

#include "distort/scaling.h"
#include "engine/plane.h"
#include "engine/sphere.h"
#include "engine/vector3d.h"
#include "gfx/blit.h"
#include "gfx/canvas.h"
#include "gfx/ellipse.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "txtgen/procedural.h"

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
typedef struct FLine {
  float x, y, dx, dy;
  float invNormalLength;
} FLineT;

void FLineInitFromPoints(FLineT *line, FPointT *p0, FPointT *p1) {
  float dx = p1->x - p0->x;
  float dy = p1->y - p0->y;

  line->x = p0->x;
  line->y = p0->y;
  line->dx = dx;
  line->dy = dy;
  line->invNormalLength = FastInvSqrt(dx * dx + dy * dy);
}

static inline float PointToLineDistance(const FLineT *line, float px, float py) {
  return fabsf((px - line->x) * line->dy - (py - line->x) * line->dx) *
    line->invNormalLength;
}

void RenderLinearGradient(PixBufT *map, const FLineT *line) {
  uint8_t *data = map->data;
  int x, y;

  for (y = 0; y < map->height; y++) {
    for (x = 0; x < map->width; x++) {
      int d = (uint8_t)lroundf(PointToLineDistance(line, (float)x, (float)y));

      if (d < 0)
        d = 0;
      if (d >= 255)
        d = 255;

      *data++ = d;
    }
  }
}

void RenderCircularGradient(PixBufT *map, const FPointT *point) {
  uint8_t *data = map->data;
  int x, y;

  for (y = 0; y < map->height; y++) {
    for (x = 0; x < map->width; x++) {
      int xc = x - point->x;
      int yc = y - point->y;
      int d = (uint8_t)lroundf(sqrtf((float)(xc * xc + yc * yc)));

      if (d < 0)
        d = 0;
      if (d >= 255)
        d = 255;

      *data++ = d;
    }
  }
}

void SetupEffect() {
  {
    FLineT line;
    FPointT pa = { 0, 0 };
    FPointT pb = { WIDTH, HEIGHT };

    FLineInitFromPoints(&line, &pa, &pb);
    RenderLinearGradient(R_("Map1"), &line);
  }

  {
    FPointT center = { WIDTH / 2, HEIGHT / 2 };
    RenderCircularGradient(R_("Map2"), &center);
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
