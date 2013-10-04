#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "std/table.h"
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

typedef struct DiskTriangle {
  uint16_t surface;
  uint16_t p[3];
} DiskTriangleT;

typedef struct DiskSurface {
  uint8_t flags;
  RGB color;
  char name[0];
} DiskSurfaceT;

static void MeshCalculateEdges(MeshT *mesh);

MeshT *NewMeshFromFile(const char *fileName) {
  DiskMeshT *header = ReadFileSimple(fileName);

  if (header) {
    MeshT *mesh = NewMesh(header->vertices, header->polygons, header->surfaces);
    uint8_t *data = header->data;
    int i;

    /* vertices */
    MemCopy(mesh->vertex, data, sizeof(Vector3D) * header->vertices);

    for (i = 0; i < header->vertices; i++) {
      mesh->vertex[i].y = - mesh->vertex[i].y;
      mesh->vertex[i].z = - mesh->vertex[i].z;
    }

    data += sizeof(Vector3D) * header->vertices;

    /* triangles */
    for (i = 0; i < header->polygons; i++) {
      DiskTriangleT *triangle = (DiskTriangleT *)data;

      mesh->polygon[i].surface = triangle->surface;
      mesh->polygon[i].p[0] = triangle->p[0];
      mesh->polygon[i].p[1] = triangle->p[1];
      mesh->polygon[i].p[2] = triangle->p[2];

      data += sizeof(DiskTriangleT);
    }

    MeshCalculateEdges(mesh);

    /* surfaces */
    for (i = 0; i < header->surfaces; i++) {
      DiskSurfaceT *surface = (DiskSurfaceT *)data;

      mesh->surface[i].name = StrDup(surface->name);
      mesh->surface[i].color.rgb = surface->color;
      mesh->surface[i].sideness = surface->flags;

      data += sizeof(DiskSurfaceT) + strlen(surface->name) + 1;
    }

    LOG("Mesh '%s' has %ld vertices, %ld polygons, %ld edges "
        "and %ld surfaces.", fileName, mesh->vertexNum, mesh->polygonNum,
        mesh->edgeNum, mesh->surfaceNum);

    MemUnref(header);

    return mesh;
  }

  return NULL;
}

/*
 *
 */
typedef struct {
  uint16_t p[2];
  uint16_t polygon, vertex;
} TriangleEdgeT;

__regargs static bool EdgeCmp(const PtrT a, const PtrT b) {
  const EdgeT *e1 = (const EdgeT *)a;
  const EdgeT *e2 = (const EdgeT *)b;

  if (e1->p[0] == e2->p[0])
    return (e1->p[1] < e2->p[1]);
  else
    return (e1->p[0] < e2->p[0]);
}

static void MeshCalculateEdges(MeshT *mesh) {
  TriangleEdgeT *edges = NewTable(TriangleEdgeT, mesh->polygonNum * 3);
  int i, j;

  {
    TriangleEdgeT *edge = edges;
    TriangleT *polygon = mesh->polygon;

    for (i = 0; i < mesh->polygonNum; i++, polygon++) {
      int16_t p0 = polygon->p[0];
      int16_t p1 = polygon->p[1];
      int16_t p2 = polygon->p[2];

      if (p0 < p1) {
        edge->p[0] = p0;
        edge->p[1] = p1;
      } else {
        edge->p[0] = p1;
        edge->p[1] = p0;
      }

      edge->polygon = i;
      edge->vertex = 0;
      edge++;

      if (p1 < p2) {
        edge->p[0] = p1;
        edge->p[1] = p2;
      } else {
        edge->p[0] = p2;
        edge->p[1] = p1;
      }

      edge->polygon = i;
      edge->vertex = 1;
      edge++;

      if (p0 < p2) {
        edge->p[0] = p0;
        edge->p[1] = p2;
      } else {
        edge->p[0] = p2;
        edge->p[1] = p0;
      }

      edge->polygon = i;
      edge->vertex = 2;
      edge++;
    }
  }

  /* Sort all edges */
  TableSort(edges, EdgeCmp, 0, mesh->polygonNum * 3 - 1);

  /* Count unique edges */
  for (i = 1, mesh->edgeNum = 1; i < mesh->polygonNum * 3; i++)
    if (edges[i].p[0] != edges[i - 1].p[0] || edges[i].p[1] != edges[i - 1].p[1])
      mesh->edgeNum++;

  {
    TriangleEdgeT *edge = edges;

    mesh->edge = NewTable(EdgeT, mesh->edgeNum);
    mesh->edge[0].p[0] = edge->p[0];
    mesh->edge[0].p[1] = edge->p[1];
    mesh->polygon[edge->polygon].e[edge->vertex] = 0;
    edge++;

    for (i = 1, j = 0; i < mesh->polygonNum * 3; i++, edge++) {
      if (edge[0].p[0] != edge[-1].p[0] || edge[0].p[1] != edge[-1].p[1]) {
        j++;
        mesh->edge[j].p[0] = edge->p[0];
        mesh->edge[j].p[1] = edge->p[1];
      }

      mesh->polygon[edge->polygon].e[edge->vertex] = j;
    }
  }

  MemUnref(edges);
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

    size_t p1 = mesh->polygon[i].p[0];
    size_t p2 = mesh->polygon[i].p[1];
    size_t p3 = mesh->polygon[i].p[2];

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
    size_t p1 = mesh->polygon[i].p[0];
    size_t p2 = mesh->polygon[i].p[1];
    size_t p3 = mesh->polygon[i].p[2];

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
    size_t p1 = mesh->polygon[i].p[0];
    size_t p2 = mesh->polygon[i].p[1];
    size_t p3 = mesh->polygon[i].p[2];

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
