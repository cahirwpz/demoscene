#include "std/debug.h"
#include "std/math.h"
#include "std/fastmath.h"
#include "std/memory.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/timer.h"
#include "system/vblank.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

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
static PixBufT *Texture;
static PaletteT *TexturePal;
static PixBufT *ColorMap;
static PixBufT *Shades;
static PixBufT *Canvas;
static UVMapT *SmallMap;
static UVMapT *Map;
static uint8_t *LightFunc;

static uint8_t *CalculateLightFunc() {
  uint8_t *lightFunc = NewTable(uint8_t, 65536);
  int i;

  for (i = -32768; i < 32768; i++) {
    int value = i;

    if (value < 0)
      value = -value;
    if (value < 128)
      value = 128;
    else if (value > 255)
      value = 255;

    lightFunc[(uint16_t)i] = value;
  }

  return lightFunc;
}

void AddInitialResources() {
  Texture = NewPixBufFromFile("data/texture-shades.8");
  TexturePal = NewPaletteFromFile("data/texture-shades.pal");
  ColorMap = NewPixBufFromFile("data/texture-shades-map.8");
  Shades = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  Canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  SmallMap = NewUVMap(H_RAYS, V_RAYS, UV_ACCURATE, 256, 256);
  Map = NewUVMap(WIDTH, HEIGHT, UV_NORMAL, 256, 256);
  LightFunc = CalculateLightFunc();
  
  UVMapSetTexture(Map, Texture);
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
  LoadPalette(TexturePal);
  PixBufClear(Canvas);
  StartProfiling();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  StopProfiling();
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

static void RenderShadeMap(PixBufT *shades, uint16_t *map) {
  uint8_t *lightFunc = LightFunc;
  int n = shades->width * shades->height;
  uint8_t *dst = shades->data;
  uint32_t offset;

  do {
    offset = *map++;
    *dst++ = lightFunc[offset];
  } while (--n);
}

void RenderEffect(int frameNumber) {
  RaycastCalculateView(frameNumber);

  PROFILE(RaycastTunnel)
    RaycastTunnel(SmallMap, CameraView.Transformed);

  PROFILE (UVMapScale8x)
    UVMapScale8x(Map, SmallMap);

  UVMapSetTexture(Map, Texture);
  UVMapSetOffset(Map, 0, frameNumber);

  PROFILE (UVMapRender)
    UVMapRender(Map, Canvas);

  PROFILE (RenderShadeMap)
    RenderShadeMap(Shades, Map->map.normal.v);

  PixBufSetColorMap(Shades, ColorMap);
  PixBufSetBlitMode(Shades, BLIT_COLOR_MAP);

  PROFILE (BlitShadeMap)
    PixBufBlit(Canvas, 0, 0, Shades, NULL);

  PROFILE (ChunkyToPlanar)
    c2p1x1_8_c5_bm(Canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
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
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while (ReadLoopEvent() != LOOP_EXIT);
}
