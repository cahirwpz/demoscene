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

  return mesh;
}

__regargs void DeleteMesh3D(Mesh3D *mesh) {
  MemFreeAuto(mesh->vertexFaceData);
  MemFree(mesh->vertexFace, sizeof(IndexListT *) * (mesh->vertices + 1));
  MemFreeAuto(mesh->faceData);
  MemFree(mesh->face, sizeof(IndexListT *) * (mesh->faces + 1));
  MemFree(mesh->edge, sizeof(EdgeT) * mesh->edges);
  MemFree(mesh->vertex, sizeof(Point3D) * mesh->vertices);
  MemFree(mesh, sizeof(Mesh3D));
}

typedef struct ExtEdge {
  WORD p0, p1;
  WORD poly;
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
      WORD p0 = vertex[n-1];
      WORD p1;

      while (--n >= 0) {
        p1 = *vertex++;

        /* Make sure lower index is first. */
        if (p0 > p1) {
          *e++ = p0; *e++ = p1;
        } else {
          *e++ = p1; *e++ = p0;
        }
        *e++ = i;

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

    edges = 1;

    while (--n > 0)
      if (EdgeCompare(head++, next++))
        edges++;
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

    *dst++ = *(EdgeT *)head;
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
