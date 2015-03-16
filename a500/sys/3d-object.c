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

  object->scale.x = fx12f(1.0);
  object->scale.y = fx12f(1.0);
  object->scale.z = fx12f(1.0); 

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

__regargs void UpdateObjectTransformation(Object3D *object) {
  Point3D *rotate = &object->rotate;
  Point3D *scale = &object->scale;
  Point3D *translate = &object->translate;

  /* object -> world: Rx * Ry * Rz * S * T */
  {
    Matrix3D *m = &object->objectToWorld;
    LoadRotate3D(m, rotate->x, rotate->y, rotate->z);
    Scale3D(m, scale->x, scale->y, scale->z);
    Translate3D(m, translate->x, translate->y, translate->z);
  }

  /* world -> object: T * S * Rz * Ry * Rx */
  {
    Matrix3D *m = &object->worldToObject;
    Matrix3D m_scale, m_rotate;

    LoadIdentity3D(&m_scale);
    m_scale.m00 = div16(1 << 24, scale->x);
    m_scale.m11 = div16(1 << 24, scale->y);
    m_scale.m22 = div16(1 << 24, scale->z);

    LoadReverseRotate3D(&m_rotate, -rotate->x, -rotate->y, -rotate->z);
    Compose3D(m, &m_scale, &m_rotate);

    /*
     * Translation is formally first, and we achieve that in ReverseTransform3D.
     * Matrix3D is used only to store the vector.
     */
    Translate3D(m, -translate->x, -translate->y, -translate->z);
  }
}

__regargs void UpdateFaceVisibility(Object3D *object) {
  WORD *src = (WORD *)object->mesh->faceNormal;
  IndexListT **faces = object->mesh->face;
  BYTE *faceFlags = object->faceFlags;
  Point3D *vertex = object->mesh->vertex;
  WORD n = object->mesh->faces;
  WORD cx, cy, cz; /* camera position in object space */

  {
    Matrix3D *M = &object->worldToObject;

    /* camera position in world space is (0, 0, 1) */
    WORD x = fx4i(0) + M->x;
    WORD y = fx4i(0) + M->y;
    WORD z = fx4i(1) + M->z;

    cx = normfx(M->m00 * x + M->m01 * y + M->m02 * z);
    cy = normfx(M->m10 * x + M->m11 * y + M->m12 * z);
    cz = normfx(M->m20 * x + M->m21 * y + M->m22 * z);
  }

  while (--n >= 0) {
    IndexListT *face = *faces++;
    WORD *p = (WORD *)&vertex[face->indices[0]];
    LONG x = (*src++) * (WORD)(cx - *p++);
    LONG y = (*src++) * (WORD)(cy - *p++);
    LONG z = (*src++) * (WORD)(cz - *p++);
    BYTE f = (x + y + z >= 0) ? -1 : 0;

    *faceFlags++ = f;
  }
}
