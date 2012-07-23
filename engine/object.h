#ifndef __ENGINE_SCENE_OBJECT_H__
#define __ENGINE_SCENE_OBJECT_H__

#include "gfx/canvas.h"
#include "engine/mesh.h"
#include "engine/ms3d.h"

typedef struct SceneObject {
  StrT name;
  MeshT *mesh;

  MatrixStack3D *ms;
  PointT *points;
} SceneObjectT;

SceneObjectT *NewSceneObject(const StrT name, MeshT *mesh);

void RenderSceneObject(SceneObjectT *self, CanvasT *canvas);

#endif
