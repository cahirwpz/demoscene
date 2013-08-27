#ifndef __ENGINE_SCENE_OBJECT_H__
#define __ENGINE_SCENE_OBJECT_H__

#include "gfx/pixbuf.h"
#include "engine/mesh.h"
#include "engine/ms3d.h"

typedef struct PolygonExt {
  uint16_t index;
  uint8_t flags;
  uint8_t color;
  float depth;

  Vector3D normal;
} PolygonExtT;

typedef struct SceneObject {
  char *name;
  MeshT *mesh;

  bool wireframe;

  MatrixStack3D *ms;
  Vector3D *vertex;
  PolygonExtT *polygonExt;
  PolygonExtT **sortedPolygonExt;
} SceneObjectT;

SceneObjectT *NewSceneObject(const char *name, MeshT *mesh);

void RenderSceneObject(SceneObjectT *self, PixBufT *canvas);

#endif
