#include "3d.h"
#include "fx.h"
#include "memory.h"
#include "qsort.h"

__regargs Mesh3D *NewMesh3D(WORD vertices, WORD faces) {
  Mesh3D *mesh = MemAlloc(sizeof(Mesh3D), MEMF_PUBLIC|MEMF_CLEAR);

  mesh->vertices = vertices;
  mesh->faces = faces;

  mesh->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);
  mesh->face = MemAlloc(sizeof(IndexListT *) * (faces + 1), MEMF_PUBLIC|MEMF_CLEAR);
  mesh->faceNormal = MemAlloc(sizeof(Point3D) * faces , MEMF_PUBLIC);

  return mesh;
}

__regargs void DeleteMesh3D(Mesh3D *mesh) {
  WORD vertices = mesh->vertices;
  WORD faces = mesh->faces;
  WORD edges = mesh->edges;

  MemFreeAuto(mesh->vertexFaceData);
  MemFree(mesh->vertexFace, sizeof(IndexListT *) * (vertices + 1));
  MemFree(mesh->faceNormal, sizeof(Point3D) * faces);
  MemFreeAuto(mesh->faceEdgeData);
  MemFree(mesh->faceEdge, sizeof(IndexListT *) * (faces + 1));
  MemFreeAuto(mesh->faceData);
  MemFree(mesh->face, sizeof(IndexListT *) * (faces + 1));
  MemFree(mesh->edge, sizeof(EdgeT) * edges);
  MemFree(mesh->vertex, sizeof(Point3D) * vertices);
  MemFree(mesh, sizeof(Mesh3D));
}

typedef struct ExtEdge {
  WORD p0, p1;
  WORD face, edge;
} ExtEdgeT;

static __regargs LONG EdgeCompare(APTR a, APTR b) {
  EdgeT *e0 = a;
  EdgeT *e1 = b;

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

__regargs void CalculateEdges(Mesh3D *mesh) {
  ExtEdgeT *edge;
  WORD count, edges;

  /* Count edges. */
  {
    IndexListT **faces = mesh->face;
    IndexListT *face;

    count = 0;

    while ((face = *faces++))
      count += face->count;

    edge = MemAllocAuto(sizeof(ExtEdgeT) * count, MEMF_PUBLIC);
  }

  /* Create all edges. */
  {
    IndexListT **faces = mesh->face;
    IndexListT *face;
    WORD *e = (WORD *)edge;
    WORD i = 0;

    while ((face = *faces++)) {
      WORD *vertex = face->indices;
      WORD n = face->count;
      WORD p0 = vertex[n-1] * sizeof(Point2D);
      WORD p1;

      while (--n >= 0) {
        p1 = *vertex++ * sizeof(Point2D);

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
    WORD n = count;

    edges = 0;
    head->edge = 0;

    while (--n > 0) {
      if (EdgeCompare(head++, next++))
        edges++;
      head->edge = edges;
    }
  }

  Log("Object has %ld edges.\n", (LONG)edges);

  mesh->edge = MemAlloc(sizeof(EdgeT) * edges, MEMF_PUBLIC);
  mesh->edges = edges;

  /* Copy unique edges to the array. */
  {
    EdgeT *dst = mesh->edge;
    ExtEdgeT *head = edge;
    ExtEdgeT *next = edge + 1;
    WORD n = count;

    while (--n > 0) {
      if (EdgeCompare(head, next))
        *dst++ = *(EdgeT *)head;
      head++; next++;
    }
  }

  /* Construct { #face => [#edge] } map. */
  mesh->faceEdge =
    MemAlloc(sizeof(IndexListT *) * (mesh->faces + 1), MEMF_PUBLIC|MEMF_CLEAR);
  mesh->faceEdgeData =
    MemAllocAuto(sizeof(WORD) * (count + mesh->faces), MEMF_PUBLIC|MEMF_CLEAR);

  /* Set up pointers. */
  {
    IndexListT **faceEdges = mesh->faceEdge;
    WORD *faceEdgeData = mesh->faceEdgeData;
    IndexListT **faces = mesh->face;
    IndexListT *face;

    while ((face = *faces++)) {
      *faceEdges++ = (IndexListT *)faceEdgeData;
      faceEdgeData += face->count + 1;
    }
  }

  /* Fill in the data. */
  {
    IndexListT **faceEdges = mesh->faceEdge;
    ExtEdgeT *e = edge;
    WORD n = count;

    while (--n >= 0) {
      IndexListT *faceEdge = faceEdges[e->face];
      faceEdge->indices[faceEdge->count++] = e->edge;
      e++;
    }
  }

  MemFreeAuto(edge);
}

/*
 * Calculates a map from vertex index into a list of face the vertex belongs
 * to.  mesh->face can be considered as a map from face number to face
 * vertices, so this procedure calculates a reverse map.
 */
__regargs void CalculateVertexFaceMap(Mesh3D *mesh) {
  WORD *faceCount = MemAlloc(sizeof(WORD) * mesh->vertices,
                             MEMF_PUBLIC|MEMF_CLEAR);

  /* 
   * Count the size of the { vertex => face } map and for each vertex a
   * number of faces it belongs to.
   */
  {
    IndexListT **faces = mesh->face;
    IndexListT *face;
    WORD count = 0;

    while ((face = *faces++)) {
      WORD n = face->count;
      WORD *v = face->indices;

      count += n;
      while (--n >= 0) {
        WORD k = *v++;
        faceCount[k]++;
      }
    }

    count += mesh->vertices;

    mesh->vertexFace = MemAlloc(sizeof(IndexListT *) * (mesh->vertices + 1),
                                MEMF_PUBLIC|MEMF_CLEAR);
    mesh->vertexFaceData = MemAllocAuto(sizeof(WORD) * count,
                                        MEMF_PUBLIC|MEMF_CLEAR);
  }

  /* Set up map pointers. */
  {
    IndexListT **vertexFaces = mesh->vertexFace;
    WORD *data = mesh->vertexFaceData;
    WORD *count = faceCount;
    WORD n = mesh->vertices;

    while (--n >= 0) {
      *vertexFaces++ = (IndexListT *)data;
      data += *count++ + 1;
    }
  }

  MemFree(faceCount, sizeof(WORD) * mesh->vertices);

  /* Finally, fill in the map. */
  {
    IndexListT **vertexFaces = mesh->vertexFace;
    IndexListT **faces = mesh->face;
    IndexListT *face;
    WORD i = 0;

    while ((face = *faces++)) {
      WORD n = face->count;
      WORD *v = face->indices;

      while (--n >= 0) {
        IndexListT *vf = vertexFaces[*v++];
        vf->indices[vf->count++] = i;
      }

      i++;
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

__regargs void CalculateFaceNormals(Mesh3D *mesh) {
  Point3D *vertex = mesh->vertex;
  IndexListT **faces = mesh->face;
  WORD *normal = (WORD *)mesh->faceNormal;
  IndexListT *face;

  while ((face = *faces++)) {
    WORD *v = face->indices;

    Point3D *p1 = &vertex[*v++];
    Point3D *p2 = &vertex[*v++];
    Point3D *p3 = &vertex[*v++];

    LONG x, y, z;
    WORD l;

    {
      WORD ax = p1->x - p2->x;
      WORD ay = p1->y - p2->y;
      WORD az = p1->z - p2->z;
      WORD bx = p2->x - p3->x;
      WORD by = p2->y - p3->y;
      WORD bz = p2->z - p3->z;

      x = ay * bz - by * az;
      y = az * bx - bz * ax;
      z = ax * by - bx * ay;
    }

    {
      WORD nx = normfx(x);
      WORD ny = normfx(y);
      WORD nz = normfx(z);

      l = isqrt(nx * nx + ny * ny + nz * nz);
    }

    /* Normal vector has a unit length. */
    *normal++ = div16(x, l);
    *normal++ = div16(y, l);
    *normal++ = div16(z, l);
  }
}
