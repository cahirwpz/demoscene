#include <strings.h>
#include <3d.h>

void UpdateVertexVisibility(Object3D *object) {
  register char s asm("d3") = 1;

  void *_objdat = object->objdat;
  register short *group asm("a2") = object->faceGroups;
  short f;

  do {
    while ((f = *group++)) {
      if (FACE(f)->flags >= 0) {
        register FaceIndexT *index asm("a3") = FACE(f)->indices;
        short vertices = FACE(f)->count - 3;
        short i;

        /* Face has at least (and usually) three vertices / edges. */
        i = index->vertex; index++; NODE3D(i)->flags = s;
        i = index->vertex; index++; NODE3D(i)->flags = s;

        do {
          i = index->vertex; index++; NODE3D(i)->flags = s;
        } while (--vertices != -1);
      }
    }
  } while (*group);
}
