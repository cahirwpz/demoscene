#include "std/debug.h"
#include "std/math.h"
#include "std/fastmath.h"
#include "std/memory.h"
#include "std/resource.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/vblank.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

#include "engine/matrix3d.h"
#include "gfx/blit.h"
#include "uvmap/raycast.h"
#include "uvmap/render.h"
#include "uvmap/scaling.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

const int H_RAYS = 41;
const int V_RAYS = 33;

typedef struct {
  Vector3D Nominal[3];
  Vector3D Transformed[3];
} CameraViewT;

static CameraViewT CameraView = {
  .Nominal = {
    { -0.5f,  0.333333f, 0.5f },
    {  0.5f,  0.333333f, 0.5f },
    { -0.5f, -0.333333f, 0.5f }
  }
};

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Texture", NewPixBufFromFile("data/texture-shades.8"));
  ResAdd("TexturePal", NewPaletteFromFile("data/texture-shades.pal"));
  ResAdd("ColorMap", NewPixBufFromFile("data/texture-shades-map.8"));
  ResAdd("Shades", NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT));
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
void RaycastCalculateView(int frameNumber) {
  Matrix3D transformation;
  Vector3D *transformed = CameraView.Transformed;
  Vector3D *nominal = CameraView.Nominal;

  LoadRotation3D(&transformation,
                 0.0f,
                 frameNumber * 0.75f,
                 frameNumber * 0.5f);

  Transform3D(transformed, nominal, 3, &transformation);

  V3D_Sub(&transformed[1], &transformed[1], &transformed[0]);
  V3D_Sub(&transformed[2], &transformed[2], &transformed[0]);
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
  UVMapT *smallMap = R_("SmallMap");
  UVMapT *map = R_("Map");
  PixBufT *canvas = R_("Canvas");
  PixBufT *shades = R_("Shades");

  RaycastCalculateView(frameNumber);
  RaycastTunnel(smallMap, CameraView.Transformed);
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
