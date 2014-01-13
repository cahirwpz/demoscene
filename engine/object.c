#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/quicksort.h"
#include "std/table.h"
#include "engine/object.h"
#include "gfx/line.h"
#include "gfx/triangle.h"
#include "engine/triangle.h"

RenderModeT RenderMode = RENDER_WIREFRAME;
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

  for (i = 0; i < polygonNum; i++, polygon++) {
    PolygonExtT *polyExt = &polygonExt[i];
    int p1 = polygon->p[0];
    int p2 = polygon->p[1];
    int p3 = polygon->p[2];

    polyExt->index = i;

    /*
     * NOTE: Don't use floating point comparison (i.e. max function) to select
     * a value. It's fragile and may be non-deterministic.
     */
    polyExt->depth = (vertex[p1].z + vertex[p2].z + vertex[p3].z) / 3.0f;

#if 0
    /* Calculate angle between camera and surface normal. */
    {
      Vector3D cameraToFace = { -vertex[p1].x, -vertex[p1].y, -vertex[p1].z };
      float angle;

      V3D_NormalizeToUnit(&cameraToFace, &cameraToFace);
      V3D_NormalizeToUnit(&unitNormal, &normal[i]);

      angle = V3D_Dot(&cameraToFace, &unitNormal);
    }
#endif
    V3D_NormalizeToUnit(&polyExt->normal, &normal[i]);


    if (polyExt->normal.z > 0)
      polyExt->flags |= 1;
    else
      polyExt->flags &= ~1;
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

static void
UpdateVertexNormals(VertexExtT *vertexExt, IndexArrayT *indexArray,
                    PolygonExtT *polygonExt, int vertexNum)
{
  int i, j;

  for (i = 0; i < vertexNum; i++) {
    uint16_t count = indexArray[i].count;
    uint16_t *index = indexArray[i].index;
    Vector3D *normal = &vertexExt[i].normal;

    normal->x = 0.0f;
    normal->y = 0.0f;
    normal->z = 0.0f;

    for (j = 0; j < count; j++)
      V3D_Add(normal, normal, &polygonExt[index[j]].normal);

    V3D_Scale(normal, normal, 1.0f / (float)count);
  }
}

__regargs static uint8_t
DetermineSurfaceColor(PixBufT *canvas, SurfaceT *surface, PolygonExtT *polyExt,
                      int color)
{
  if (RenderMode == RENDER_FLAT_SHADING) {
    int shade = abs((int)(polyExt->normal.z * 255.0f));

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
    EdgeScanT *e1, *e2, *e3;
    int p1 = polygon->p[0];
    int p2 = polygon->p[1];
    int p3 = polygon->p[2];

    SurfaceT *surface = &mesh->surface[polygon->surface];

    if (vertex[p1].flags & vertex[p2].flags & vertex[p3].flags)
      continue;

    if (!RenderAllFaces && !polyExt->flags && !surface->sideness)
      continue;

    canvas->fgColor =
      DetermineSurfaceColor(canvas, surface, polyExt, polygon->surface);

    e1 = &edge[polygon->e[0]];
    e2 = &edge[polygon->e[1]];
    e3 = &edge[polygon->e[2]];

    switch (RenderMode) {
      case RENDER_WIREFRAME:
        if (!e1->done && !(vertex[p1].flags & vertex[p2].flags)) {
          if (vertex[p1].flags | vertex[p2].flags)
            DrawLine(canvas, e1->xs, e1->ys, e1->xe, e1->ye);
          else
            DrawLineUnsafe(canvas, e1->xs, e1->ys, e1->xe, e1->ye);
          e1->done = true;
        }

        if (!e2->done && !(vertex[p2].flags & vertex[p3].flags)) {
          if (vertex[p2].flags | vertex[p3].flags)
            DrawLine(canvas, e2->xs, e2->ys, e2->xe, e2->ye);
          else
            DrawLineUnsafe(canvas, e2->xs, e2->ys, e2->xe, e2->ye);
          e2->done = true;
        }

        if (!e3->done && !(vertex[p1].flags & vertex[p3].flags)) {
          if (vertex[p1].flags | vertex[p3].flags)
            DrawLine(canvas, e3->xs, e3->ys, e3->xe, e3->ye);
          else
            DrawLineUnsafe(canvas, e3->xs, e3->ys, e3->xe, e3->ye);
          e3->done = true;
        }
        break;

      case RENDER_WIREFRAME_AA:
        if (!e1->done && !(vertex[p1].flags & vertex[p2].flags)) {
          DrawLineAA(canvas, e1->xs, e1->ys, e1->xe, e1->ye);
          e1->done = true;
        }

        if (!e2->done && !(vertex[p2].flags & vertex[p3].flags)) {
          DrawLineAA(canvas, e2->xs, e2->ys, e2->xe, e2->ye);
          e2->done = true;
        }

        if (!e3->done && !(vertex[p1].flags & vertex[p3].flags)) {
          DrawLineAA(canvas, e3->xs, e3->ys, e3->xe, e3->ye);
          e3->done = true;
        }
        break;

      case RENDER_FILLED:
      case RENDER_FLAT_SHADING:
        {
          bool clipping = vertex[p1].flags | vertex[p2].flags | vertex[p3].flags;

          if (!clipping) {
            EdgeScanT *e1 = &edge[polygon->e[0]];
            EdgeScanT *e2 = &edge[polygon->e[1]];
            EdgeScanT *e3 = &edge[polygon->e[2]];
            RasterizeTriangle(canvas, e1, e2, e3);
          } else {
            DrawTriangle(canvas,
                         (TriPoint *)&vertex[p1],
                         (TriPoint *)&vertex[p2], 
                         (TriPoint *)&vertex[p3]);
          }
        }
        break;

      case RENDER_GOURAUD_SHADING:
        {
          TriPointC point[3];

          point[0].x = vertex[p1].x;
          point[0].y = vertex[p1].y;
          point[0].c = fabsf(vertex[p1].normal.z) * 255.0f;

          point[1].x = vertex[p2].x;
          point[1].y = vertex[p2].y;
          point[1].c = fabsf(vertex[p2].normal.z) * 255.0f;

          point[2].x = vertex[p3].x;
          point[2].y = vertex[p3].y;
          point[2].c = fabsf(vertex[p3].normal.z) * 255.0f;

          DrawTriangleC(canvas, &point[0], &point[1], &point[2]);
        }
        break;
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

  if (RenderMode == RENDER_GOURAUD_SHADING)
    UpdateVertexNormals(self->vertexExt, mesh->vertexToPoly.vertex,
                        self->polygonExt, mesh->vertexNum);

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

      InitEdgeScan(edgeScan, y1, y2, x1, x2);
    }
  }

  /* Render the object. */
  RenderObject(self, canvas);
}
