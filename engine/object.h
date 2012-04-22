#ifndef __ENGINE_SCENE_OBJECT_H__
#define __ENGINE_SCENE_OBJECT_H__

#include "gfx/ms3d.h"
#include "gfx/canvas.h"
#include "engine/mesh.h"

typedef struct SceneObject {
  char *name;
  MeshT *mesh;

  MatrixStack3D *ms;
  PointT *points;
} SceneObjectT;

SceneObjectT *NewSceneObject(const char *name, MeshT *mesh);
void DeleteSceneObject(SceneObjectT *object);

void ApplyTransformation(SceneObjectT *self, int width, int height);
void RenderSceneObject(SceneObjectT *self, CanvasT *canvas);

#define RSC_SCENE_OBJECT(NAME, MESH) \
  AddRscSimple(NAME, NewSceneObject(NAME, MESH), (FreeFuncT)DeleteSceneObject)

#endif
