#include <3d.h>
#include <fx.h>
#include <system/memory.h>

Object3D *NewObject3D(Mesh3D *mesh) {
  Object3D *object = MemAlloc(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);

  object->objdat = (void *)mesh->data;
  object->vertexGroups= mesh->vertexGroups;
  object->edgeGroups = mesh->edgeGroups;
  object->faceGroups = mesh->faceGroups;
  object->objects = mesh->objects;

  object->visibleFace =
    MemAlloc(sizeof(SortItemT) * (mesh->faces + 1), MEMF_PUBLIC);

  object->scale.x = fx12f(1.0);
  object->scale.y = fx12f(1.0);
  object->scale.z = fx12f(1.0); 

  return object;
}
