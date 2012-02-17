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
    object->vertex = NEW_A(Vector3D, vertices);
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
    Vector3D *vertexPtr = (Vector3D *)&data[2];
    TriangleT *trianglePtr = (TriangleT *)&vertexPtr[vertices];

    memcpy(object->vertex, vertexPtr, sizeof(Vector3D) * vertices);
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
  Vector3D med = { 0.0f, 0.0f, 0.0f };
  int i;

  for (i = 0; i < object->vertex_count; i++)
    V3D_Add(&med, &med, &object->vertex[i]);

  V3D_Scale(&med, &med, 1.0f / object->vertex_count);

  for (i = 0; i < object->vertex_count; i++)
    V3D_Sub(&object->vertex[i], &object->vertex[i], &med)
}

void NormalizeObject(ObjectT *object) {
  float m = 0.0f;
  int i;

  for (i = 0; i < object->vertex_count; i++) {
    m = max(abs(object->vertex[i].x), m); 
    m = max(abs(object->vertex[i].y), m); 
    m = max(abs(object->vertex[i].z), m); 
  }

  for (i = 0; i < object->vertex_count; i++)
    V3D_Scale(&object->vertex[i], &object->vertex[i], 1.0f / m);
}
