#ifndef __GFX_MS3D_H__
#define __GFX_MS3D_H__

#include "std/stack.h"
#include "gfx/matrix3d.h"

typedef StackT MatrixStack3D;

MatrixStack3D *NewMatrixStack3D();

void Reset3D(MatrixStack3D *ts);
void PushIdentity3D(MatrixStack3D *ts);
void PushPerspective3D(MatrixStack3D *ts,
                       float viewerX, float viewerY, float viewerZ);
void PushRotation3D(MatrixStack3D *ts,
                    float angleX, float angleY, float angleZ);
void PushScaling3D(MatrixStack3D *ts,
                   float scaleX, float scaleY, float scaleZ);
void PushTranslation3D(MatrixStack3D *ts,
                       float moveX, float moveY, float moveZ);
Matrix3D *GetMatrix3D(MatrixStack3D *ts, size_t num);

#endif
