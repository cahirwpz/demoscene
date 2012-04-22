#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "engine/object.h"
#include "gfx/canvas.h"
#include "gfx/ms3d.h"

#include "system/c2p.h"
#include "system/display.h"
#include "system/input.h"
#include "system/vblank.h"

#include "frame_tools.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

/*
 * Set up resources.
 */
void AddInitialResources() {
  RSC_MESH_FILE("Mesh", "data/whelpz.robj");
  RSC_SCENE_OBJECT("Object", R_("Mesh"));
  RSC_CANVAS("Canvas", WIDTH, HEIGHT);

  {
    MeshT *mesh = R_("Mesh");

    CenterMeshPosition(mesh);
    NormalizeMeshSize(mesh);
  }
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
  SceneObjectT *obj = R_("Object");
  MatrixStack3D *ms = obj->ms;
  CanvasT *canvas = R_("Canvas");

  Reset3D(ms);
  PushIdentity3D(ms);
  PushRotation3D(ms, (float)(frameNumber), (float)(frameNumber * 2), (float)(frameNumber * -3));
  PushTranslation3D(ms, 0.0f, 0.0f, 2.0f);
  PushScaling3D(ms, 60.0f, 60.0f, 60.0f);
  PushPerspective3D(ms, 0, 0, 160.0f);

  CanvasFill(canvas, 0);
  RenderSceneObject(obj, canvas);
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
          const char *name[] = {"left", "right", "middle"};

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
