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
  self->polygonExt = NewTable(PolygonExtT, mesh->polygonNum);
  self->sortedPolygonExt = (PolygonExtT **)NewTableAdapter(self->polygonExt);
  self->surfaceNormal = NewTable(Vector3D, mesh->polygonNum);

  return self;
}

static bool SortByDepth(const PtrT a asm("a0"), const PtrT b asm("a1")) {
  return (((PolygonExtT *)a)->depth < ((PolygonExtT *)b)->depth);
}

static void UpdatePolygonExt(PolygonExtT *polygonExt, TriangleT *polygon,
                             size_t polygonNum, Vector3D *vertex, Vector3D *normal)
{
  int i;

  for (i = 0; i < polygonNum; i++) {
    int p1 = polygon[i].p1;
    int p2 = polygon[i].p2;
    int p3 = polygon[i].p3;
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

static void Render(PixBufT *canvas, PolygonExtT **sortedPolygonExt,
                   TriangleT *polygon, size_t polygonNum,
                   Vector3D *vertex, SurfaceT *surface)
{
  int j;

  for (j = 0; j < polygonNum; j++) {
    PolygonExtT *polyExt = sortedPolygonExt[j];

    int i = polyExt->index;
    int p1 = polygon[i].p1;
    int p2 = polygon[i].p2;
    int p3 = polygon[i].p3;

    if (!RenderAllFaces && !polyExt->flags && !surface[polygon[i].surface].sideness)
      continue;

    if (RenderFlatShading) {
      if (canvas->blit.cmap.data) {
        int color = polyExt->color + canvas->blit.cmap.shift;

        if (color > 255)
          color = 255;
        if (color < 0)
          color = 0;

        color += polygon[i].surface * 256;

        canvas->fgColor = canvas->blit.cmap.data[color];
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

  /* Apply transformations to surface normals */
  Transform3D_2(self->surfaceNormal, mesh->surfaceNormal, mesh->polygonNum,
                GetMatrix3D(self->ms, 0));

  /* Calculate polygon normals & depths. */
  UpdatePolygonExt(self->polygonExt, mesh->polygon, mesh->polygonNum,
                   self->vertex, self->surfaceNormal);

  /* Sort polygons by depth. */
  TableSort((PtrT *)self->sortedPolygonExt,
            SortByDepth, 0, mesh->polygonNum - 1);

  ProjectTo2D(self->vertex, self->vertex, mesh->vertexNum,
              canvas->width / 2, canvas->height / 2, -160.0f);

  /* Render the object. */
  Render(canvas,
         self->sortedPolygonExt, mesh->polygon, mesh->polygonNum,
         self->vertex, mesh->surface);
}
