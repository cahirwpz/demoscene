#include "std/stack.h"
#include "gfx/transformations.h"
#include "gfx/matrix2d.h"
#include "gfx/matrix3d.h"
#include "system/debug.h"

static StackT *Matrices = NULL;

bool TS_Init() {
  if (!Matrices)
    Matrices = NewStack((AllocFuncT)NewMatrix3D, (FreeFuncT)DeleteMatrix3D);

  return Matrices ? TRUE : FALSE;
}

void TS_End() {
  DeleteStack(Matrices);
  Matrices = NULL;
}

void TS_Reset() {
  StackReset(Matrices);
}

void TS_Compose2D() {
  Matrix2D *a = (Matrix2D *)StackGet(Matrices, 2);
  Matrix2D *b = (Matrix2D *)StackGet(Matrices, 1);

  if (a && b) {
    M2D_Multiply((Matrix2D *)StackPush(Matrices), a, b);
  } else {
    LOG("TS_Compose2D: At least 2 matrices on stack required!\n");
  }
}

void TS_Compose3D() {
  Matrix3D *a = (Matrix3D *)StackGet(Matrices, 2);
  Matrix3D *b = (Matrix3D *)StackGet(Matrices, 1);

  if (a && b) {
    M3D_Multiply((Matrix3D *)StackPush(Matrices), a, b);
  } else {
    LOG("TS_Compose3D: At least 2 matrices on stack required!\n");
  }
}

void TS_Transpose2D() {
  Matrix2D *a = (Matrix2D *)StackGet(Matrices, 1);

  if (a) {
    M2D_Transpose((Matrix2D *)StackPush(Matrices), a);
  } else {
    LOG("TS_Transpose2D: At least 1 matrix on stack required!\n");
  }
}

void TS_PushIdentity2D() {
  M2D_LoadIdentity((Matrix2D *)StackPush(Matrices));
}

void TS_PushIdentity3D() {
  M3D_LoadIdentity((Matrix3D *)StackPush(Matrices));
}

void TS_PushRotation2D(float angle) {
  M2D_LoadRotation((Matrix2D *)StackPush(Matrices), angle);
}

void TS_PushRotation3D(float angleX, float angleY, float angleZ) {
  M3D_LoadRotation((Matrix3D *)StackPush(Matrices), angleX, angleY, angleZ);
}

void TS_PushScaling2D(float scaleX, float scaleY) {
  M2D_LoadScaling((Matrix2D *)StackPush(Matrices), scaleX, scaleY);
}

void TS_PushScaling3D(float scaleX, float scaleY, float scaleZ) {
  M3D_LoadScaling((Matrix3D *)StackPush(Matrices), scaleX, scaleY, scaleZ);
}

void TS_PushTranslation2D(float moveX, float moveY) {
  M2D_LoadTranslation((Matrix2D *)StackPush(Matrices), moveX, moveY);
}

void TS_PushTranslation3D(float moveX, float moveY, float moveZ) {
  M3D_LoadTranslation((Matrix3D *)StackPush(Matrices), moveX, moveY, moveZ);
}

Matrix2D *TS_GetMatrix2D(size_t num) {
  return (Matrix2D *)StackGet(Matrices, num);
}

Matrix3D *TS_GetMatrix3D(size_t num) {
  return (Matrix3D *)StackGet(Matrices, num);
}

void TS_PushPerspective(float viewerX, float viewerY, float viewerZ) {
  M3D_LoadPerspective((Matrix3D *)StackPush(Matrices),
                      viewerX, viewerY, viewerZ);
}
