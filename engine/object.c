#include "std/memory.h"
#include "engine/object.h"
#include "gfx/line.h"

SceneObjectT *NewSceneObject(const StrT name, MeshT *mesh) {
  SceneObjectT *self = MemNew(sizeof(SceneObjectT));

  self->name = StrDup(name);
  self->mesh = mesh;
  self->ms = NewMatrixStack3D();
  self->points = MemNew(sizeof(PointT) * mesh->vertexNum);

  return self;
}

void DeleteSceneObject(SceneObjectT *self) {
  if (self) {
    DeleteMatrixStack3D(self->ms);
    MemUnref(self->points);
    MemUnref(self->name);
    MemUnref(self);
  }
}

void RenderSceneObject(SceneObjectT *self, CanvasT *canvas) {
  MeshT *mesh = self->mesh;
  PointT *points = self->points;
  size_t i;

  {
    Matrix3D *transformation = GetMatrix3D(self->ms, 0);

    ProjectTo2D(GetCanvasWidth(canvas)/2, GetCanvasHeight(canvas)/2, points,
                mesh->vertex, mesh->vertexNum, transformation);
  }

  for (i = 0; i < mesh->polygonNum; i++) {
    size_t p1 = mesh->polygon[i].p1;
    size_t p2 = mesh->polygon[i].p2;
    size_t p3 = mesh->polygon[i].p3;

    DrawLine(canvas, points[p1].x, points[p1].y, points[p2].x, points[p2].y);
    DrawLine(canvas, points[p2].x, points[p2].y, points[p3].x, points[p3].y);
    DrawLine(canvas, points[p3].x, points[p3].y, points[p1].x, points[p1].y);
  }
}
