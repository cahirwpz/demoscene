#include "engine/scene.h"
#include "std/memory.h"
#include "std/list.h"

struct Scene {
  ListT *objects;
};

SceneT *NewScene() {
  SceneT *self = MemNew0(sizeof(SceneT));
  self->objects = NewList();
  return self;
}

void DeleteScene(SceneT *self) {
  if (self) {
    DeleteListFull(self->objects, (FreeFuncT)DeleteSceneObject);
    MemFree(self);
  }
}

void SceneAddObject(SceneT *self, SceneObjectT *object) {
  ListPushBack(self->objects, object);
}

static CmpT CompareName(const SceneObjectT *obj, const StrT name) {
  return strcmp(obj->name, name);
}

MatrixStack3D *GetObjectTranslation(SceneT *self, const StrT name) {
  SceneObjectT *object = ListSearch(self->objects, (CompareFuncT)CompareName, name);

  return object ? object->ms : NULL;
}

void RenderScene(SceneT *self, CanvasT *canvas) {
  void RenderObject(SceneObjectT *obj) {
    RenderSceneObject(obj, canvas);
  }

  ListForEach(self->objects, (IterFuncT)RenderObject, NULL);
}
