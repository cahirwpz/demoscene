#include <3d.h>
#include <system/memory.h>

void DeleteObject3D(Object3D *object) {
  if (object) {
    MemFree(object->edge);
    MemFree(object->faceVertexIndexList);
    MemFree(object->faceEdgeIndexList);
    MemFree(object->visibleFace);
    MemFree(object->vertex);
    MemFree(object);
  }
}
