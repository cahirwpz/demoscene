#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "engine/mesh.h"

MeshT *NewMesh(size_t vertices, size_t triangles) {
  MeshT *mesh = NEW_S(MeshT);

  mesh->vertex_count = vertices;
  mesh->triangle_count = triangles;
  mesh->vertex = NEW_A(Vector3D, vertices);
  mesh->triangle = NEW_A(TriangleT, triangles);

  return mesh;
}

MeshT *NewMeshFromFile(const char *fileName, uint32_t memFlags) {
  uint16_t *data = ReadFileSimple(fileName, memFlags);

  if (data) {
    uint16_t vertices = data[0];
    uint16_t triangles = data[1];

    MeshT *mesh = NewMesh(vertices, triangles);

    Vector3D *vertexPtr = (Vector3D *)&data[2];
    TriangleT *trianglePtr = (TriangleT *)&vertexPtr[vertices];

    memcpy(mesh->vertex, vertexPtr, sizeof(Vector3D) * vertices);
    memcpy(mesh->triangle, trianglePtr, sizeof(TriangleT) * triangles);

    LOG("Mesh '%s' has %d vertices and %d triangles.",
        fileName, vertices, triangles);

    DELETE(data);

    return mesh;
  }

  return NULL;
}

void DeleteMesh(MeshT *mesh) {
  if (mesh) {
    DELETE(mesh->triangle);
    DELETE(mesh->vertex);
    DELETE(mesh);
  }
}

void CenterMeshPosition(MeshT *mesh) {
  Vector3D med = { 0.0f, 0.0f, 0.0f };
  int i;

  for (i = 0; i < mesh->vertex_count; i++)
    V3D_Add(&med, &med, &mesh->vertex[i]);

  V3D_Scale(&med, &med, 1.0f / mesh->vertex_count);

  for (i = 0; i < mesh->vertex_count; i++)
    V3D_Sub(&mesh->vertex[i], &mesh->vertex[i], &med);
}

void NormalizeMeshSize(MeshT *mesh) {
  float m = 0.0f;
  int i;

  for (i = 0; i < mesh->vertex_count; i++) {
    m = max(abs(mesh->vertex[i].x), m); 
    m = max(abs(mesh->vertex[i].y), m); 
    m = max(abs(mesh->vertex[i].z), m); 
  }

  for (i = 0; i < mesh->vertex_count; i++)
    V3D_Scale(&mesh->vertex[i], &mesh->vertex[i], 1.0f / m);
}
