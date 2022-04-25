#include <3d.h>
#include <fx.h>
#include <system/memory.h>

Object3D *NewObject3D(Mesh3D *mesh) {
  Object3D *object = MemAlloc(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);
  short vertices = mesh->vertices;
  short faces = mesh->faces;
  short edges = mesh->edges;

  object->mesh = mesh;
  object->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);
  object->vertexFlags = MemAlloc(vertices, MEMF_PUBLIC);
  object->faceFlags = MemAlloc(faces, MEMF_PUBLIC);
  if (edges)
    object->edgeFlags = MemAlloc(edges, MEMF_PUBLIC);
  object->visibleFace = MemAlloc(sizeof(SortItemT) * faces, MEMF_PUBLIC);

  object->scale.x = fx12f(1.0);
  object->scale.y = fx12f(1.0);
  object->scale.z = fx12f(1.0); 

  return object;
}
