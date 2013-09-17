#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "engine/mesh.h"

static void DeleteSurface(SurfaceT *surface) {
  MemUnref(surface->name);
}

TYPEDECL(SurfaceT, (FreeFuncT)DeleteSurface);

static void DeleteMesh(MeshT *mesh) {
  MemUnref(mesh->vertexToPoly.vertex);
  MemUnref(mesh->vertexToPoly.indices);
  MemUnref(mesh->surfaceNormal);
  MemUnref(mesh->vertexNormal);
  MemUnref(mesh->surface);
  MemUnref(mesh->polygon);
  MemUnref(mesh->vertex);
}

TYPEDECL(MeshT, (FreeFuncT)DeleteMesh);

MeshT *NewMesh(size_t vertices, size_t polygons, size_t surfaces) {
  MeshT *mesh = NewInstance(MeshT);

  mesh->vertexNum = vertices;
  mesh->polygonNum = polygons;
  mesh->surfaceNum = surfaces;
  mesh->vertex = NewTable(Vector3D, vertices);
  mesh->polygon = NewTable(TriangleT, polygons);
  mesh->surface = NewTableOfType(SurfaceT, surfaces);

  return mesh;
}

typedef struct DiskMesh {
  uint16_t vertices;
  uint16_t polygons;
  uint16_t surfaces;
  uint8_t data[0];
} DiskMeshT;

typedef struct DiskSurface {
  uint8_t flags;
  RGB color;
  char name[0];
} DiskSurfaceT;

MeshT *NewMeshFromFile(const char *fileName) {
  DiskMeshT *header = ReadFileSimple(fileName);

  if (header) {
    MeshT *mesh = NewMesh(header->vertices, header->polygons, header->surfaces);
    uint8_t *data = header->data;
    int i;

    memcpy(mesh->vertex, data, sizeof(Vector3D) * header->vertices);
    data += sizeof(Vector3D) * header->vertices;

    for (i = 0; i < header->vertices; i++) {
      mesh->vertex[i].y = - mesh->vertex[i].y;
      mesh->vertex[i].z = - mesh->vertex[i].z;
    }

    memcpy(mesh->polygon, data, sizeof(TriangleT) * header->polygons);
    data += sizeof(TriangleT) * header->polygons;

    for (i = 0; i < header->surfaces; i++) {
      DiskSurfaceT *surface = (DiskSurfaceT *)data;

      mesh->surface[i].name = StrDup(surface->name);
      mesh->surface[i].color.rgb = surface->color;
      mesh->surface[i].sideness = surface->flags;

      data += sizeof(DiskSurfaceT) + strlen(surface->name) + 1;
    }

    LOG("Mesh '%s' has %d vertices, %d polygons and %d surfaces.",
        fileName, header->vertices, header->polygons, header->surfaces);

    MemUnref(header);

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
    V3D_NormalizeToUnit(normal, normal);
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

/*
 * Use provided palette to map surface RGB color to the palette. 
 */
void MeshApplyPalette(MeshT *mesh, PaletteT *palette) {
  SurfaceT *surface = mesh->surface;
  int i;

  for (i = 0; i < mesh->surfaceNum; i++)
    surface[i].color.clut = PaletteFindNearest(palette, surface[i].color.rgb);
}
