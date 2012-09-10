#include "std/debug.h"
#include "std/math.h"
#include "std/memory.h"
#include "std/table.h"
#include "engine/object.h"
#include "gfx/line.h"
#include "gfx/triangle.h"

static void DeleteSceneObject(SceneObjectT *self) {
  MemUnref(self->ms);
  MemUnref(self->vertex);
  MemUnref(self->polygonExt);
  MemUnref(self->surfaceNormal);
  MemUnref(self->name);
}

TYPEDECL(SceneObjectT, (FreeFuncT)DeleteSceneObject);

SceneObjectT *NewSceneObject(const StrT name, MeshT *mesh) {
  SceneObjectT *self = NewInstance(SceneObjectT);

  self->name = StrDup(name);
  self->mesh = mesh;
  self->ms = NewMatrixStack3D();
  self->vertex = NewTable(Vector3D, mesh->vertexNum);
  self->polygonExt = NewTable(PolygonExtT, mesh->polygonNum);
  self->surfaceNormal = NewTable(Vector3D, mesh->polygonNum);

  return self;
}

static bool SortByDepth(const PtrT a asm("a0"), const PtrT b asm("a1")) {
  return (((PolygonExtT *)a)->depth < ((PolygonExtT *)b)->depth);
}

void RenderSceneObject(SceneObjectT *self, CanvasT *canvas) {
  MeshT *mesh = self->mesh;
  Vector3D *vertex = self->vertex;

  /* Apply transformations */
  {
    Matrix3D *transformation = GetMatrix3D(self->ms, 0);

    ProjectTo2D(GetCanvasWidth(canvas)/2, GetCanvasHeight(canvas)/2, vertex,
                mesh->vertex, mesh->vertexNum, transformation);
  }

  /* Calculate polygons depth. */
  {
    PolygonExtT *polygonExt = self->polygonExt;
    TriangleT *polygon = mesh->polygon;
    size_t i;

    for (i = 0; i < mesh->polygonNum; i++) {
      size_t p1 = polygon[i].p1;
      size_t p2 = polygon[i].p2;
      size_t p3 = polygon[i].p3;

      polygonExt[i].index = i;
      polygonExt[i].depth = max(max(vertex[p1].z, vertex[p2].z), vertex[p3].z);
    }
  }

  /* Sort polygons by depth. */
  {
    SortAdapterT *adapter = NewSortAdapter(self->polygonExt, SortByDepth);
    TableSort(adapter, 0, mesh->polygonNum - 1);
    MemUnref(adapter);
  }

  /* Render the object. */
  {
    int j = mesh->polygonNum - 1;
    float zMin = self->polygonExt[0].depth;
    float zMax = self->polygonExt[j].depth;
    float zInvDiff = 255.0f / (zMax - zMin);

    do {
      size_t i = self->polygonExt[j].index;
      size_t p1 = mesh->polygon[i].p1;
      size_t p2 = mesh->polygon[i].p2;
      size_t p3 = mesh->polygon[i].p3;

      CanvasSetFgCol(canvas, (zMax - self->polygonExt[j].depth) * zInvDiff);

      if (self->wireframe) {
        DrawLine(canvas, vertex[p1].x, vertex[p1].y, vertex[p2].x, vertex[p2].y);
        DrawLine(canvas, vertex[p2].x, vertex[p2].y, vertex[p3].x, vertex[p3].y);
        DrawLine(canvas, vertex[p3].x, vertex[p3].y, vertex[p1].x, vertex[p1].y);
      } else {
        DrawTriangle(canvas, vertex[p1].x, vertex[p1].y,
                     vertex[p2].x, vertex[p2].y,
                     vertex[p3].x, vertex[p3].y);
      }
    } while (j-- >= 0);
  }
}
