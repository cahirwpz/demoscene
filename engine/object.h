#ifndef __ENGINE_SCENE_OBJECT_H__
#define __ENGINE_SCENE_OBJECT_H__

#include "gfx/pixbuf.h"
#include "engine/mesh.h"
#include "engine/ms3d.h"
#include "engine/triangle.h"

typedef enum {
  RENDER_WIREFRAME,
  RENDER_WIREFRAME_AA,
  RENDER_FILLED,
  RENDER_FLAT_SHADING,
  RENDER_GOURAUD_SHADING
} RenderModeT;

extern RenderModeT RenderMode;
extern bool RenderAllFaces;

typedef struct PolygonExt {
  uint16_t index;
  uint8_t flags;
  float depth;
  Vector3D normal;
} PolygonExtT;

typedef struct VertexExt {
  uint8_t flags;
  float x, y;
  Vector3D normal;
} VertexExtT;

typedef struct SceneObject {
  char *name;
  MeshT *mesh;

  MatrixStack3D *ms;
  Vector3D *vertex;
  VertexExtT *vertexExt;
  PolygonExtT *polygonExt;
  PolygonExtT **sortedPolygonExt;
  EdgeScanT *edgeScan;
  Vector3D *surfaceNormal;
} SceneObjectT;

SceneObjectT *NewSceneObject(const char *name, MeshT *mesh);

void RenderSceneObject(SceneObjectT *self, PixBufT *canvas);

#endif
