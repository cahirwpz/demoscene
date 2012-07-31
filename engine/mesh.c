#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "engine/mesh.h"

static void DeleteMesh(MeshT *mesh) {
  MemUnref(mesh->vertexToPoly.vertex);
  MemUnref(mesh->vertexToPoly.indices);
  MemUnref(mesh->surfaceNormal);
  MemUnref(mesh->vertexNormal);
  MemUnref(mesh->polygon);
  MemUnref(mesh->vertex);
}

TYPEDECL(MeshT, (FreeFuncT)DeleteMesh);

MeshT *NewMesh(size_t vertices, size_t polygons) {
  MeshT *mesh = NewInstance(MeshT);

  mesh->vertexNum = vertices;
  mesh->polygonNum = polygons;
  mesh->vertex = NewTable(Vector3D, vertices);
  mesh->polygon = NewTable(TriangleT, polygons);

  return mesh;
}

MeshT *NewMeshFromFile(const StrT fileName) {
  uint16_t *data = ReadFileSimple(fileName);

  if (data) {
    uint16_t vertices = data[0];
    uint16_t polygons = data[1];

    MeshT *mesh = NewMesh(vertices, polygons);

    Vector3D *vertexPtr = (Vector3D *)&data[2];
    TriangleT *polygonPtr = (TriangleT *)&vertexPtr[vertices];

    memcpy(mesh->vertex, vertexPtr, sizeof(Vector3D) * vertices);
    memcpy(mesh->polygon, polygonPtr, sizeof(TriangleT) * polygons);

    LOG("Mesh '%s' has %d vertices and %d polygons.",
        fileName, vertices, polygons);

    MemUnref(data);

    return mesh;
  }

  return NULL;
}

/*
 * Calculates the center of mesh and repositions it.  The center of a mesh is a
 * center of its mass using uniform weights for each vertex.
 */
void CenterMeshPosition(MeshT *mesh) {
  Vector3D med = { 0.0f, 0.0f, 0.0f };
  int i;

  for (i = 0; i < mesh->vertexNum; i++)
    V3D_Add(&med, &med, &mesh->vertex[i]);

  V3D_Scale(&med, &med, 1.0f / mesh->vertexNum);

  for (i = 0; i < mesh->vertexNum; i++)
    V3D_Sub(&mesh->vertex[i], &mesh->vertex[i], &med);
}

/*
 * Scales the mesh so that it fits into a sphere of radius 1.0 at origin of
 * (0.0, 0.0, 0.0).
 */
void NormalizeMeshSize(MeshT *mesh) {
  float m = 0.0f;
  size_t i;

  for (i = 0; i < mesh->vertexNum; i++) {
    m = max(abs(mesh->vertex[i].x), m); 
    m = max(abs(mesh->vertex[i].y), m); 
    m = max(abs(mesh->vertex[i].z), m); 
  }

  for (i = 0; i < mesh->vertexNum; i++)
    V3D_Scale(&mesh->vertex[i], &mesh->vertex[i], 1.0f / m);
}

/*
 * For given triangle T with vertices A, B and C, surface normal N is a cross
 * product between vectors AB and BC.
 *
 * Ordering of vertices in polygon description is meaningful - depending on
 * that the normal vector will be directed inwards or outwards.
 *
 * Lightwave convention is used:
 * "The vertex list for each polygon should begin at a convex vertex and
 * proceed clockwise as seen from the visible side of the polygon."
 */
void CalculateSurfaceNormals(MeshT *mesh) {
  size_t i;

  if (mesh->surfaceNormal)
    PANIC("Already added surface normals to mesh %p.", mesh);

  mesh->surfaceNormal = NewTable(Vector3D, mesh->polygonNum);

  for (i = 0; i < mesh->polygonNum; i++) {
    Vector3D *normal = &mesh->surfaceNormal[i];
    Vector3D u, v;

    size_t p1 = mesh->polygon[i].p1;
    size_t p2 = mesh->polygon[i].p2;
    size_t p3 = mesh->polygon[i].p3;

    V3D_Sub(&u, &mesh->vertex[p1], &mesh->vertex[p2]);
    V3D_Sub(&v, &mesh->vertex[p2], &mesh->vertex[p3]);

    V3D_Cross(normal, &u, &v);
    V3D_Normalize(normal, normal, 1.0f);
  }
}

/*
 * Calculates a map from vertex index into a list of polygon the vertex belongs
 * to.  mesh->polygon can be considered as a map from polygon number to polygon
 * vertices, so this procedure calculates a reverse map.
 */
void CalculateVertexToPolygonMap(MeshT *mesh) {
  IndexMapT *map = &mesh->vertexToPoly;
  size_t i, j;

  map->vertex = NewTable(IndexArrayT, mesh->vertexNum);
  map->indices = NewTable(uint16_t, mesh->vertexNum * 3);

  for (i = 0; i < mesh->polygonNum; i++) {
    size_t p1 = mesh->polygon[i].p1;
    size_t p2 = mesh->polygon[i].p2;
    size_t p3 = mesh->polygon[i].p3;

    map->vertex[p1].count++;
    map->vertex[p2].count++;
    map->vertex[p3].count++;
  }

  for (i = 0, j = 0; i < mesh->vertexNum;) {
    map->vertex[i].index = &map->indices[j];

    j += map->vertex[i++].count;
  }

  for (i = 0; i < mesh->vertexNum; i++)
    map->vertex[i].count = 0;

  for (i = 0; i < mesh->polygonNum; i++) {
    size_t p1 = mesh->polygon[i].p1;
    size_t p2 = mesh->polygon[i].p2;
    size_t p3 = mesh->polygon[i].p3;

    map->vertex[p1].index[map->vertex[p1].count++] = i;
    map->vertex[p2].index[map->vertex[p2].count++] = i;
    map->vertex[p3].index[map->vertex[p3].count++] = i;
  }
}

/*
 * Vertex normal vector is defined as averaged normal of all adjacent polygons.
 * Assumption is made that each vertex belong to at least one polygon.
 */
void CalculateVertexNormals(MeshT *mesh) {
  size_t i, j;

  mesh->vertexNormal = NewTable(Vector3D, mesh->vertexNum);

  for (i = 0; i < mesh->vertexNum; i++) {
    Vector3D normal = { 0.0f, 0.0f, 0.0f };

    for (j = 0; j < mesh->vertexToPoly.vertex[i].count; j++) {
      uint16_t p = mesh->vertexToPoly.vertex[i].index[j];

      V3D_Add(&normal, &normal, &mesh->surfaceNormal[p]);
    }

    V3D_Normalize(&mesh->vertexNormal[i], &normal, 1.0f);
  }
}
