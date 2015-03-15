#ifndef __3D_H__
#define __3D_H__

#include "2d.h"

/* 3D transformations */

typedef struct {
  WORD x, y, z;
} Point3D;

typedef struct {
  WORD m00, m01, m02, x;
  WORD m10, m11, m12, y;
  WORD m20, m21, m22, z;
} Matrix3D;

__regargs void LoadIdentity3D(Matrix3D *M);
__regargs void Translate3D(Matrix3D *M, WORD x, WORD y, WORD z);
__regargs void Scale3D(Matrix3D *M, WORD sx, WORD sy, WORD sz);
__regargs void LoadRotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az);
__regargs void Compose3D(Matrix3D *md, Matrix3D *ma, Matrix3D *mb);
__regargs void Rotate3D(Matrix3D *M, WORD ax, WORD ay, WORD az);
__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, WORD n);

/* 3D polygon and line clipping */

#define PF_NEAR 16
#define PF_FAR  32

typedef struct {
  WORD near;
  WORD far;
} Frustum3D;

extern Frustum3D ClipFrustum;

__regargs void PointsInsideFrustum(Point3D *in, UBYTE *flags, UWORD n);
__regargs UWORD ClipPolygon3D(Point3D *in, Point3D **outp, UWORD n,
                              UWORD clipFlags);

/* 3D mesh representation */

typedef struct {
  WORD vertices;
  WORD faces;
  WORD edges;

  Point3D *vertex;
  Point3D *faceNormal;
  EdgeT *edge;
  IndexListT **face;       /* { #face => [#vertex] } */
  WORD *faceData;
  IndexListT **faceEdge;   /* { #face => [#edge] } */
  WORD *faceEdgeData;
  IndexListT **vertexFace; /* { #vertex => [#face] } */
  WORD *vertexFaceData;
} Mesh3D;

__regargs Mesh3D *NewMesh3D(WORD vertices, WORD faces);
__regargs void DeleteMesh3D(Mesh3D *mesh);
__regargs void CalculateEdges(Mesh3D *mesh);
__regargs void CalculateVertexFaceMap(Mesh3D *mesh);
__regargs void CalculateFaceNormals(Mesh3D *mesh);
__regargs Mesh3D *LoadLWO(char *filename, FLOAT scale);

/* 3D object representation */

typedef struct {
  Matrix3D world;
  Mesh3D *mesh;

  Point3D *vertex;     /* camera coordinates */
  BYTE *vertexFlags;   /* used by clipping */
  Point2D *point;      /* screen coordinates */
  Point3D *faceNormal; /* for back-face culling and lighting */
  BYTE *faceFlags;     /* e.g. visiblity flags */
  BYTE *edgeFlags;
} Object3D;

__regargs Object3D *NewObject3D(Mesh3D *mesh);
__regargs void DeleteObject3D(Object3D *object);
__regargs void UpdateFaceNormals(Object3D *object);

#endif
