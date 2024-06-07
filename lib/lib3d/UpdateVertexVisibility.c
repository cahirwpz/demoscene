#include <strings.h>
#include <3d.h>

void UpdateVertexVisibility(Object3D *object) {
  char *vertexFlags = &object->vertex[0].flags;
  short **vertexIndexList = object->faceVertexIndexList;
  short n = object->faces;

  register short s asm("d7") = -1;

  while (--n >= 0) {
    short *vertexIndex = *vertexIndexList++;

    if (vertexIndex[FV_FLAGS]) {
      short count = vertexIndex[FV_COUNT];
      short i;

      /* Face has at least (and usually) three vertices. */
      switch (count) {
        case 6: i = *vertexIndex++ << 3; vertexFlags[i] = s;
        case 5: i = *vertexIndex++ << 3; vertexFlags[i] = s;
        case 4: i = *vertexIndex++ << 3; vertexFlags[i] = s;
        case 3: i = *vertexIndex++ << 3; vertexFlags[i] = s;
                i = *vertexIndex++ << 3; vertexFlags[i] = s;
                i = *vertexIndex++ << 3; vertexFlags[i] = s;
                i = *vertexIndex++ << 3; vertexFlags[i] = s;
        default: break;
      }
    }
  }
}
