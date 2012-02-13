#ifndef __ENGINE_OBJECT_H__
#define __ENGINE_OBJECT_H__

#include "std/types.h"
#include "gfx/common.h"

typedef struct Triangle {
  uint16_t p1, p2, p3;
} TriangleT;

typedef struct Object {
  size_t vertex_count;
  size_t triangle_count;

  VertexT *vertex;
  TriangleT *triangle;
} ObjectT;

ObjectT *NewObject(size_t vertices, size_t triangles);
ObjectT *NewObjectFromFile(const char *fileName, uint32_t memFlags);
void DeleteObject(ObjectT *object);
void NormalizeObject(ObjectT *object);
void CenterObjectPosition(ObjectT *object);

#endif
