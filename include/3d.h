#ifndef __3D_H__
#define __3D_H__

#include <sort.h>
#include <2d.h>
#include <pixmap.h>

extern char SqrtTab8[256];

/* 3D transformations */

typedef struct {
  short u, v;
} UVCoord;

typedef struct {
  short x, y, z;
  short pad;
} Point3D;

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

/* 3D polygon and line clipping */

#define PF_NEAR 16
#define PF_FAR  32

typedef struct {
  short near;
  short far;
} Frustum3D;

extern Frustum3D ClipFrustum;

void PointsInsideFrustum(Point3D *in, u_char *flags, u_short n);
u_short ClipPolygon3D(Point3D *in, Point3D **outp, u_short n,
                      u_short clipFlags);

/* 3D mesh representation */

typedef struct {
  short vertices;
  short faces;
  short edges;

  Point3D *vertex;
  Point3D *faceNormal;
  Point3D *vertexNormal;
  EdgeT *edge;
  /* '|' indicates 0 offset, '[x]' is an array of x's */
  short **face;            /* { #face => [#vertices | [vertex]] } */
  IndexListT **faceEdge;   /* { #face => [#edge] } */
  IndexListT **vertexFace; /* { #vertex => [#face] } */
} Mesh3D;

void CalculateEdges(Mesh3D *mesh);
void CalculateVertexFaceMap(Mesh3D *mesh);
void CalculateVertexNormals(Mesh3D *mesh);
void CalculateFaceNormals(Mesh3D *mesh);
void ResetMesh3D(Mesh3D *mesh);

/* 3D object representation */

typedef struct {
  Mesh3D *mesh;

  Point3D rotate;
  Point3D scale;
  Point3D translate;

  Matrix3D objectToWorld; /* object -> world transformation */
  Matrix3D worldToObject; /* world -> object transformation */

  Point3D camera;      /* camera position in object space */

  Point3D *vertex;     /* camera coordinates or screen coordinates + depth */
  char *vertexFlags;   /* used by clipping */
  char *faceFlags;     /* e.g. visiblity flags */
  char *edgeFlags;

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
