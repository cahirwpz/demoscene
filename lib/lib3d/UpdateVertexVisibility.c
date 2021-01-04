#include <strings.h>
#include <3d.h>

void UpdateVertexVisibility(Object3D *object) {
  char *vertexFlags = object->vertexFlags;
  char *faceFlags = object->faceFlags;
  IndexListT **faces = object->mesh->face;
  short n = object->mesh->faces;

  bzero(vertexFlags, object->mesh->vertices);

  while (--n >= 0) {
    IndexListT *face = *faces++;

    if (*faceFlags++ >= 0) {
      short *vi = face->indices;
      short count = face->count;

      /* Face has at least (and usually) three vertices. */
      switch (count) {
        case 6: vertexFlags[*vi++] = -1;
        case 5: vertexFlags[*vi++] = -1;
        case 4: vertexFlags[*vi++] = -1;
        case 3: vertexFlags[*vi++] = -1;
                vertexFlags[*vi++] = -1;
                vertexFlags[*vi++] = -1;
        default: break;
      }
    }
  }
}
