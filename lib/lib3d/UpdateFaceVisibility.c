#include <3d.h>

void UpdateFaceVisibility(Object3D *object) {
  short cx = object->camera.x;
  short cy = object->camera.y;
  short cz = object->camera.z;
  char *sqrt = SqrtTab8;

  void *_objdat = object->objdat;
  short *group = object->faceGroups;
  short f;

  do {
    while ((f = *group++)) {
      short px, py, pz;
      int v;

      {
        short i = FACE(f)->indices[0].vertex;
        short *p = (short *)POINT(i);
        px = cx - *p++;
        py = cy - *p++;
        pz = cz - *p++;
      }

      {
        short *fn = FACE(f)->normal;
        int x = *fn++ * px;
        int y = *fn++ * py;
        int z = *fn++ * pz;
        v = x + y + z;
      }

      if (v >= 0) {
        short l;
        /* normalize dot product */
#if 0
        int s = px * px + py * py + pz * pz;
        s = swap16(s); /* s >>= 16, ignore upper word */
#else
        short s;
        asm("mulsw %0,%0\n"
            "mulsw %1,%1\n"
            "mulsw %2,%2\n"
            "addl  %1,%0\n"
            "addl  %2,%0\n"
            "swap  %0\n"
            : "+d" (px), "+d" (py), "+d" (pz));
        s = px;
#endif
        v = swap16(v); /* f >>= 16, ignore upper word */
        l = div16((short)v * (short)v, s);
        FACE(f)->flags = (l >= 256) ? 15 : sqrt[l];
      } else {
        FACE(f)->flags = -1;
      }
    }
  } while (*group);
}
