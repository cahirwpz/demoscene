#include <3d.h>

void UpdateFaceVisibility(Object3D *object) {
  short *src = (short *)object->faceNormal;
  short **vertexIndexList = object->faceVertexIndexList;
  void *point = object->point;
  short n = object->faces;
  char *sqrt = SqrtTab8;

  short *camera = (short *)&object->camera;

  while (--n >= 0) {
    short *vertexIndex = *vertexIndexList++;
    short px, py, pz;
    short l;
    int f;

    {
      short i = *vertexIndex << 3;
      short *p = (short *)(point + i);
      short *c = camera;
      px = *c++ - *p++;
      py = *c++ - *p++;
      pz = *c++ - *p++;
    }

    {
      int x = *src++ * px;
      int y = *src++ * py;
      int z = *src++ * pz;
      f = x + y + z;
    }

    src++;

    if (f >= 0) {
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
      f = swap16(f); /* f >>= 16, ignore upper word */
      l = div16((short)f * (short)f, s);
      if (l >= 256)
        l = 15;
      else
        l = sqrt[l];
    } else {
      l = -1;
    }

    vertexIndex[FV_FLAGS] = l;
  }
}
