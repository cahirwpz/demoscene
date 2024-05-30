#include <strings.h>
#include <3d.h>

void UpdateVertexVisibility(Object3D *object) {
  char *vertexFlags = object->vertexFlags;
  char *faceFlags = object->faceFlags;
  short **faces = object->mesh->face;
  short n = object->mesh->faces;

  bzero(vertexFlags, object->mesh->vertices);

  while (--n >= 0) {
    short *face = *faces++;

    if (*faceFlags++ >= 0) {
      short count = face[-1];

      /* Face has at least (and usually) three vertices. */
      switch (count) {
        case 6: vertexFlags[*face++] = -1;
        case 5: vertexFlags[*face++] = -1;
        case 4: vertexFlags[*face++] = -1;
        case 3: vertexFlags[*face++] = -1;
                vertexFlags[*face++] = -1;
                vertexFlags[*face++] = -1;
        default: break;
      }
    }
  }
}
