#include <3d.h>
#include <system/memory.h>

/*
 * Calculates a map from vertex index into a list of face the vertex belongs
 * to.  mesh->face can be considered as a map from face number to face
 * vertices, so this procedure calculates a reverse map.
 */
void CalculateVertexFaceMap(Mesh3D *mesh) {
  short *faceCount = MemAlloc(sizeof(short) * mesh->vertices,
                             MEMF_PUBLIC|MEMF_CLEAR);

  /* 
   * Count the size of the { vertex => face } map and for each vertex a
   * number of faces it belongs to.
   */
  {
    IndexListT **faces = mesh->face;
    IndexListT *face;
    short count = 0;

    while ((face = *faces++)) {
      short n = face->count;
      short *v = face->indices;

      count += n;
      while (--n >= 0) {
        short k = *v++;
        faceCount[k]++;
      }
    }

    count += mesh->vertices;

    mesh->vertexFace = MemAlloc(sizeof(IndexListT *) * (mesh->vertices + 1) +
                                sizeof(short) * count, MEMF_PUBLIC|MEMF_CLEAR);
  }

  /* Set up map pointers. */
  {
    IndexListT **vertexFaces = mesh->vertexFace;
    short *data = (short *)&mesh->vertexFace[mesh->vertices + 1];
    short *count = faceCount;
    short n = mesh->vertices;

    while (--n >= 0) {
      *vertexFaces++ = (IndexListT *)data;
      data += *count++ + 1;
    }
  }

  MemFree(faceCount);

  /* Finally, fill in the map. */
  {
    IndexListT **vertexFaces = mesh->vertexFace;
    IndexListT **faces = mesh->face;
    IndexListT *face;
    short i = 0;

    while ((face = *faces++)) {
      short n = face->count;
      short *v = face->indices;

      while (--n >= 0) {
        IndexListT *vf = vertexFaces[*v++];
        vf->indices[vf->count++] = i;
      }

      i++;
    }
  }
} 
