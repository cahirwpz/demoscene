#include <debug.h>
#include <3d.h>
#include <stdlib.h>
#include <system/memory.h>

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
    short **faces = mesh->face;
    short *face;

    count = 0;

    while ((face = *faces++))
      count += face[-1];

    edge = MemAlloc(sizeof(ExtEdgeT) * count, MEMF_PUBLIC);
  }

  /* Create all edges. */
  {
    short **faces = mesh->face;
    short *face;
    short *e = (short *)edge;
    short i = 0;

    while ((face = *faces++)) {
      short n = face[-1];
      short p0 = face[n-1] * sizeof(Point3D);
      short p1;

      while (--n >= 0) {
        p1 = *face++ * sizeof(Point3D);

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
    short **faces = mesh->face;
    short *face;

    mesh->faceEdge = faceEdges;

    while ((face = *faces++)) {
      *faceEdges++ = (IndexListT *)faceEdgeData;
      faceEdgeData += face[-1] + 1;
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
