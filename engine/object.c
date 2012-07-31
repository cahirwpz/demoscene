#include "std/memory.h"
#include "engine/object.h"
#include "gfx/line.h"
#include "gfx/triangle.h"

static void DeleteSceneObject(SceneObjectT *self) {
  MemUnref(self->ms);
  MemUnref(self->vertex);
  MemUnref(self->polygonExt);
  MemUnref(self->name);
}

SceneObjectT *NewSceneObject(const StrT name, MeshT *mesh) {
  SceneObjectT *self = NewRecordGC(SceneObjectT, (FreeFuncT)DeleteSceneObject);
  size_t i;

  self->name = StrDup(name);
  self->mesh = mesh;
  self->ms = NewMatrixStack3D();
  self->vertex = NewTable(Vector3D, mesh->vertexNum);
  self->polygonExt = NewTable(PolygonExtT, mesh->polygonNum);

  for (i = 0; i < mesh->polygonNum; i++)
    self->polygonExt[i].index = i;

  return self;
}

static void CalculateDepth(PolygonExtT *polygonExt,
                           TriangleT *polygon, size_t count, Vector3D *vertex)
{
  size_t i;

  for (i = 0; i < count; i++) {
    size_t p1 = polygon[i].p1;
    size_t p2 = polygon[i].p2;
    size_t p3 = polygon[i].p3;

    polygonExt[i].depth = (vertex[p1].z + vertex[p2].z + vertex[p3].z) / 3;
  }
}

void RenderSceneObject(SceneObjectT *self, CanvasT *canvas) {
  MeshT *mesh = self->mesh;
  Vector3D *vertex = self->vertex;
  size_t i;

  {
    Matrix3D *transformation = GetMatrix3D(self->ms, 0);

    ProjectTo2D(GetCanvasWidth(canvas)/2, GetCanvasHeight(canvas)/2, vertex,
                mesh->vertex, mesh->vertexNum, transformation);
  }

  CalculateDepth(self->polygonExt, mesh->polygon, mesh->polygonNum, vertex);

  for (i = 0; i < mesh->polygonNum; i++) {
    size_t p1 = mesh->polygon[i].p1;
    size_t p2 = mesh->polygon[i].p2;
    size_t p3 = mesh->polygon[i].p3;

    CanvasSetFgCol(canvas, i % 128 + 128);

#if 0
    DrawLine(canvas, vertex[p1].x, vertex[p1].y, vertex[p2].x, vertex[p2].y);
    DrawLine(canvas, vertex[p2].x, vertex[p2].y, vertex[p3].x, vertex[p3].y);
    DrawLine(canvas, vertex[p3].x, vertex[p3].y, vertex[p1].x, vertex[p1].y);
#else
    DrawTriangle(canvas, vertex[p1].x, vertex[p1].y,
                         vertex[p2].x, vertex[p2].y,
                         vertex[p3].x, vertex[p3].y);
#endif
  }
}
