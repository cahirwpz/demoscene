#ifndef __ENGINE_MESH_H__
#define __ENGINE_MESH_H__

#include "std/types.h"
#include "gfx/vector3d.h"

typedef struct Triangle {
  uint16_t p1, p2, p3;
} TriangleT;

typedef struct Mesh {
  size_t vertex_count;
  size_t triangle_count;

  Vector3D *vertex;
  TriangleT *triangle;
} MeshT;

MeshT *NewMesh(size_t vertices, size_t triangles);
MeshT *NewMeshFromFile(const char *fileName);
void DeleteMesh(MeshT *mesh);
void NormalizeMeshSize(MeshT *mesh);
void CenterMeshPosition(MeshT *mesh);

#define RSC_MESH_FILE(NAME, FILENAME) { \
  void *_alloc() { return NewMeshFromFile(FILENAME); } \
  AddSimpleResource(NAME, _alloc, (FreeFuncT)DeleteMesh); \
}

#endif
