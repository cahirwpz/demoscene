#include "std/debug.h"
#include "std/math.h"
#include "std/fastmath.h"
#include "std/memory.h"
#include "std/resource.h"

#include "engine/ms3d.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

#include "gfx/blit.h"
#include "uvmap/render.h"
#include "uvmap/scaling.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const int H_RAYS = 41;
const int V_RAYS = 33;

/*
 * Set up resources.
 */
void AddInitialResources() {
  static Vector3D View[3] = {
    { -0.5f,  0.333333f, 0.5f }, 
    {  0.5f,  0.333333f, 0.5f },
    { -0.5f, -0.333333f, 0.5f }
  };

  static Vector3D ViewTransformed[3];

  ResAddStatic("View", View);
  ResAddStatic("ViewTransformed", ViewTransformed);
  ResAdd("Texture", NewPixBufFromFile("data/texture-shades.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture-shades.pal"));
  ResAdd("ColorMap", NewPixBufFromFile("data/texture-shades-map.8"));
  ResAdd("Shades", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
  ResAdd("ms3d", NewMatrixStack3D());
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));
  ResAdd("SmallMap", NewUVMap(H_RAYS, V_RAYS, UV_ACCURATE, 256, 256));
  ResAdd("Map", NewUVMap(WIDTH, HEIGHT, UV_NORMAL, 256, 256));
  
  UVMapSetTexture(R_("Map"), R_("Texture"));
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
  LoadPalette(R_("TexturePal"));
  PixBufClear(R_("Canvas"));
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
}

/*
 * Effect rendering functions.
 */
void CalculateView(int frameNumber, Vector3D *view) {
  Matrix3D transformation;

  LoadRotation3D(&transformation,
                 0.0f,
                 frameNumber * 0.75f,
                 frameNumber * 0.5f);

  Transform3D(view, R_("View"), 3, &transformation);

  V3D_Sub(&view[1], &view[1], &view[0]);
  V3D_Sub(&view[2], &view[2], &view[0]);
}

void RaytraceTunnel(UVMapT *map, Vector3D *view) {
  Vector3D ray = view[0];
  Vector3D dp = view[1];
  Vector3D dq = view[2];

  size_t h = map->height;
  size_t i = 0;

  V3D_Scale(&dp, &view[1], 1.0f / (int)(map->width - 1));
  V3D_Scale(&dq, &view[2], 1.0f / (int)(map->height - 1));

  do {
    Vector3D leftRay = ray;
    size_t w = map->width;

    do {
      float t = FastInvSqrt(ray.x * ray.x + ray.y * ray.y);

      Vector3D intersection = { t * ray.x, t * ray.y, t * ray.z };

      float a = FastAtan2(intersection.x, intersection.y);
      float u = a / (2 * M_PI);
      float v = intersection.z / 8.0f;

      UVMapSet(map, i++, u, v);

      V3D_Add(&ray, &ray, &dp);
    } while (--w);

    V3D_Add(&ray, &leftRay, &dq);
  } while (--h);
}

static void RenderShadeMap(PixBufT *shades, int16_t *map) {
  int n = shades->width * shades->height;
  uint8_t *dst = shades->data;

  while (n--) {
    int value = *map++;

    if (value < 0)
      value = -value;
    if (value < 128)
      value = 128;
    if (value > 255)
      value = 255;

    *dst++ = value;
  }
}

void RenderEffect(int frameNumber) {
  Vector3D *view = R_("ViewTransformed");
  UVMapT *smallMap = R_("SmallMap");
  UVMapT *map = R_("Map");
  PixBufT *canvas = R_("Canvas");
  PixBufT *shades = R_("Shades");

  CalculateView(frameNumber, view);
  RaytraceTunnel(smallMap, view);
  UVMapScale8x(map, smallMap);
  UVMapSetTexture(map, R_("Texture"));
  UVMapSetOffset(map, 0, frameNumber);
  UVMapRender(map, canvas);

  RenderShadeMap(shades, map->map.normal.v);

  PixBufSetColorMap(shades, R_("ColorMap"), 0);
  PixBufSetBlitMode(shades, BLIT_COLOR_MAP);
  PixBufBlit(canvas, 0, 0, shades, NULL);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  } while (ReadLoopEvent() != LOOP_EXIT);
}
