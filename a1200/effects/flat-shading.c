#include "std/debug.h"
#include "std/memory.h"

#include "engine/ms3d.h"
#include "engine/scene.h"
#include "gfx/blit.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/frame.h"
#include "tools/gradient.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/fileio.h"
#include "system/vblank.h"

#include "uvmap/misc.h"
#include "uvmap/render.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static MeshT *mesh;
static PixBufT *texture;
static PaletteT *texturePal;
static UVMapT *uvmap;
static PixBufT *colorMap;
static PixBufT *canvas;
static PixBufT *shades;
static PixBufT *orig;
static SceneT *scene;

void AcquireResources() {
  LoadPngImage(&texture, &texturePal, "data/texture-shades.png");
  LoadPngImage(&colorMap, NULL, "data/texture-shades-map.png");
  mesh = NewMeshFromFile("data/shattered_ball.robj");

  CenterMeshPosition(mesh);
  CalculateSurfaceNormals(mesh);
  NormalizeMeshSize(mesh);

  RenderAllFaces = false;
  RenderMode = RENDER_FLAT_SHADING;
}

void ReleaseResources() {
  MemUnref(mesh);
  MemUnref(texture);
  MemUnref(texturePal);
  MemUnref(colorMap);
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  PixBufT *map = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);

  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  shades = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  scene = NewScene();
  orig = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);

  LoadPalette(texturePal);

  UVMapGenerate2(uvmap);
  UVMapSetTexture(uvmap, texture);

  PixBufBlit(orig, 0, 0, map, NULL);

  SceneAddObject(scene, NewSceneObject("Object", mesh));
}

void TearDownEffect() {
  MemUnref(uvmap);
  MemUnref(shades);
  MemUnref(canvas);
  MemUnref(scene);
  MemUnref(orig);
}

static int EffectNum = 1;

void RenderEffect(int frameNumber) {
  PixBufT *umap;

  int du = 2 * frameNumber;
  int dv = 4 * frameNumber;

  {
    MatrixStack3D *ms = GetObjectTranslation(scene, "Object");

    StackReset(ms);
    PushScaling3D(ms, 1.25f, 1.25f, 1.25f);
    PushRotation3D(ms, 0, (float)(-frameNumber * 2), frameNumber);
    PushTranslation3D(ms, 0.0f, 0.0f, -2.0f);
  }

  if (EffectNum == 0) {
    umap = NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u);

    PROFILE(PixBufClear)
      PixBufClear(shades);
    PROFILE(RenderScene)
      RenderScene(scene, shades);

    PixBufSetBlitMode(shades, BLIT_ADDITIVE);
    PixBufBlit(umap, 0, 0, orig, NULL);
    PixBufBlit(umap, 0, 0, shades, NULL);

    UVMapSetOffset(uvmap, du, dv);
    UVMapRender(uvmap, canvas);

    MemUnref(umap);
  } else {
    UVMapSetOffset(uvmap, du, dv);
    UVMapRender(uvmap, canvas);

    PROFILE(PixBufClear)
      PixBufClear(shades);
    PROFILE(RenderScene)
      RenderScene(scene, shades);

    PixBufSetColorMap(shades, colorMap);
    PixBufSetBlitMode(shades, BLIT_COLOR_MAP);

    PixBufBlit(canvas, 0, 0, shades, NULL);
  }

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      EffectNum = (EffectNum + 1) % 2;
    if (event == LOOP_PREV)
      EffectNum = (EffectNum - 1) % 2;

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
