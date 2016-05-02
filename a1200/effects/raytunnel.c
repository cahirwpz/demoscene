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
#include "gfx/png.h"
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

static PixBufT *shades;
static PixBufT *canvas;
static PixBufT *texture;
static PaletteT *texturePal;
static PixBufT *colorMap;
static UVMapT *smallMap;
static UVMapT *uvmap;
static uint8_t *lightFunc;

static uint8_t *CalculateLightFunc() {
  uint8_t *array = NewTable(uint8_t, 65536);
  int i;

  for (i = -32768; i < 32768; i++) {
    int value = i;

    if (value < 0)
      value = -value;
    if (value < 128)
      value = 128;
    else if (value > 255)
      value = 255;

    array[(uint16_t)i] = value;
  }

  return array;
}

void AcquireResources() {
  LoadPngImage(&texture, &texturePal, "data/texture-shades.png");
  LoadPngImage(&colorMap, NULL, "data/texture-shades-map.png");
  shades = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  smallMap = NewUVMap(H_RAYS, V_RAYS, UV_ACCURATE, 256, 256);
  uvmap = NewUVMap(WIDTH, HEIGHT, UV_NORMAL, 256, 256);
  lightFunc = CalculateLightFunc();
  
  UVMapSetTexture(uvmap, texture);
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  LoadPalette(texturePal);
  PixBufClear(canvas);
}

void TearDownEffect() {
}

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
    RaycastTunnel(smallMap, CameraView.Transformed);

  PROFILE (UVMapScale8x)
    UVMapScale8x(uvmap, smallMap);

  UVMapSetTexture(uvmap, texture);
  UVMapSetOffset(uvmap, 0, frameNumber);

  PROFILE (UVMapRender)
    UVMapRender(uvmap, canvas);

  PROFILE (RenderShadeMap)
    RenderShadeMap(shades, uvmap->map.normal.v);

  PixBufSetColorMap(shades, colorMap);
  PixBufSetBlitMode(shades, BLIT_COLOR_MAP);

  PROFILE (BlitShadeMap)
    PixBufBlit(canvas, 0, 0, shades, NULL);

  PROFILE (ChunkyToPlanar)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

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
