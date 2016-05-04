#include "gfx/palette.h"
#include "gfx/png.h"
#include "system/fileio.h"
#include "uvmap/common.h"

#include "startup.h"

static const int WIDTH = 320;
static const int HEIGHT = 256;
static const int DEPTH = 8;

static PixBufT *canvas;
static PixBufT *heightMap;
static UVMapT *bumpMap;
static PixBufT *reflectionMap;

void RenderBumpMapOptimized(int16_t *mapU asm("a0"), int16_t *mapV asm("a1"),
                            uint8_t *rmap asm("a2"), uint8_t *dst asm("a3"),
                            int16_t width asm("d0"), int16_t height asm("d1"),
                            int16_t light_x asm("d2"), int16_t light_y asm("d3"));

static PixBufT *CreateReflectionMap() {
  PixBufT *reflectionMap = NewPixBuf(PIXBUF_GRAY, 256, 256);
  uint8_t *map = reflectionMap->data;
  int x, y;

  for (y = 0; y < 256; y++) {
    for (x = 0; x < 256; x++) {
      float fx = (x - 128) / 128.0f;
      float fy = (y - 128) / 128.0f;
      float fz = 1.0f - sqrt(fx * fx + fy * fy);

      fz *= 255.0f;

      if (fz < 0.0f)
        fz = 0.0f;

      *map++ = fz;
    }
  }

  return reflectionMap;
}

static void CalculateBumpMap(UVMapT *bumpMap, PixBufT *heightMap) {
  uint8_t *src = heightMap->data;
  int16_t *mapU = bumpMap->map.normal.u;
  int16_t *mapV = bumpMap->map.normal.v;
  int width = heightMap->width;
  int height = heightMap->height;
  int x, y;

  mapU += width + 1;
  mapV += width + 1;

  for (y = 1; y < height - 1; y++) {
    for (x = 1; x < width - 1; x++) {
      *mapU++ = src[y * width + x + 1] - src[y * width + x];
      *mapV++ = src[y * width + x] - src[(y - 1) * width + x];
    }

    mapU += 2;
    mapV += 2;
  }
}

static void
RenderBumpMap(PixBufT *canvas, UVMapT *bumpMap, PixBufT *reflectionMap,
              int light_x, int light_y)
{
  uint8_t *dst = canvas->data;
  int16_t *mapU = bumpMap->map.normal.u;
  int16_t *mapV = bumpMap->map.normal.v;
  uint8_t *rmap = reflectionMap->data;
  int16_t width = canvas->width;
  int16_t height = canvas->height;

  RenderBumpMapOptimized(mapU, mapV, rmap, dst, width, height, light_x, light_y);

#if 0
  int16_t x, y;

  mapU += width + 1;
  mapV += width + 1;
  dst += width + 1;

  for (y = 1; y < height - 1; y++) {
    int16_t diff_y = y - light_y;

    for (x = 1; x < width - 1; x++) {
      int16_t diff_x = x - light_x;

      int16_t normal_x = (*mapU++) + diff_x;
      int16_t normal_y = (*mapV++) + diff_y;

      if (normal_x & 0xff00)
        normal_x = 0;
      if (normal_y & 0xff00)
        normal_y = 0;

      *dst++ = rmap[(uint8_t)normal_y * 256 + (uint8_t)normal_x];
    }

    dst += 2;
    mapU += 2;
    mapV += 2;
  }
#endif
}

static void Load() {
  LoadPngImage(&heightMap, NULL, "data/samkaat-absinthe.png");
}

static void UnLoad() {
  MemUnref(heightMap);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  bumpMap = NewUVMap(WIDTH, HEIGHT, UV_NORMAL, 256, 256);
  CalculateBumpMap(bumpMap, heightMap);

  reflectionMap = CreateReflectionMap();

  InitDisplay(WIDTH, HEIGHT, DEPTH);
}

static void Kill() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(bumpMap);
  MemUnref(reflectionMap);
}

static void Render(int frameNumber) {
  PROFILE(BumpMap) {
    RenderBumpMap(canvas, bumpMap, reflectionMap,
                  64 * sin((frameNumber & 255) * M_PI / 128) + 32, 0);
  }
  PROFILE(C2P)
    c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

EffectT Effect = { "BumpMap", Load, UnLoad, Init, Kill, Render };
