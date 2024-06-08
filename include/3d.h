#ifndef __3D_H__
#define __3D_H__

#include <sort.h>
#include <2d.h>
#include <pixmap.h>

extern char SqrtTab8[256];

/* 3D transformations */

typedef struct {
  short x, y, z;
  /* one if vertex belongs to a face that is visible,
   * otherwise set to zero,
   * remember to set to 0 after use */
  char flags;
} Point3D;

typedef struct Edge {
  /* negative if edge belongs to a face that is not visible,
   * otherwise it's an edge color in range 0..15
   * remember to set to -1 after use */
  short flags;
  short point[2];
} EdgeT;

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

/* Flags store negative value when face is not visible,
 * or a color of face normalized to 0..15 range. */
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
  /* '|' indicates 0 offset */
  short **faceVertexIndexList; /* [flags #vertices | vertex-indices...] */
  short **faceEdgeIndexList;   /* [#edges | edge-indices...] */
  Point3D *faceNormal;

  /* private */
  EdgeT *edge;
  Point3D *vertex;     /* camera coordinates or screen coordinates + depth */

  /* ends with guard element */
  SortItemT *visibleFace;
} Object3D;

Object3D *NewObject3D(Mesh3D *mesh);
void DeleteObject3D(Object3D *object);
void UpdateFaceNormals(Object3D *object);
void UpdateObjectTransformation(Object3D *object);
void UpdateFaceVisibility(Object3D *object);
void UpdateVertexVisibility(Object3D *object);
void SortFaces(Object3D *object);

#endif
