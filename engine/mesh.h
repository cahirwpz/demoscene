#ifndef __ENGINE_MESH_H__
#define __ENGINE_MESH_H__

#include "std/types.h"
#include "gfx/vector3d.h"

typedef struct Triangle {
  uint16_t p1, p2, p3;
} TriangleT;

typedef struct IndexArray {
  uint16_t count;
  uint16_t *index;
} IndexArrayT;

typedef struct IndexMap {
  IndexArrayT *vertex;
  uint16_t *indices;
} IndexMapT;

typedef struct Mesh {
  size_t vertexNum;
  size_t polygonNum;

  Vector3D *vertex;
  TriangleT *polygon;

  /* map from vertex index to list of polygon indices */
  IndexMapT vertexToPoly;

  /* useful for lighting and backface culling */
  Vector3D *surfaceNormal;
  Vector3D *vertexNormal;
} MeshT;

MeshT *NewMesh(size_t vertices, size_t triangles);
MeshT *NewMeshFromFile(const StrT fileName);
void DeleteMesh(MeshT *mesh);
void NormalizeMeshSize(MeshT *mesh);
void CenterMeshPosition(MeshT *mesh);

void CalculateSurfaceNormals(MeshT *mesh);
void CalculateVertexToPolygonMap(MeshT *mesh);
void CalculateVertexNormals(MeshT *mesh);

#define RSC_MESH_FILE(NAME, FILENAME) \
  AddRscSimple(NAME, NewMeshFromFile(FILENAME), (FreeFuncT)DeleteMesh)

#endif
