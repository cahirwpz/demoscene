#include <3d.h>
#include <fx.h>
#include <system/memory.h>

Object3D *NewObject3D(Mesh3D *mesh) {
  Object3D *object = MemAlloc(sizeof(Object3D), MEMF_PUBLIC|MEMF_CLEAR);
  short vertices = mesh->vertices;
  short faces = mesh->faces;
  short edges = mesh->edges;

  object->vertices = vertices;
  object->faces = faces;
  object->edges = edges;

  object->point = mesh->vertex;
  object->faceNormal = mesh->faceNormal;

  object->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);
  object->edge = MemAlloc(sizeof(Point3D *) * 2 * edges, MEMF_PUBLIC);
  {
    short *in = (short *)mesh->edge;
    Point3D **out = &object->edge[0].p0;
    short n = edges * 2;

    while (--n >= 0) {
      *out++ = &object->vertex[*in++];
    }
  }

  object->faceVertexIndexList =
    MemAlloc((sizeof(short *) + 1) * faces, MEMF_PUBLIC);
  {
    short **indexListPtr = object->faceVertexIndexList;
    short *list = mesh->faceVertex;
    short n;

    while ((n = *list++)) {
      *indexListPtr++ = list;
      list += n;
    }

    *indexListPtr = NULL;
  }

  object->faceEdgeIndexList =
    MemAlloc((sizeof(short *) + 1) * faces, MEMF_PUBLIC);
  {
    short **indexListPtr = object->faceEdgeIndexList;
    short *list = mesh->faceEdge;
    short n;

    while ((n = *list++)) {
      *indexListPtr++ = list;
      list += n;
    }

    *indexListPtr = NULL;
  }

  object->vertexFlags = MemAlloc(vertices, MEMF_PUBLIC);
  object->faceFlags = MemAlloc(faces, MEMF_PUBLIC);
  if (edges)
    object->edgeFlags = MemAlloc(edges, MEMF_PUBLIC);
  object->visibleFace = MemAlloc(sizeof(SortItemT) * faces, MEMF_PUBLIC);

  object->scale.x = fx12f(1.0);
  object->scale.y = fx12f(1.0);
  object->scale.z = fx12f(1.0); 

  return object;
}
