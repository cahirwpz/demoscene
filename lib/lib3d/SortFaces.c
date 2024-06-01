#include <3d.h>

void SortFaces(Object3D *object) {
  short **vertexIndexList = object->faceVertexIndexList;
  short n = object->faces;
  void *vertex = object->vertex;
  char *faceFlags = object->faceFlags;
  short count = 0;
  short index = 0;

  short *item = (short *)object->visibleFace;

  while (--n >= 0) {
    short *vertexIndex = *vertexIndexList++;

    if (*faceFlags++ >= 0) {
      short z;
      short i;

      i = *vertexIndex++ << 3;
      z = ((Point3D *)(vertex + i))->z;
      i = *vertexIndex++ << 3;
      z += ((Point3D *)(vertex + i))->z;
      i = *vertexIndex++ << 3;
      z += ((Point3D *)(vertex + i))->z;

      *item++ = z;
      *item++ = index;
      count++;
    }

    index++;
  }

  object->visibleFaces = count;

  SortItemArray(object->visibleFace, count);
}
