#include <memory.h>
#include <3d.h>

void DeleteObject3D(Object3D *object) {
  if (object) {
    MemFree(object->visibleFace);
    MemFree(object->edgeFlags);
    MemFree(object->faceFlags);
    MemFree(object->vertexFlags);
    MemFree(object->vertex);
    MemFree(object);
  }
}
