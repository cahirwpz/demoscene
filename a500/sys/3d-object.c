#include "memory.h"
#include "file.h"
#include "reader.h"
#include "3d.h"
#include "fx.h"

__regargs Object3D *NewObject3D(Mesh3D *mesh) {
  Object3D *object = MemAlloc(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);
  WORD vertices = mesh->vertices;
  WORD faces = mesh->faces;
  WORD edges = mesh->edges;

  object->mesh = mesh;
  object->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);
  object->vertexFlags = MemAlloc(vertices, MEMF_PUBLIC);
  object->point = MemAlloc(sizeof(Point2D) * vertices, MEMF_PUBLIC);
  object->faceNormal = MemAlloc(sizeof(Point3D) * faces, MEMF_PUBLIC);
  object->faceFlags = MemAlloc(faces, MEMF_PUBLIC);
  object->edgeFlags = MemAlloc(edges, MEMF_PUBLIC);

  return object;
}

__regargs void DeleteObject3D(Object3D *object) {
  WORD vertices = object->mesh->vertices;
  WORD faces = object->mesh->faces;
  WORD edges = object->mesh->edges;

  MemFree(object->edgeFlags, edges);
  MemFree(object->faceFlags, faces);
  MemFree(object->faceNormal, sizeof(Point3D) * faces);
  MemFree(object->point, sizeof(Point2D) * vertices);
  MemFree(object->vertexFlags, vertices);
  MemFree(object->vertex, sizeof(Point3D) * vertices);
  MemFree(object, sizeof(Object3D));
}

/*
 * For given triangle T with vertices A, B and C, surface normal N is a cross
 * product between vectors AB and BC.
 *
 * Ordering of vertices in polygon description is meaningful - depending on
 * that the normal vector will be directed inwards or outwards.
 *
 * Clockwise convention is used.
 */

__regargs void UpdateFaceNormals(Object3D *object) {
  Point3D *point = object->vertex;
  IndexListT **faces = object->mesh->face;
  WORD *normal = (WORD *)object->faceNormal;
  IndexListT *face;

  while ((face = *faces++)) {
    WORD *v = face->indices;

    Point3D *p1 = &point[*v++];
    Point3D *p2 = &point[*v++];
    Point3D *p3 = &point[*v++];

    WORD ax = p1->x - p2->x;
    WORD ay = p1->y - p2->y;
    WORD az = p1->z - p2->z;
    WORD bx = p2->x - p3->x;
    WORD by = p2->y - p3->y;
    WORD bz = p2->z - p3->z;

    *normal++ = normfx(ay * bz - by * az);
    *normal++ = normfx(az * bx - bz * ax);
    *normal++ = normfx(ax * by - bx * ay);
  }
}
