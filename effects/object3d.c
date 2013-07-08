#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "engine/ms3d.h"
#include "engine/scene.h"
#include "tools/frame.h"
#include "tools/loopevent.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  ResAdd("Scene", NewScene());
  ResAdd("Mesh", NewMeshFromFile("data/whelpz.robj"));
  ResAdd("Canvas", NewPixBuf(PIXBUF_CLUT, WIDTH, HEIGHT));

  {
    MeshT *mesh = R_("Mesh");

    CenterMeshPosition(mesh);
    NormalizeMeshSize(mesh);
  }

  SceneAddObject(R_("Scene"), NewSceneObject("Object", R_("Mesh")));
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
void RenderMesh(int frameNumber) {
  PixBufT *canvas = R_("Canvas");
  SceneT *scene = R_("Scene");
  float s = sin(frameNumber * 3.14159265f / 90.0f);

  {
    MatrixStack3D *ms = GetObjectTranslation(scene, "Object");

    StackReset(ms);
    PushScaling3D(ms, 0.6f + 0.25f * s, 0.6f + 0.25f * s, 0.6f + 0.25f * s);
    PushRotation3D(ms, (float)(frameNumber), (float)(frameNumber * 2), (float)(frameNumber * -3));
    PushTranslation3D(ms, 0.0f, 0.0f, 2.0f);
  }

  PixBufClear(canvas);
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
      SceneObjectT *object = GetObject(R_("Scene"), "Object");

      object->wireframe = !object->wireframe;
    }

    RenderMesh(frameNumber);
    RenderFrameNumber(frameNumber);
    RenderFramesPerSecond(frameNumber);

    DisplaySwap();
  } while ((event = ReadLoopEvent()) != LOOP_EXIT);
}
