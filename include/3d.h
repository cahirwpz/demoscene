#ifndef __3D_H__
#define __3D_H__

#include <sort.h>
#include <2d.h>
#include <pixmap.h>

extern char SqrtTab8[256];

/* 3D transformations */

typedef struct {
  short x, y, z;
  char flags; /* remember to reset after use */
  char pad;
} Point3D;  /* sizeof(Point3D) = 8, for easy indexing */

typedef struct Edge {
  Point3D *point[2];
  char flags; /* remember to reset after use */
  char pad[7];
} EdgeT; /* sizeof(EdgeT) = 16, for easy indexing */

typedef struct {
  short m00, m01, m02, x;
  short m10, m11, m12, y;
  short m20, m21, m22, z;
} Matrix3D;

void LoadIdentity3D(Matrix3D *M);
void Translate3D(Matrix3D *M, short x, short y, short z);
void Scale3D(Matrix3D *M, short sx, short sy, short sz);
void LoadRotate3D(Matrix3D *M, short ax, short ay, short az);
void LoadReverseRotate3D(Matrix3D *M, short ax, short ay, short az);
void Compose3D(Matrix3D *md, Matrix3D *ma, Matrix3D *mb);
void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, short n);

/* 3D mesh representation */

typedef struct Mesh3D {
  short vertices;
  short faces;
  short edges;

  short *vertex;
  short *faceNormal;
  short *edge;       /* [vertex_0 vertex_1] */
  short *faceVertex; /* [#vertices vertices...] */
  short *faceEdge;   /* [#edge edges...] */
} Mesh3D;

/* 3D object representation */

/* '|' indicates 0 offset */
#define FV_FLAGS -2 /* offset to flags in faceVertexIndexList */
#define FV_COUNT -1 /* offset to #vertices in faceVertexIndexList */
#define FE_COUNT -1 /* offset to #edges in faceEdgeIndexList */

typedef struct Object3D {
  Point3D rotate;
  Point3D scale;
  Point3D translate;

  Matrix3D objectToWorld; /* object -> world transformation */
  Matrix3D worldToObject; /* world -> object transformation */

  /* camera position in object space */
  Point3D camera;

  /* potentially shared between objects, copied from mesh */
  short faces;
  short vertices;
  short edges;

  Point3D *point;
  short **faceVertexIndexList; /* [#vertices | vertex-indices...] */
  short **faceEdgeIndexList;   /* [#edges | edge-indices...] */
  Point3D *faceNormal;

  /* private */
  EdgeT *edge;
  Point3D *vertex;     /* camera coordinates or screen coordinates + depth */

  SortItemT *visibleFace;
  short visibleFaces;
} Object3D;

Object3D *NewObject3D(Mesh3D *mesh);
void DeleteObject3D(Object3D *object);
void UpdateFaceNormals(Object3D *object);
void UpdateObjectTransformation(Object3D *object);
void UpdateFaceVisibility(Object3D *object);
void UpdateVertexVisibility(Object3D *object);
void SortFaces(Object3D *object);

#endif
