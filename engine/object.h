#ifndef __ENGINE_SCENE_OBJECT_H__
#define __ENGINE_SCENE_OBJECT_H__

#include "gfx/ms3d.h"
#include "gfx/canvas.h"
#include "engine/mesh.h"

typedef struct SceneObject {
  StrT name;
  MeshT *mesh;

  MatrixStack3D *ms;
  PointT *points;
} SceneObjectT;

SceneObjectT *NewSceneObject(const StrT name, MeshT *mesh);
void DeleteSceneObject(SceneObjectT *object);

void RenderSceneObject(SceneObjectT *self, CanvasT *canvas);

#endif
