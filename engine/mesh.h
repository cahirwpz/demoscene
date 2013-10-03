#ifndef __ENGINE_MESH_H__
#define __ENGINE_MESH_H__

#include "std/types.h"
#include "engine/vector3d.h"
#include "gfx/palette.h"

typedef struct Edge {
  uint16_t p[2];
} EdgeT;

typedef struct Triangle {
  uint16_t surface;
  uint16_t p[3];
  uint16_t e[3];
} TriangleT;

typedef struct IndexArray {
  uint16_t count;
  uint16_t *index;
} IndexArrayT;

typedef struct IndexMap {
  IndexArrayT *vertex;
  uint16_t *indices;
} IndexMapT;

typedef struct Surface {
  char *name;
  bool sideness;
  union {
    RGB rgb;
    uint8_t clut;
  } color;
} SurfaceT;

typedef struct Mesh {
  size_t vertexNum;
  size_t polygonNum;
  size_t surfaceNum;
  size_t edgeNum;

  Vector3D *vertex;
  TriangleT *polygon;
  SurfaceT *surface;
  EdgeT *edge;

  /* map from vertex index to list of polygon indices */
  IndexMapT vertexToPoly;

  /* useful for lighting and backface culling */
  Vector3D *surfaceNormal;
  Vector3D *vertexNormal;
} MeshT;

MeshT *NewMesh(size_t vertices, size_t triangles, size_t surfaces);
MeshT *NewMeshFromFile(const char *fileName);
void NormalizeMeshSize(MeshT *mesh);
void CenterMeshPosition(MeshT *mesh);

void CalculateSurfaceNormals(MeshT *mesh);
void CalculateVertexToPolygonMap(MeshT *mesh);
void CalculateVertexNormals(MeshT *mesh);

void MeshApplyPalette(MeshT *mesh, PaletteT *palette);

#endif
