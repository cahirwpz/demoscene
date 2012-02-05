#include "std/stack.h"
#include "gfx/transformations.h"
#include "gfx/matrix2d.h"
#include "gfx/matrix3d.h"

static StackT *Matrices;

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

  if (a && b)
    M2D_Multiply((Matrix2D *)StackPush(Matrices), a, b);
}

void TS_Transpose2D() {
  Matrix2D *a = (Matrix2D *)StackGet(Matrices, 1);

  if (a)
    M2D_Transpose((Matrix2D *)StackPush(Matrices), a);
}

void TS_PushIdentity2D() {
  M2D_LoadIdentity((Matrix2D *)StackPush(Matrices));
}

void TS_PushRotation2D(float angle) {
  M2D_LoadRotation((Matrix2D *)StackPush(Matrices), angle);
}

void TS_PushScaling2D(float sx, float sy) {
  M2D_LoadScaling((Matrix2D *)StackPush(Matrices), sx, sy);
}

void TS_PushTranslation2D(float tx, float ty) {
  M2D_LoadTranslation((Matrix2D *)StackPush(Matrices), tx, ty);
}
