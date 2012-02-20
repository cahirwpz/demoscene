#include "std/debug.h"
#include "gfx/ms3d.h"

MatrixStack3D *NewMatrixStack3D() {
  return NewStack(NewAtomPool(sizeof(Matrix3D), 20));
}

void DeleteMatrixStack3D(MatrixStack3D *ms) {
  DeleteStack(ms);
}

void Reset3D(MatrixStack3D *ms) {
  StackReset(ms);
}

static void Compose3D(MatrixStack3D *ms) {
  if (StackSize(ms) >= 2) {
    Matrix3D *d = (Matrix3D *)StackPushNew(ms);
    Matrix3D *a = (Matrix3D *)StackPeek(ms, 2);
    Matrix3D *b = (Matrix3D *)StackPeek(ms, 1);

    M3D_Multiply(d, a, b);
  }
}

void PushIdentity3D(MatrixStack3D *ms) {
  M3D_LoadIdentity((Matrix3D *)StackPushNew(ms));
}

void PushRotation3D(MatrixStack3D *ms,
                    float angleX, float angleY, float angleZ)
{
  M3D_LoadRotation((Matrix3D *)StackPushNew(ms), angleX, angleY, angleZ);
  Compose3D(ms);
}

void PushScaling3D(MatrixStack3D *ms,
                   float scaleX, float scaleY, float scaleZ)
{
  M3D_LoadScaling((Matrix3D *)StackPushNew(ms), scaleX, scaleY, scaleZ);
  Compose3D(ms);
}

void PushTranslation3D(MatrixStack3D *ms,
                       float moveX, float moveY, float moveZ)
{
  M3D_LoadTranslation((Matrix3D *)StackPushNew(ms), moveX, moveY, moveZ);
  Compose3D(ms);
}

void PushPerspective3D(MatrixStack3D *ms,
                       float viewerX, float viewerY, float viewerZ)
{
  M3D_LoadPerspective((Matrix3D *)StackPushNew(ms), viewerX, viewerY, viewerZ);
  Compose3D(ms);
}

Matrix3D *GetMatrix3D(MatrixStack3D *ms, size_t num) {
  return (Matrix3D *)StackPeek(ms, num);
}
