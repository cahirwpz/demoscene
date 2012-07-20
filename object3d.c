#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "engine/scene.h"
#include "gfx/canvas.h"
#include "gfx/ms3d.h"
#include "tools/frame.h"

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
  ResAdd("Canvas", NewCanvas(WIDTH, HEIGHT));

  {
    MeshT *mesh = R_("Mesh");

    CenterMeshPosition(mesh);
    NormalizeMeshSize(mesh);
  }

  SceneAddObject(R_("Scene"), NewSceneObject("Object1", R_("Mesh")));
  SceneAddObject(R_("Scene"), NewSceneObject("Object2", R_("Mesh")));
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
  CanvasFill(R_("Canvas"), 0);
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
  CanvasT *canvas = R_("Canvas");
  SceneT *scene = R_("Scene");
  float s = sin(frameNumber * 3.14159265f / 90.0f);

  {
    MatrixStack3D *ms = GetObjectTranslation(scene, "Object1");

    StackReset(ms);
    PushScaling3D(ms, 0.6f + 0.25 * s, 0.6f + 0.25f * s, 0.6f + 0.25f * s);
    PushRotation3D(ms, (float)(frameNumber), (float)(frameNumber * 2), (float)(frameNumber * -3));
    PushTranslation3D(ms, -0.75f, 0.0f, 2.0f);
    PushPerspective3D(ms, 0, 0, 160.0f);
  }

  {
    MatrixStack3D *ms = GetObjectTranslation(scene, "Object2");

    StackReset(ms);
    PushScaling3D(ms, 0.6f - 0.25f * s, 0.6f - 0.25f * s, 0.6f - 0.25f * s);
    PushRotation3D(ms, (float)(-frameNumber), (float)(-frameNumber * 2), (float)(frameNumber * 3));
    PushTranslation3D(ms, 0.75f, 0.0f, 2.0f);
    PushPerspective3D(ms, 0, 0, 160.0f);
  }

  CanvasFill(canvas, 0);
  RenderScene(scene, canvas);
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(R_("Canvas")),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

void PrintAllEvents() {
  InputEventT event; 

  while (EventQueuePop(&event)) {
    switch (event.ie_Class) {
      case IECLASS_RAWKEY:
        LOG("Key %ld %s (%04lx).",
            (LONG)(event.ie_Code & ~IECODE_UP_PREFIX),
            (event.ie_Code & IECODE_UP_PREFIX) ? "up" : "down",
            (LONG)event.ie_Qualifier);
        break;

      case IECLASS_RAWMOUSE:
        if (event.ie_Code == IECODE_NOBUTTON) {
          LOG("Mouse move: (%ld,%ld).", (LONG)event.ie_X, (LONG)event.ie_Y);
        } else {
          const StrT name[] = {"left", "right", "middle"};

          LOG("Mouse %s key %s.",
              name[(event.ie_Code & ~IECODE_UP_PREFIX) - IECODE_LBUTTON],
              (event.ie_Code & IECODE_UP_PREFIX) ? "up" : "down");
        }
        break;

      default:
        break;
    }
  }
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  while (GetVBlankCounter() < 500) {
    int frameNumber = GetVBlankCounter();

    PrintAllEvents();

    RenderMesh(frameNumber);
    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  }
}
