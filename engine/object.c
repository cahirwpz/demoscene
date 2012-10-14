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
  MemUnref(self->sortedPolygonExt);
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
  self->sortedPolygonExt = (PolygonExtT **)NewTableAdapter(self->polygonExt);

  return self;
}

static bool SortByDepth(const PtrT a asm("a0"), const PtrT b asm("a1")) {
  return (((PolygonExtT *)a)->depth < ((PolygonExtT *)b)->depth);
}

void RenderSceneObject(SceneObjectT *self, CanvasT *canvas) {
  MeshT *mesh = self->mesh;
  Vector3D *vertex = self->vertex;

  /* Apply vertex transformations. */
  Transform3D(vertex, mesh->vertex, mesh->vertexNum,
              GetMatrix3D(self->ms, 0));

  /* Calculate polygon normals & depths. */
  {
    TriangleT *polygon = mesh->polygon;
    size_t i;

    for (i = 0; i < mesh->polygonNum; i++) {
      PolygonExtT *polyExt = &self->polygonExt[i];
      Vector3D *normal = &polyExt->normal;

      size_t p1 = polygon[i].p1;
      size_t p2 = polygon[i].p2;
      size_t p3 = polygon[i].p3;

      polyExt->index = i;
      polyExt->depth = max(max(vertex[p1].z, vertex[p2].z), vertex[p3].z);

      /* Calculate polygon normal. */
      {
        Vector3D u, v;

        V3D_Sub(&u, (Vector3D *)&vertex[p1], (Vector3D *)&vertex[p2]);
        V3D_Sub(&v, (Vector3D *)&vertex[p2], (Vector3D *)&vertex[p3]);

        V3D_Cross(normal, &u, &v);
        V3D_Normalize(normal, normal, 1.0f);
      }

      if (normal->z >= 0) {
        polyExt->flags |= 1;
        polyExt->color = (uint32_t)(normal->z * 255.0f);
      } else {
        polyExt->flags |= ~1;
      }
    }
  }

  /* Sort polygons by depth. */
  TableSort((PtrT *)self->sortedPolygonExt, SortByDepth, 0, mesh->polygonNum - 1);

  ProjectTo2D(vertex, vertex, mesh->vertexNum,
              GetCanvasWidth(canvas) / 2, GetCanvasHeight(canvas) / 2, 160.0f);

  /* Render the object. */
  {
    size_t n = mesh->polygonNum;
    size_t j;

    for (j = 0; j < n; j++) {
      PolygonExtT *polyExt = self->sortedPolygonExt[j];

      size_t i = polyExt->index;
      size_t p1 = mesh->polygon[i].p1;
      size_t p2 = mesh->polygon[i].p2;
      size_t p3 = mesh->polygon[i].p3;

      if (!polyExt->flags)
        continue;

      CanvasSetFgCol(canvas, polyExt->color);

      if (self->wireframe) {
        DrawLine(canvas, vertex[p1].x, vertex[p1].y, vertex[p2].x, vertex[p2].y);
        DrawLine(canvas, vertex[p2].x, vertex[p2].y, vertex[p3].x, vertex[p3].y);
        DrawLine(canvas, vertex[p3].x, vertex[p3].y, vertex[p1].x, vertex[p1].y);
      } else {
        DrawTriangle(canvas,
                     vertex[p1].x, vertex[p1].y,
                     vertex[p2].x, vertex[p2].y,
                     vertex[p3].x, vertex[p3].y);
      }
    }
  }
}
