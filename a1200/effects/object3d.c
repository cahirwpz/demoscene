#include <math.h>

#include "engine/ms3d.h"
#include "engine/object.h"
#include "engine/scene.h"
#include "gfx/png.h"
#include "system/input.h"

#include "startup.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static SceneT *scene;
static MeshT *mesh;
static PaletteT *palette;

static void Load() {
#if 0
  ResAdd("Mesh", NewMeshFromFile("data/shattered_ball.robj"));
  ResAdd("ColorMap", NewPixBufFromFile("data/shattered_ball_cmap.8"));
  ResAdd("Palette", NewPaletteFromFile("data/shattered_ball_cmap.pal"));
#else
  LoadPngImage(NULL, &palette, "data/wecan_logo_cmap.png");
  mesh = NewMeshFromFile("data/wecan_logo.robj");
#endif

  CalculateSurfaceNormals(mesh);
  NormalizeMeshSize(mesh);
  MeshApplyPalette(mesh, palette);

  RenderMode = RENDER_GOURAUD_SHADING;
  RenderAllFaces = false;
}

static void UnLoad() {
  MemUnref(mesh);
}

static void Init() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  PixBufClear(canvas);

  scene = NewScene();
  SceneAddObject(scene, NewSceneObject("Object", mesh));

  InitDisplay(WIDTH, HEIGHT, DEPTH);
  LoadPalette(palette);
}

static void Kill() {
  KillDisplay();

  MemUnref(canvas);
  MemUnref(scene);
}

static void Render(int frameNumber_) {
  float frameNumber = frameNumber_;
  float s = sin(frameNumber * 3.14159265f / 90.0f) + 1.0f;

  {
    MatrixStack3D *ms = GetObjectTranslation(scene, "Object");

    StackReset(ms);
    PushScaling3D(ms, 0.75f + 0.5f * s, 0.75f + 0.5f * s, 0.75f + 0.5f * s);
    PushRotation3D(ms, 0, (float)(-frameNumber * 2), frameNumber);
    PushTranslation3D(ms, 0.0f, 0.0f, -2.0f);
  }

  PixBufClear(canvas);
#if 0
  RenderFlatShading = true;
  PixBufSetColorMap(canvas, R_("ColorMap"), -32);
#endif
  PROFILE(RenderScene)
    RenderScene(scene, canvas);

  c2p1x1_8_c5_bm(canvas->data, GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

static void HandleEvent(InputEventT *event) {
  static bool paused = FALSE;
  static int oldFrameNumber = 0;
  int frameNumber = GetVBlankCounter();

  if (KEY_RELEASED(event, KEY_SPACE))
    paused = !paused;

  if (paused) {
    SetVBlankCounter(oldFrameNumber);
    frameNumber = oldFrameNumber;
  } else {
    oldFrameNumber = frameNumber;
  }

  if (KEY_RELEASED(event, KEY_RETURN)) {
    RenderMode++;
    if (RenderMode > 4)
      RenderMode = 0;

    if (RenderMode < RENDER_FILLED)
      RenderAllFaces = true;
    else
      RenderAllFaces = false;
  }
}

EffectT Effect = { "Object3D", Load, UnLoad, Init, Kill, Render, HandleEvent };
