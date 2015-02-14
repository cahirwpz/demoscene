#ifndef __ENGINE_SCENE_H__
#define __ENGINE_SCENE_H__

#include "engine/object.h"

typedef struct Scene SceneT;

SceneT *NewScene();
void SceneAddObject(SceneT *self, SceneObjectT *object);
SceneObjectT *GetObject(SceneT *self, const char *name);
MatrixStack3D *GetObjectTranslation(SceneT *self, const char *name);
void RenderScene(SceneT *self, PixBufT *canvas);

#endif
