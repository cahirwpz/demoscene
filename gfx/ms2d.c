#include "std/debug.h"
#include "gfx/ms2d.h"

MatrixStack2D *NewMatrixStack2D() {
  return NewStack(NewAtomPool(sizeof(Matrix2D), 20));
}

void DeleteMatrixStack2D(MatrixStack2D *ms) {
  DeleteStack(ms);
}

void Reset2D(MatrixStack2D *ms) {
  StackReset(ms);
}

static void Compose2D(MatrixStack2D *ms) {
  if (StackSize(ms) >= 2) {
    Matrix2D *d = (Matrix2D *)StackPushNew(ms);
    Matrix2D *a = (Matrix2D *)StackPeek(ms, 2);
    Matrix2D *b = (Matrix2D *)StackPeek(ms, 1);

    M2D_Multiply(d, a, b);
  }
}

void PushIdentity2D(MatrixStack2D *ms) {
  M2D_LoadIdentity((Matrix2D *)StackPushNew(ms));
}

void PushRotation2D(MatrixStack2D *ms, float angle) {
  M2D_LoadRotation((Matrix2D *)StackPushNew(ms), angle);
  Compose2D(ms);
}

void PushScaling2D(MatrixStack2D *ms, float scaleX, float scaleY) {
  M2D_LoadScaling((Matrix2D *)StackPushNew(ms), scaleX, scaleY);
  Compose2D(ms);
}

void PushTranslation2D(MatrixStack2D *ms, float moveX, float moveY) {
  M2D_LoadTranslation((Matrix2D *)StackPushNew(ms), moveX, moveY);
  Compose2D(ms);
}

Matrix2D *GetMatrix2D(MatrixStack2D *ms, size_t num) {
  return (Matrix2D *)StackPeek(ms, num);
}
