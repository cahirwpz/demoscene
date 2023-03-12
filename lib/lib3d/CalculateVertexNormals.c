#include <3d.h>
#include <system/memory.h>

void CalculateVertexNormals(Mesh3D *mesh) {
  mesh->vertexNormal = MemAlloc(sizeof(Point3D) * mesh->vertices,
                                MEMF_PUBLIC|MEMF_CLEAR);

  {
    short *normal = (short *)mesh->vertexNormal;
    void *faceNormal = mesh->faceNormal;

    IndexListT **vertexFaces = mesh->vertexFace;
    IndexListT *vertexFace;

    while ((vertexFace = *vertexFaces++)) {
      short n = vertexFace->count;
      short *v = vertexFace->indices;
      int nx = 0;
      int ny = 0;
      int nz = 0;

      while (--n >= 0) {
        short *fn = (short *)(faceNormal + (short)(*v++ << 3));
        nx += *fn++; ny += *fn++; nz += *fn++;
      }

      *normal++ = div16(nx, n);
      *normal++ = div16(ny, n);
      *normal++ = div16(nz, n);
      normal++;
    }
  }
}
