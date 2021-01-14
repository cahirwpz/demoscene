#include <3d.h>
#include <memory.h>

void ResetMesh3D(Mesh3D *mesh) {
  MemFree(mesh->faceNormal);
  MemFree(mesh->vertexNormal);
  MemFree(mesh->edge);
  MemFree(mesh->faceEdge);
  MemFree(mesh->vertexFace);

  mesh->edges = 0;
  mesh->faceNormal = NULL;
  mesh->vertexNormal = NULL;
  mesh->edge = NULL;
  mesh->faceEdge = NULL;
  mesh->vertexFace = NULL;
}
