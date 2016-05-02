#include <math.h>

#include "std/debug.h"
#include "std/memory.h"

#include "engine/ms3d.h"
#include "engine/object.h"
#include "engine/scene.h"
#include "tools/frame.h"
#include "tools/loopevent.h"
#include "tools/profiling.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static PixBufT *canvas;
static SceneT *scene;
static MeshT *mesh;
static PaletteT *palette;

void AcquireResources() {
#if 0
  ResAdd("Mesh", NewMeshFromFile("data/shattered_ball.robj"));
  ResAdd("ColorMap", NewPixBufFromFile("data/shattered_ball_cmap.8"));
  ResAddPngImage("ColorMap", "Palette", "data/wecan_logo_cmap.png");
  ResAdd("Palette", NewPaletteFromFile("data/shattered_ball_cmap.pal"));
#else
  mesh = NewMeshFromFile("data/wecan_logo.robj");
#endif

  CalculateSurfaceNormals(mesh);
  NormalizeMeshSize(mesh);
  MeshApplyPalette(mesh, palette);

  RenderMode = RENDER_GOURAUD_SHADING;
  RenderAllFaces = false;
}

void ReleaseResources() {
}

bool SetupDisplay() {
  return InitDisplay(WIDTH, HEIGHT, DEPTH);
}

void SetupEffect() {
  canvas = NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT);
  scene = NewScene();

  SceneAddObject(scene, NewSceneObject("Object", mesh));

  PixBufClear(canvas);
  LoadPalette(palette);
}

void TearDownEffect() {
}

void RenderMesh(int frameNumber_) {
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

/*
 * Main loop.
 */

void MainLoop() {
  LoopEventT event = LOOP_CONTINUE;

  SetVBlankCounter(0);

  do {
    static bool paused = FALSE;
    static int oldFrameNumber = 0;
    int frameNumber = GetVBlankCounter();

    if (event == LOOP_PAUSE)
      paused = !paused;

    if (paused) {
      SetVBlankCounter(oldFrameNumber);
      frameNumber = oldFrameNumber;
    } else {
      oldFrameNumber = frameNumber;
    }

    if (event == LOOP_TRIGGER) {
      RenderMode++;
      if (RenderMode > 4)
        RenderMode = 0;

      if (RenderMode < RENDER_FILLED)
        RenderAllFaces = true;
      else
        RenderAllFaces = false;
    }

    RenderMesh(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
