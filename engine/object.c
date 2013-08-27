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

SceneObjectT *NewSceneObject(const char *name, MeshT *mesh) {
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

static void UpdatePolygonExt(PolygonExtT *polygonExt, TriangleT *polygon,
                             size_t polygonNum, Vector3D *vertex)
{
  size_t i;

  for (i = 0; i < polygonNum; i++) {
    Vector3D *normal = &polygonExt[i].normal;

    size_t p1 = polygon[i].p1;
    size_t p2 = polygon[i].p2;
    size_t p3 = polygon[i].p3;

    polygonExt[i].index = i;
    /*
     * NOTE: Don't use floating point comparison (i.e. max function) to select
     * a value. It's fragile and may be non-deterministic.
     */
    polygonExt[i].depth = vertex[p1].z + vertex[p2].z + vertex[p3].z;

    /* Calculate polygon normal. */
    {
      Vector3D u, v;

      V3D_Sub(&u, &vertex[p1], &vertex[p2]);
      V3D_Sub(&v, &vertex[p2], &vertex[p3]);

      V3D_Cross(normal, &u, &v);
      V3D_Normalize(normal, normal, 255.0f);
    }

    if (normal->z >= 0) {
      polygonExt[i].flags |= 1;
      polygonExt[i].color = normal->z;
    } else {
      polygonExt[i].flags &= ~1;
      polygonExt[i].color = 0;
    }
  }
}

static void Render(PixBufT *canvas, bool wireframe,
                   PolygonExtT **sortedPolygonExt, TriangleT *polygon,
                   size_t polygonNum, Vector3D *vertex)
{
  size_t j;

  for (j = 0; j < polygonNum; j++) {
    PolygonExtT *polyExt = sortedPolygonExt[j];

    size_t i = polyExt->index;
    size_t p1 = polygon[i].p1;
    size_t p2 = polygon[i].p2;
    size_t p3 = polygon[i].p3;

    if (!polyExt->flags)
      continue;

    canvas->fgColor = polyExt->color;

    if (wireframe) {
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

void RenderSceneObject(SceneObjectT *self, PixBufT *canvas) {
  MeshT *mesh = self->mesh;

  /* Apply vertex transformations. */
  Transform3D(self->vertex, mesh->vertex, mesh->vertexNum,
              GetMatrix3D(self->ms, 0));

  /* Calculate polygon normals & depths. */
  UpdatePolygonExt(self->polygonExt, mesh->polygon, mesh->polygonNum,
                   self->vertex);

  /* Sort polygons by depth. */
  TableSort((PtrT *)self->sortedPolygonExt,
            SortByDepth, 0, mesh->polygonNum - 1);

  ProjectTo2D(self->vertex, self->vertex, mesh->vertexNum,
              canvas->width / 2, canvas->height / 2, 160.0f);

  /* Render the object. */
  Render(canvas, self->wireframe,
         self->sortedPolygonExt, mesh->polygon, mesh->polygonNum,
         self->vertex);
}
