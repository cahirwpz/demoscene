#include <debug.h>
#include <3d.h>
#include <fx.h>
#include <system/memory.h>

/*
 * For given triangle T with vertices A, B and C, surface normal N is a cross
 * product between vectors AB and BC.
 *
 * Ordering of vertices in polygon description is meaningful - depending on
 * that the normal vector will be directed inwards or outwards.
 *
 * Clockwise convention is used.
 */

void CalculateFaceNormals(Mesh3D *mesh) {
  mesh->faceNormal = MemAlloc(sizeof(Point3D) * mesh->faces, MEMF_PUBLIC);

  {
    Point3D *vertex = mesh->vertex;
    short *normal = (short *)mesh->faceNormal;
    short **faces = mesh->face;
    short *face;

    while ((face = *faces++)) {
      Point3D *p1 = &vertex[*face++];
      Point3D *p2 = &vertex[*face++];
      Point3D *p3 = &vertex[*face++];

      int x, y, z;
      short l;

      {
        short ax = p1->x - p2->x;
        short ay = p1->y - p2->y;
        short az = p1->z - p2->z;
        short bx = p2->x - p3->x;
        short by = p2->y - p3->y;
        short bz = p2->z - p3->z;

        x = ay * bz - by * az;
        y = az * bx - bz * ax;
        z = ax * by - bx * ay;
      }

      {
        short nx = normfx(x);
        short ny = normfx(y);
        short nz = normfx(z);

        l = isqrt(nx * nx + ny * ny + nz * nz);
      }

      if (l == 0)
        Panic("[3D] #%ld face normal vector has zero length!\n",
              (ptrdiff_t)(faces - mesh->face));

      /* Normal vector has a unit length. */
      *normal++ = div16(x, l);
      *normal++ = div16(y, l);
      *normal++ = div16(z, l);
      normal++;
    }
  }
}
