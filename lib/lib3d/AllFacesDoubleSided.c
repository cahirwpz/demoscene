#include <3d.h>

void AllFacesDoubleSided(Object3D *object) {
  void *_objdat = object->objdat;
  short *group = object->faceGroups;
  short f;

  do {
    while ((f = *group++)) {
      FACE(f)->material |= 0x80;
    }
  } while (*group);
}
