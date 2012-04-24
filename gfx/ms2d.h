#ifndef __GFX_MS2D_H__
#define __GFX_MS2D_H__

#include "std/stack.h"
#include "gfx/matrix2d.h"

typedef StackT MatrixStack2D;

MatrixStack2D *NewMatrixStack2D();

void Reset2D(MatrixStack2D *ts);
void PushIdentity2D(MatrixStack2D *ts);
void PushRotation2D(MatrixStack2D *ts, float angle);
void PushScaling2D(MatrixStack2D *ts, float scaleX, float scaleY);
void PushTranslation2D(MatrixStack2D *ts, float moveX, float moveY);
Matrix2D *GetMatrix2D(MatrixStack2D *ts, size_t num);

#endif
