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

void CenterObjectPosition(ObjectT *object) {
  VertexT med = { 0.0f, 0.0f, 0.0f };
  int i;

  for (i = 0; i < object->vertex_count; i++) {
    med.x += object->vertex[i].x;
    med.y += object->vertex[i].y;
    med.z += object->vertex[i].z;
  }

  med.x /= object->vertex_count;
  med.y /= object->vertex_count;
  med.z /= object->vertex_count;

  for (i = 0; i < object->vertex_count; i++) {
    object->vertex[i].x -= med.x;
    object->vertex[i].y -= med.y; 
    object->vertex[i].z -= med.z; 
  }
}

void NormalizeObject(ObjectT *object) {
  float m = 0.0f;
  int i;

  for (i = 0; i < object->vertex_count; i++) {
    m = max(abs(object->vertex[i].x), m); 
    m = max(abs(object->vertex[i].y), m); 
    m = max(abs(object->vertex[i].z), m); 
  }

  for (i = 0; i < object->vertex_count; i++) {
    object->vertex[i].x /= m;
    object->vertex[i].y /= m; 
    object->vertex[i].z /= m; 
  }
}
