#include <string.h>

#include "system/memory.h"
#include "system/fileio.h"
#include "system/debug.h"
#include "engine/object.h"

ObjectT *NewObject(size_t vertices, size_t triangles) {
  ObjectT *object = NEW_S(ObjectT);

  if (object) {
    object->vertex_count = vertices;
    object->triangle_count = triangles;
    object->vertex = NEW_A(VertexT, vertices);
    object->triangle = NEW_A(TriangleT, triangles);

    if (object->vertex && object->triangle)
      return object;

    DeleteObject(object);
  }

  return NULL;
}

ObjectT *NewObjectFromFile(const char *fileName, uint32_t memFlags) {
  uint16_t *data = ReadFileSimple(fileName, memFlags);

  if (!data)
    return NULL;

  uint16_t vertices = data[0];
  uint16_t triangles = data[1];

  ObjectT *object = NewObject(vertices, triangles);

  if (object) {
    VertexT *vertexPtr = (VertexT *)&data[2];
    TriangleT *trianglePtr = (TriangleT *)&vertexPtr[vertices];

    memcpy(object->vertex, vertexPtr, sizeof(VertexT) * vertices);
    memcpy(object->triangle, trianglePtr, sizeof(TriangleT) * triangles);
  }

  LOG("Object '%s' has %ld vertices and %ld triangles.\n",
      fileName, (ULONG)vertices, (ULONG)triangles);

  DELETE(data);

  return object;
}

void DeleteObject(ObjectT *object) {
  if (object) {
    DELETE(object->triangle);
    DELETE(object->vertex);
  }

  DELETE(object);
}
