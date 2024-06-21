#include <3d.h>
#include <system/memory.h>

void DeleteObject3D(Object3D *object) {
  if (object) {
    MemFree(object->visibleFace);
    MemFree(object);
  }
}
