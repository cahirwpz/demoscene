#ifndef __3D_H__
#define __3D_H__

#include "sort.h"
#include "2d.h"
#include "pixmap.h"

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

__regargs void LoadIdentity3D(Matrix3D *M);
__regargs void Translate3D(Matrix3D *M, short x, short y, short z);
__regargs void Scale3D(Matrix3D *M, short sx, short sy, short sz);
__regargs void LoadRotate3D(Matrix3D *M, short ax, short ay, short az);
__regargs void LoadReverseRotate3D(Matrix3D *M, short ax, short ay, short az);
__regargs void Compose3D(Matrix3D *md, Matrix3D *ma, Matrix3D *mb);
__regargs void Transform3D(Matrix3D *M, Point3D *out, Point3D *in, short n);

/* 3D polygon and line clipping */

#define PF_NEAR 16
#define PF_FAR  32

typedef struct {
  short near;
  short far;
} Frustum3D;

extern Frustum3D ClipFrustum;

__regargs void PointsInsideFrustum(Point3D *in, u_char *flags, u_short n);
__regargs u_short ClipPolygon3D(Point3D *in, Point3D **outp, u_short n,
                              u_short clipFlags);

/* 3D mesh representation */

typedef struct {
  PixmapT *pixmap;
  char filename[0];
} MeshImageT;

typedef struct {
  u_char r, g, b;
  u_char sideness;
  short  texture;
} MeshSurfaceT;

typedef struct {
  short vertices;
  short faces;
  short edges;
  short surfaces;
  short images;

  Point3D *vertex;
  UVCoord *uv;
  Point3D *faceNormal;
  u_char *faceSurface;
  Point3D *vertexNormal;
  EdgeT *edge;
  IndexListT **face;       /* { #face => [#vertex] } */
  IndexListT **faceEdge;   /* { #face => [#edge] } */
  IndexListT **faceUV;     /* { #face => [#uv] } */
  IndexListT **vertexFace; /* { #vertex => [#face] } */

  MeshImageT **image;
  MeshSurfaceT *surface;
} Mesh3D;

__regargs void CalculateEdges(Mesh3D *mesh);
__regargs void CalculateVertexFaceMap(Mesh3D *mesh);
__regargs void CalculateVertexNormals(Mesh3D *mesh);
__regargs void CalculateFaceNormals(Mesh3D *mesh);
__regargs void ResetMesh3D(Mesh3D *mesh);

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

__regargs Object3D *NewObject3D(Mesh3D *mesh);
__regargs void DeleteObject3D(Object3D *object);
__regargs void UpdateFaceNormals(Object3D *object);
__regargs void UpdateObjectTransformation(Object3D *object);
__regargs void UpdateFaceVisibility(Object3D *object);
__regargs void UpdateVertexVisibility(Object3D *object);
__regargs void SortFaces(Object3D *object);

#endif
