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

  object->point = (Point3D *)mesh->vertex;
  object->faceNormal = (Point3D *)mesh->faceNormal;

  object->vertex = MemAlloc(sizeof(Point3D) * vertices, MEMF_PUBLIC);
  object->edge = MemAlloc(sizeof(EdgeT) * edges, MEMF_PUBLIC);
  {
    short *in = mesh->edge;
    EdgeT *out = object->edge;
    short n = edges;

    while (--n >= 0) {
      out->flags = 0;
      out->point[0] = &object->vertex[*in++];
      out->point[1] = &object->vertex[*in++];
      out++;
    }
  }

  object->faceVertexIndexList =
    MemAlloc((sizeof(short *) + 1) * faces, MEMF_PUBLIC);
  {
    short **indexListPtr = object->faceVertexIndexList;
    short *list = mesh->faceVertex + 1;
    short n;

    while ((n = *list++)) {
      *indexListPtr++ = list;
      list += n + 1;
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

  object->visibleFace = MemAlloc(sizeof(SortItemT) * faces, MEMF_PUBLIC);

  object->scale.x = fx12f(1.0);
  object->scale.y = fx12f(1.0);
  object->scale.z = fx12f(1.0); 

  return object;
}
