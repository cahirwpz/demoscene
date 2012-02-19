#include "std/debug.h"
#include "std/stack.h"
#include "gfx/transformations.h"
#include "gfx/matrix2d.h"
#include "gfx/matrix3d.h"

static StackT *Matrices = NULL;

void TS_Init() {
  if (!Matrices)
    Matrices = NewStack(NewAtomPool(sizeof(Matrix3D), 20));
}

void TS_End() {
  DeleteStack(Matrices);
  Matrices = NULL;
}

void TS_Reset() {
  StackReset(Matrices);
}

void TS_Compose2D() {
  if (StackSize(Matrices) >= 2) {
    Matrix2D *d = (Matrix2D *)StackPushNew(Matrices);
    Matrix2D *a = (Matrix2D *)StackPeek(Matrices, 2);
    Matrix2D *b = (Matrix2D *)StackPeek(Matrices, 1);

    M2D_Multiply(d, a, b);
  }
}

void TS_Compose3D() {
  if (StackSize(Matrices) >= 2) {
    Matrix3D *d = (Matrix3D *)StackPushNew(Matrices);
    Matrix3D *a = (Matrix3D *)StackPeek(Matrices, 2);
    Matrix3D *b = (Matrix3D *)StackPeek(Matrices, 1);

    M3D_Multiply(d, a, b);
  }
}

void TS_Transpose2D() {
  if (StackSize(Matrices) >= 1) {
    Matrix2D *d = (Matrix2D *)StackPushNew(Matrices);
    Matrix2D *a = (Matrix2D *)StackPeek(Matrices, 1);

    M2D_Transpose(d, a);
  }
}

void TS_PushIdentity2D() {
  M2D_LoadIdentity((Matrix2D *)StackPushNew(Matrices));
}

void TS_PushIdentity3D() {
  M3D_LoadIdentity((Matrix3D *)StackPushNew(Matrices));
}

void TS_PushRotation2D(float angle) {
  M2D_LoadRotation((Matrix2D *)StackPushNew(Matrices), angle);
}

void TS_PushRotation3D(float angleX, float angleY, float angleZ) {
  M3D_LoadRotation((Matrix3D *)StackPushNew(Matrices), angleX, angleY, angleZ);
}

void TS_PushScaling2D(float scaleX, float scaleY) {
  M2D_LoadScaling((Matrix2D *)StackPushNew(Matrices), scaleX, scaleY);
}

void TS_PushScaling3D(float scaleX, float scaleY, float scaleZ) {
  M3D_LoadScaling((Matrix3D *)StackPushNew(Matrices), scaleX, scaleY, scaleZ);
}

void TS_PushTranslation2D(float moveX, float moveY) {
  M2D_LoadTranslation((Matrix2D *)StackPushNew(Matrices), moveX, moveY);
}

void TS_PushTranslation3D(float moveX, float moveY, float moveZ) {
  M3D_LoadTranslation((Matrix3D *)StackPushNew(Matrices), moveX, moveY, moveZ);
}

Matrix2D *TS_GetMatrix2D(size_t num) {
  return (Matrix2D *)StackPeek(Matrices, num);
}

Matrix3D *TS_GetMatrix3D(size_t num) {
  return (Matrix3D *)StackPeek(Matrices, num);
}

void TS_PushPerspective(float viewerX, float viewerY, float viewerZ) {
  M3D_LoadPerspective((Matrix3D *)StackPushNew(Matrices),
                      viewerX, viewerY, viewerZ);
}
