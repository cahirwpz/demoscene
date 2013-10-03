#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/quicksort.h"
#include "std/table.h"
#include "engine/object.h"
#include "gfx/line.h"
#include "gfx/triangle.h"

static void DeleteSceneObject(SceneObjectT *self) {
  MemUnref(self->ms);
  MemUnref(self->vertex);
  MemUnref(self->polygonExt);
  MemUnref(self->sortedPolygonExt);
  MemUnref(self->surfaceNormal);
  MemUnref(self->vertexExt);
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
    polygonExt[i].depth = vertex[p1].z + vertex[p2].z + vertex[p3].z;

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

bool RenderFlatShading = false;
bool RenderWireFrame = false;
bool RenderAllFaces = false;

typedef int fixed_t;

static inline fixed_t float_to_fx(float v) {
  return v * 16.0f;
}

static inline int fx_rint(fixed_t v) {
  return (v + 8) >> 4;
}

static void UpdateVertexExt(PixBufT *canvas, VertexExtT *dst, Vector3D *src, int vertexNum) {
  const float viewerX = canvas->width / 2;
  const float viewerY = canvas->height / 2;
  const float viewerZ = -160.0f;
  int i;

  for (i = 0; i < vertexNum; i++) {
    float invZ = viewerZ / src[i].z;
    fixed_t fx = float_to_fx(src[i].x * invZ + viewerX);
    fixed_t fy = float_to_fx(src[i].y * invZ + viewerY);
    int x = fx_rint(fx);
    int y = fx_rint(fy);
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

static void Render(PixBufT *canvas, PolygonExtT **sortedPolygonExt,
                   TriangleT *polygon, size_t polygonNum,
                   Vector3D *vertex, VertexExtT *vertexExt, SurfaceT *surface)
{
  int j;

  for (j = 0; j < polygonNum; j++) {
    PolygonExtT *polyExt = sortedPolygonExt[j];

    int i = polyExt->index;
    int p1 = polygon[i].p[0];
    int p2 = polygon[i].p[1];
    int p3 = polygon[i].p[2];

    if (vertexExt[p1].flags & vertexExt[p2].flags & vertexExt[p3].flags)
      continue;

    if (!RenderAllFaces && !polyExt->flags && !surface[polygon[i].surface].sideness)
      continue;

    if (RenderFlatShading) {
      if (canvas->blit.cmap) {
        int color = polyExt->color / 2 + 128 - 16;

        if (color > 255)
          color = 255;
        if (color < 0)
          color = 0;

        color += polygon[i].surface * 256;

        canvas->fgColor = canvas->blit.cmap[color];
      } else {
        canvas->fgColor = polyExt->color;
      }
    } else {
      canvas->fgColor = surface[polygon[i].surface].color.clut;
    }

    if (RenderWireFrame) {
      DrawLine(canvas, vertex[p1].x, vertex[p1].y, vertex[p2].x, vertex[p2].y);
      DrawLine(canvas, vertex[p2].x, vertex[p2].y, vertex[p3].x, vertex[p3].y);
      DrawLine(canvas, vertex[p3].x, vertex[p3].y, vertex[p1].x, vertex[p1].y);
    } else {
      DrawTriangleNew(canvas,
                      vertexExt[p1].x, vertexExt[p1].y,
                      vertexExt[p2].x, vertexExt[p2].y,
                      vertexExt[p3].x, vertexExt[p3].y,
                      vertexExt[p1].flags | vertexExt[p2].flags | vertexExt[p3].flags);
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

  /* Render the object. */
  Render(canvas,
         self->sortedPolygonExt, mesh->polygon, mesh->polygonNum,
         self->vertex, self->vertexExt, mesh->surface);
}
