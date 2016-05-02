#include "engine/ms3d.h"
#include "engine/scene.h"
#include "gfx/blit.h"
#include "gfx/palette.h"
#include "gfx/png.h"
#include "tools/gradient.h"
#include "system/fileio.h"

#include "uvmap/misc.h"
#include "uvmap/render.h"

#include "startup.h"

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

static void Load() {
  LoadPngImage(&texture, &texturePal, "data/texture-shades.png");
  LoadPngImage(&colorMap, NULL, "data/texture-shades-map.png");
  mesh = NewMeshFromFile("data/shattered_ball.robj");

  CenterMeshPosition(mesh);
  CalculateSurfaceNormals(mesh);
  NormalizeMeshSize(mesh);

  RenderAllFaces = false;
  RenderMode = RENDER_FLAT_SHADING;
}

static void UnLoad() {
  MemUnref(mesh);
  MemUnref(texture);
  MemUnref(texturePal);
  MemUnref(colorMap);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);

  uvmap = NewUVMap(WIDTH, HEIGHT, UV_FAST, 256, 256);
  UVMapGenerate2(uvmap);
  UVMapSetTexture(uvmap, texture);

  orig = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);
  PixBufBlit(orig, 0, 0,
             NewPixBufWrapper(WIDTH, HEIGHT, uvmap->map.fast.u), NULL);

  scene = NewScene();
  SceneAddObject(scene, NewSceneObject("Object", mesh));

  shades = NewPixBuf(PIXBUF_GRAY, WIDTH, HEIGHT);

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(texturePal);
}

static void Kill() {
  KillDisplay();

  MemUnref(uvmap);
  MemUnref(shades);
  MemUnref(canvas);
  MemUnref(scene);
  MemUnref(orig);
}

static int effectNum = 1;

static void RenderEffect(int frameNumber) {
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

  if (effectNum == 0) {
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

static void Loop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_NEXT)
      effectNum = (effectNum + 1) % 2;
    if (event == LOOP_PREV)
      effectNum = (effectNum - 1) % 2;

    RenderEffect(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}

EffectT Effect = { "FlatShade", Load, UnLoad, Init, Kill, Loop };
