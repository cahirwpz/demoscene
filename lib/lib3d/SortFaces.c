#include <3d.h>

void SortFaces(Object3D *object) {
  IndexListT **faces = object->mesh->face;
  short n = object->mesh->faces;
  void *point = object->vertex;
  char *faceFlags = object->faceFlags;
  short count = 0;
  short index = 0;

  short *item = (short *)object->visibleFace;

  while (--n >= 0) {
    IndexListT *face = *faces++;

    if (*faceFlags++ >= 0) {
      short *vi = face->indices;
      short i1 = *vi++ << 3;
      short i2 = *vi++ << 3;
      short i3 = *vi++ << 3;
      short z = 0;

      z += *(short *)(point + i1 + 4);
      z += *(short *)(point + i2 + 4);
      z += *(short *)(point + i3 + 4);

      *item++ = z;
      *item++ = index;
      count++;
    }

    index++;
  }

  object->visibleFaces = count;

  SortItemArray(object->visibleFace, count);
}
