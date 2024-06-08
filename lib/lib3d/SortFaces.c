#include <3d.h>

void SortFaces(Object3D *object) {
  short **vertexIndexList = object->faceVertexIndexList;
  short n = object->faces;
  void *vertex = object->vertex;
  short count = 0;
  short index = 0;

  short *item = (short *)object->visibleFace;

  while (--n >= 0) {
    short *vertexIndex = *vertexIndexList++;

    if (vertexIndex[FV_FLAGS] >= 0) {
      short z;
      short i;

      i = *vertexIndex++;
      z = ((Point3D *)(vertex + i))->z;
      i = *vertexIndex++;
      z += ((Point3D *)(vertex + i))->z;
      i = *vertexIndex++;
      z += ((Point3D *)(vertex + i))->z;

      *item++ = z;
      *item++ = index;
      count++;
    }

    index++;
  }

  /* guard element */
  *item++ = 0;
  *item++ = -1;

  SortItemArray(object->visibleFace, count);
}
