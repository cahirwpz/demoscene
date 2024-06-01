#include <strings.h>
#include <3d.h>

void UpdateVertexVisibility(Object3D *object) {
  char *vertexFlags = object->vertexFlags;
  char *faceFlags = object->faceFlags;
  short **vertexIndexList = object->faceVertexIndexList;
  short n = object->faces;

  bzero(vertexFlags, object->vertices);

  while (--n >= 0) {
    short *vertexIndex = *vertexIndexList++;

    if (*faceFlags++ >= 0) {
      short count = vertexIndex[-1];

      /* Face has at least (and usually) three vertices. */
      switch (count) {
        case 6: vertexFlags[*vertexIndex++] = -1;
        case 5: vertexFlags[*vertexIndex++] = -1;
        case 4: vertexFlags[*vertexIndex++] = -1;
        case 3: vertexFlags[*vertexIndex++] = -1;
                vertexFlags[*vertexIndex++] = -1;
                vertexFlags[*vertexIndex++] = -1;
        default: break;
      }
    }
  }
}
