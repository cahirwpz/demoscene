#include "engine/scene.h"
#include "std/memory.h"
#include "std/list.h"

struct Scene {
  ListT *objects;
};

static void DeleteScene(SceneT *self) {
  MemUnref(self->objects);
}

TYPEDECL(SceneT, (FreeFuncT)DeleteScene);

SceneT *NewScene() {
  SceneT *self = NewInstance(SceneT);
  self->objects = NewList();
  return self;
}

void SceneAddObject(SceneT *self, SceneObjectT *object) {
  ListPushBack(self->objects, object);
}

static CmpT CompareName(const SceneObjectT *obj, const char *name) {
  return strcmp(obj->name, name);
}

MatrixStack3D *GetObjectTranslation(SceneT *self, const char *name) {
  SceneObjectT *object = ListSearch(self->objects, (CompareFuncT)CompareName, (PtrT)name);

  return object ? object->ms : NULL;
}

SceneObjectT *GetObject(SceneT *self, const char *name) {
  return ListSearch(self->objects, (CompareFuncT)CompareName, (PtrT)name);
}

void RenderScene(SceneT *self, PixBufT *canvas) {
  void RenderObject(SceneObjectT *obj) {
    RenderSceneObject(obj, canvas);
  }

  ListForEach(self->objects, (IterFuncT)RenderObject, NULL);
}
