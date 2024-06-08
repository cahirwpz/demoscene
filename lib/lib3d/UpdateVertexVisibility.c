#include <strings.h>
#include <3d.h>

void UpdateVertexVisibility(Object3D *object) {
  char *vertexFlags = &object->vertex[0].flags;
  short **vertexIndexList = object->faceVertexIndexList;

  register short n asm("d2") = object->faces - 1;
  register char s asm("d3") = 1;

  do {
    short *vertexIndex = *vertexIndexList++;

    if (vertexIndex[FV_FLAGS] >= 0) {
      short m = vertexIndex[FV_COUNT] - 3;
      short i;

      /* Face has at least (and usually) three vertices / edges. */
      i = *vertexIndex++; vertexFlags[i] = s;
      i = *vertexIndex++; vertexFlags[i] = s;

      do {
        i = *vertexIndex++; vertexFlags[i] = s;
      } while (--m != -1);
    }
  } while (--n != -1);
}
