#include <3d.h>
#include <fx.h>

#define MULROW() {             \
  int t0 = (*a++) * (*b0++);   \
  int t1 = (*a++) * (*b1++);   \
  int t2 = (*a++) * (*b2++);   \
  *d++ = normfx(t0 + t1 + t2); \
}

void Compose3D(Matrix3D *md, Matrix3D *ma, Matrix3D *mb) {
  short *a = &ma->m00;
  short *d = &md->m00;

  {
    short *b0 = &mb->m00;
    short *b1 = &mb->m10;
    short *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b0);
  }

  {
    short *b0 = &mb->m00;
    short *b1 = &mb->m10;
    short *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b1);
  }

  {
    short *b0 = &mb->m00;
    short *b1 = &mb->m10;
    short *b2 = &mb->m20;

    MULROW(); a -= 3;
    MULROW(); a -= 3;
    MULROW();

    *d++ = (*a++) + (*b2);
  }
}
