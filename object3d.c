#include <math.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/resource.h"

#include "engine/mesh.h"
#include "gfx/line.h"
#include "gfx/ms3d.h"
#include "gfx/triangle.h"

#include "system/c2p.h"
#include "system/display.h"
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
  RSC_CANVAS("Canvas", WIDTH, HEIGHT);
  RSC_MS3D("ms3d");

  {
    MeshT *mesh = R_("Mesh");

    CenterMeshPosition(mesh);
    NormalizeMeshSize(mesh);

    RSC_ARRAY("Points", PointT, mesh->vertex_count);
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
  MatrixStack3D *ms = R_("ms3d");

  Reset3D(ms);
  PushIdentity3D(ms);
  PushRotation3D(ms, (float)(frameNumber), (float)(frameNumber * 2), (float)(frameNumber * -3));
  PushTranslation3D(ms, 0.0f, 0.0f, 2.0f);
  PushScaling3D(ms, 60.0f, 60.0f, 60.0f);
  PushPerspective3D(ms, 0, 0, 160.0f);

  {
    size_t i;
    MeshT *mesh = R_("Mesh");
    PointT *points = R_("Points");
    CanvasT *canvas = R_("Canvas");

    ProjectTo2D(WIDTH/2, HEIGHT/2, points, mesh->vertex, mesh->vertex_count,
                GetMatrix3D(ms, 0));

    CanvasFill(canvas, 0);

    for (i = 0; i < mesh->triangle_count; i++) {
      size_t p1 = mesh->triangle[i].p1;
      size_t p2 = mesh->triangle[i].p2;
      size_t p3 = mesh->triangle[i].p3;

      DrawLine(canvas, points[p1].x, points[p1].y, points[p2].x, points[p2].y);
      DrawLine(canvas, points[p2].x, points[p2].y, points[p3].x, points[p3].y);
      DrawLine(canvas, points[p3].x, points[p3].y, points[p1].x, points[p1].y);
    }
  }
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(R_("Canvas")),
                 GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
}

/*
 * Main loop.
 */
void MainLoop() {
  SetVBlankCounter(0);

  while (GetVBlankCounter() < 500) {
    int frameNumber = GetVBlankCounter();

    RenderMesh(frameNumber);
    RenderChunky(frameNumber);
    RenderFrameNumber(frameNumber);

    DisplaySwap();
  }
}
