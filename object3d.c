#include <math.h>

#include "engine/mesh.h"
#include "gfx/line.h"
#include "gfx/transformations.h"
#include "gfx/triangle.h"
#include "std/resource.h"

#include "system/c2p.h"
#include "system/debug.h"
#include "system/display.h"
#include "system/memory.h"
#include "system/vblank.h"

#include "frame_tools.h"

const int WIDTH = 320;
const int HEIGHT = 256;
const int DEPTH = 8;

static CanvasT *Canvas;
static MeshT *Mesh;
static Vector3D *Vertices;
static PointT *Points;

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
  Canvas = NewCanvas(WIDTH, HEIGHT);
  CanvasFill(Canvas, 0);

  Mesh = GetResource("mesh");
  CenterMeshPosition(Mesh);
  NormalizeMesh(Mesh);
  Points = NEW_A(PointT, Mesh->vertex_count);
  Vertices = NEW_A(Vector3D, Mesh->vertex_count);

  TS_Init();
}

/*
 * Tear down effect function.
 */
void TearDownEffect() {
  TS_End();

  DELETE(Points);
  DELETE(Vertices);
  DeleteCanvas(Canvas);
}

/*
 * Effect rendering functions.
 */
void RenderMesh(int frameNumber) {
  size_t i;

  TS_Reset();
  TS_PushIdentity3D();
  TS_PushRotation3D((float)(frameNumber), (float)(frameNumber * 2), (float)(frameNumber * -3));
  TS_Compose3D();
  TS_PushTranslation3D(0.0f, 0.0f, 2.0f);
  TS_Compose3D();
  TS_PushScaling3D(60.0f, 60.0f, 60.0f);
  TS_Compose3D();
  TS_PushPerspective(0, 0, 160.0f);
  TS_Compose3D();

  M3D_Project2D(WIDTH/2, HEIGHT/2, Points, Mesh->vertex, Mesh->vertex_count, TS_GetMatrix3D(1));

  CanvasFill(Canvas, 0);

  for (i = 0; i < Mesh->triangle_count; i++) {
    size_t p1 = Mesh->triangle[i].p1;
    size_t p2 = Mesh->triangle[i].p2;
    size_t p3 = Mesh->triangle[i].p3;

    DrawLine(Canvas, Points[p1].x, Points[p1].y, Points[p2].x, Points[p2].y);
    DrawLine(Canvas, Points[p2].x, Points[p2].y, Points[p3].x, Points[p3].y);
    DrawLine(Canvas, Points[p3].x, Points[p3].y, Points[p1].x, Points[p1].y);
  }
}

void RenderChunky(int frameNumber) {
  c2p1x1_8_c5_bm(GetCanvasPixelData(Canvas), GetCurrentBitMap(), WIDTH, HEIGHT, 0, 0);
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
