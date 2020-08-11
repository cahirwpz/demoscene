#include "debug.h"
#include "3d.h"
#include "fx.h"
#include "memory.h"
#include "qsort.h"

typedef struct ExtEdge {
  short p0, p1;
  short face, edge;
} ExtEdgeT;

static int EdgeCompare(const void *a, const void *b) {
  const EdgeT *e0 = a;
  const EdgeT *e1 = b;

  if (e0->p0 < e1->p0)
    return -1;
  if (e0->p0 > e1->p0)
    return 1;

  if (e0->p1 < e1->p1)
    return -1;
  if (e0->p1 > e1->p1)
    return 1;

  return 0;
}

void CalculateEdges(Mesh3D *mesh) {
  ExtEdgeT *edge;
  short count, edges;

  /* Count edges. */
  {
    IndexListT **faces = mesh->face;
    IndexListT *face;

    count = 0;

    while ((face = *faces++))
      count += face->count;

    edge = MemAlloc(sizeof(ExtEdgeT) * count, MEMF_PUBLIC);
  }

  /* Create all edges. */
  {
    IndexListT **faces = mesh->face;
    IndexListT *face;
    short *e = (short *)edge;
    short i = 0;

    while ((face = *faces++)) {
      short *vertex = face->indices;
      short n = face->count;
      short p0 = vertex[n-1] * sizeof(Point3D);
      short p1;

      while (--n >= 0) {
        p1 = *vertex++ * sizeof(Point3D);

        /* Make sure lower index is first. */
        if (p0 > p1) {
          *e++ = p1; *e++ = p0;
        } else {
          *e++ = p0; *e++ = p1;
        }
        *e++ = i;
        *e++ = 0;

        p0 = p1;
      }

      i++;
    }
  }

  /* Sort the edges lexicographically. */
  qsort(edge, count, sizeof(ExtEdgeT), EdgeCompare);

  /* Count unique edges. */
  {
    ExtEdgeT *head = edge;
    ExtEdgeT *next = edge + 1;
    short n = count;

    edges = 0;
    head->edge = 0;

    while (--n > 0) {
      if (EdgeCompare(head++, next++))
        edges++;
      head->edge = edges;
    }

    edges++;
  }

  Log("[3D] Mesh has %d edges\n", edges);

  mesh->edge = MemAlloc(sizeof(EdgeT) * edges, MEMF_PUBLIC);
  mesh->edges = edges;

  /* Copy unique edges to the array. */
  {
    EdgeT *dst = mesh->edge;
    ExtEdgeT *head = edge;
    ExtEdgeT *next = edge + 1;
    short n = count;

    *dst++ = *(EdgeT *)head;

    while (--n > 0) {
      if (EdgeCompare(head, next))
        *dst++ = *(EdgeT *)next;
      head++; next++;
    }
  }

  /* Construct { #face => [#edge] } map. */

  /* Set up pointers. */
  {
    IndexListT **faceEdges = MemAlloc(sizeof(IndexListT *) * (mesh->faces + 1) +
                                      sizeof(short) * (count + mesh->faces),
                                      MEMF_PUBLIC|MEMF_CLEAR);
    short *faceEdgeData = (short *)&faceEdges[mesh->faces + 1];
    IndexListT **faces = mesh->face;
    IndexListT *face;

    mesh->faceEdge = faceEdges;

    while ((face = *faces++)) {
      *faceEdges++ = (IndexListT *)faceEdgeData;
      faceEdgeData += face->count + 1;
    }
  }

  /* Fill in the data. */
  {
    IndexListT **faceEdges = mesh->faceEdge;
    ExtEdgeT *e = edge;
    short n = count;

    while (--n >= 0) {
      IndexListT *faceEdge = faceEdges[e->face];
      faceEdge->indices[faceEdge->count++] = e->edge;
      e++;
    }
  }

  MemFree(edge);
}

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
    IndexListT **faces = mesh->face;
    short *normal = (short *)mesh->faceNormal;
    IndexListT *face;

    while ((face = *faces++)) {
      short *v = face->indices;

      Point3D *p1 = &vertex[*v++];
      Point3D *p2 = &vertex[*v++];
      Point3D *p3 = &vertex[*v++];

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

void ResetMesh3D(Mesh3D *mesh) {
  MemFree(mesh->faceNormal);
  MemFree(mesh->vertexNormal);
  MemFree(mesh->edge);
  MemFree(mesh->faceEdge);
  MemFree(mesh->vertexFace);

  mesh->edges = 0;
  mesh->faceNormal = NULL;
  mesh->vertexNormal = NULL;
  mesh->edge = NULL;
  mesh->faceEdge = NULL;
  mesh->vertexFace = NULL;
}
