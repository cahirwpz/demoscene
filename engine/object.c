#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/quicksort.h"
#include "std/table.h"
#include "engine/object.h"
#include "gfx/line.h"
#include "gfx/triangle.h"
#include "engine/triangle.h"

bool RenderFlatShading = false;
bool RenderWireFrame = false;
bool RenderAllFaces = false;

static void DeleteSceneObject(SceneObjectT *self) {
  MemUnref(self->ms);
  MemUnref(self->vertex);
  MemUnref(self->vertexExt);
  MemUnref(self->polygonExt);
  MemUnref(self->sortedPolygonExt);
  MemUnref(self->edgeScan);
  MemUnref(self->surfaceNormal);
  MemUnref(self->name);
}

TYPEDECL(SceneObjectT, (FreeFuncT)DeleteSceneObject);

SceneObjectT *NewSceneObject(const char *name, MeshT *mesh) {
  SceneObjectT *self = NewInstance(SceneObjectT);

  self->name = StrDup(name);
  self->mesh = mesh;
  self->ms = NewMatrixStack3D();
  self->vertex = NewTable(Vector3D, mesh->vertexNum);
  self->vertexExt = NewTable(VertexExtT, mesh->vertexNum);
  self->polygonExt = NewTable(PolygonExtT, mesh->polygonNum);
  self->sortedPolygonExt = (PolygonExtT **)NewTableAdapter(self->polygonExt);
  self->edgeScan = NewTable(EdgeScanT, mesh->edgeNum);
  self->surfaceNormal = NewTable(Vector3D, mesh->polygonNum);

  return self;
}

static inline bool SortByDepth(const PolygonExtT *a, const PolygonExtT *b) {
  return a->depth < b->depth;
}

QUICKSORT(PolygonExtT, SortByDepth);

static void UpdatePolygonExt(PolygonExtT *polygonExt, TriangleT *polygon,
                             size_t polygonNum, Vector3D *vertex, Vector3D *normal)
{
  int i;

  for (i = 0; i < polygonNum; i++) {
    int p1 = polygon[i].p[0];
    int p2 = polygon[i].p[1];
    int p3 = polygon[i].p[2];
    Vector3D unitNormal;
    float angle;

    polygonExt[i].index = i;

    /*
     * NOTE: Don't use floating point comparison (i.e. max function) to select
     * a value. It's fragile and may be non-deterministic.
     */
    polygonExt[i].depth = (vertex[p1].z + vertex[p2].z + vertex[p3].z) / 3.0f;

    /* Calculate angle between camera and surface normal. */
    {
      Vector3D cameraToFace = { -vertex[p1].x, -vertex[p1].y, -vertex[p1].z };

      V3D_NormalizeToUnit(&cameraToFace, &cameraToFace);
      V3D_NormalizeToUnit(&unitNormal, &normal[i]);

      angle = V3D_Dot(&cameraToFace, &unitNormal);
    }

    angle = unitNormal.z;

    polygonExt[i].color = abs((int)(angle * 255.0f));

    if (angle > 0)
      polygonExt[i].flags |= 1;
    else
      polygonExt[i].flags &= ~1;
  }
}

static void UpdateVertexExt(PixBufT *canvas, VertexExtT *dst, Vector3D *src, int vertexNum) {
  const float viewerX = canvas->width / 2;
  const float viewerY = canvas->height / 2;
  const float viewerZ = -160.0f;
  int i;

  for (i = 0; i < vertexNum; i++) {
    float invZ = viewerZ / src[i].z;
    float fx = src[i].x * invZ + viewerX;
    float fy = src[i].y * invZ + viewerY;
    int x = lroundf(fx);
    int y = lroundf(fy);
    uint8_t flags = 0;

    if (x < 0)
      flags |= 1;
    else if (x > canvas->width - 1)
      flags |= 2;

    if (y < 0)
      flags |= 4;
    else if (y > canvas->height - 1)
      flags |= 8;

    dst[i].x = fx;
    dst[i].y = fy;
    dst[i].flags = flags;
  }
}

__regargs static uint8_t
DetermineSurfaceColor(PixBufT *canvas, SurfaceT *surface, int color, int shade) {
  if (RenderFlatShading) {
    if (canvas->blit.cmap) {
      shade = shade / 2 + 128 - 16;

      if (shade > 255)
        shade = 255;
#if 0
      if (shade < 0)
        shade = 0;
#endif

      return canvas->blit.cmap[(color << 8) | shade];
    }
    return shade;
  }
  return surface->color.clut;
}

static void RenderObject(SceneObjectT *self, PixBufT *canvas) {
  MeshT *mesh = self->mesh;
  VertexExtT *vertex = self->vertexExt;
  EdgeScanT *edge = self->edgeScan;
  int j;

  for (j = 0; j < mesh->polygonNum; j++) {
    PolygonExtT *polyExt = self->sortedPolygonExt[j];
    TriangleT *polygon = &mesh->polygon[polyExt->index];
    int p1 = polygon->p[0];
    int p2 = polygon->p[1];
    int p3 = polygon->p[2];

    SurfaceT *surface = &mesh->surface[polygon->surface];

    if (vertex[p1].flags & vertex[p2].flags & vertex[p3].flags)
      continue;

    if (!RenderAllFaces && !polyExt->flags && !surface->sideness)
      continue;

    canvas->fgColor =
      DetermineSurfaceColor(canvas, surface, polygon->surface, polyExt->color);

    if (RenderWireFrame) {
      DrawLine(canvas,
               lroundf(vertex[p1].x), lroundf(vertex[p1].y),
               lroundf(vertex[p2].x), lroundf(vertex[p2].y));
      DrawLine(canvas, 
               lroundf(vertex[p2].x), lroundf(vertex[p2].y),
               lroundf(vertex[p3].x), lroundf(vertex[p3].y));
      DrawLine(canvas,
               lroundf(vertex[p3].x), lroundf(vertex[p3].y),
               lroundf(vertex[p1].x), lroundf(vertex[p1].y));
    } else {
      bool clipping = vertex[p1].flags | vertex[p2].flags | vertex[p3].flags;

      if (!clipping) {
        EdgeScanT *e1 = &edge[polygon->e[0]];
        EdgeScanT *e2 = &edge[polygon->e[1]];
        EdgeScanT *e3 = &edge[polygon->e[2]];
        RasterizeTriangle(canvas, e1, e2, e3);
      } else {
        DrawTriangle(canvas,
                     vertex[p1].x, vertex[p1].y,
                     vertex[p2].x, vertex[p2].y,
                     vertex[p3].x, vertex[p3].y);
      }
    }
  }
}

void RenderSceneObject(SceneObjectT *self, PixBufT *canvas) {
  MeshT *mesh = self->mesh;

  /* Apply vertex transformations. */
  Transform3D(self->vertex, mesh->vertex, mesh->vertexNum,
              GetMatrix3D(self->ms, 0));

  UpdateVertexExt(canvas, self->vertexExt, self->vertex, mesh->vertexNum);

  /* Apply transformations to surface normals */
  Transform3D_2(self->surfaceNormal, mesh->surfaceNormal, mesh->polygonNum,
                GetMatrix3D(self->ms, 0));

  /* Calculate polygon normals & depths. */
  UpdatePolygonExt(self->polygonExt, mesh->polygon, mesh->polygonNum,
                   self->vertex, self->surfaceNormal);

  /* Sort polygons by depth. */
  QuickSortPolygonExtT(self->sortedPolygonExt, 0, mesh->polygonNum - 1);

  /* Invalidate all edges */
  {
    VertexExtT *vertex = self->vertexExt;
    int i;

    for (i = 0; i < mesh->edgeNum; i++) {
      EdgeScanT *edgeScan = &self->edgeScan[i];
      EdgeT *edge = &mesh->edge[i];

      float x1 = vertex[edge->p[0]].x;
      float y1 = vertex[edge->p[0]].y;
      float x2 = vertex[edge->p[1]].x;
      float y2 = vertex[edge->p[1]].y;

      if (y1 > y2) {
        swapr(x1, x2);
        swapr(y1, y2);
      }

      InitEdgeScan(edgeScan, FP16_float(y1), FP16_float(y2), FP16_float(x1), FP16_float(x2));
    }
  }

  /* Render the object. */
  RenderObject(self, canvas);
}
