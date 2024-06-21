#ifndef __3D_H__
#define __3D_H__

#include <cdefs.h>
#include <sort.h>
#include <2d.h>
#include <pixmap.h>

extern char SqrtTab8[256];

/* 3D transformations */

typedef struct Point3D {
  short x, y, z;
} Point3D;

typedef struct Node3D {
  /* one if vertex belongs to a face that is visible,
   * otherwise set to zero,
   * remember to set to 0 after use */
  char flags;
  Point3D point;
  Point3D vertex;
} Node3D;

typedef struct Edge {
  /* negative if edge belongs to a face that is not visible,
   * otherwise it's an edge color in range 0..15
   * remember to set to -1 after use */
  char flags;
  char pad;
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

/*
 * 3D mesh representation
 *
 * Please read the output from obj2c to understand it.
 */
typedef struct Mesh3D {
  short vertices;
  short edges;
  short faces;
  short materials;

  /* these arrays are shared with Object3D */
  void *data;

  short *vertexGroups;
  short *edgeGroups;
  short *faceGroups;
  short *objects;
} Mesh3D;

/* 3D object representation */

typedef struct FaceIndex {
  short vertex;
  short edge;
} FaceIndexT; 

typedef struct Face {
  /* Face normal - absent for points and lines. */
  short normal[3];
  /* Flags store negative value when face is not visible,
   * or a color of face normalized to 0..15 range. */
  char flags;
  char material;
  short count;
  FaceIndexT indices[0];
} FaceT;

typedef struct Object3D {
  /* copied from mesh */
  void *objdat;

  short *vertexGroups;
  short *edgeGroups;
  short *faceGroups;
  short *objects;

  /* private */
  Point3D rotate;
  Point3D scale;
  Point3D translate;

  Matrix3D objectToWorld; /* object -> world transformation */
  Matrix3D worldToObject; /* world -> object transformation */

  /* camera position in object space */
  Point3D camera;

  /* ends with guard element */
  SortItemT *visibleFace;
} Object3D;

/* The environment has to define `_objdat`. */
static inline void *_getptr(void *ptr, short i, const short o) {
  ptr += i + o;
  return ptr;
}

#define NODE3D(i) ((Node3D *)_getptr(_objdat, i, -2))
#define POINT(i) ((Point3D *)_getptr(_objdat, i, offsetof(Node3D, point) - 2))
#define VERTEX(i) ((Point3D *)_getptr(_objdat, i, offsetof(Node3D, vertex) - 2))
#define EDGE(i) ((EdgeT *)_getptr(_objdat, i, 0))
#define FACE(i) ((FaceT *)_getptr(_objdat, i, 0))

Object3D *NewObject3D(Mesh3D *mesh);
void DeleteObject3D(Object3D *object);
void UpdateFaceNormals(Object3D *object);
void UpdateObjectTransformation(Object3D *object);
void UpdateFaceVisibility(Object3D *object);
void UpdateVertexVisibility(Object3D *object);
void SortFaces(Object3D *object);

#endif
