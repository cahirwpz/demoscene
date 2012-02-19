#ifndef __GFX_TRANSFORMATIONS_H__
#define __GFX_TRANSFORMATIONS_H__

#include "gfx/matrix2d.h"
#include "gfx/matrix3d.h"

void TS_Init();
void TS_End();

void TS_Reset();
void TS_Compose2D();
void TS_Transpose2D();
void TS_PushIdentity2D();
void TS_PushRotation2D(float angle);
void TS_PushScaling2D(float scaleX, float scaleY);
void TS_PushTranslation2D(float moveX, float moveY);
Matrix2D *TS_GetMatrix2D(size_t num);

void TS_Compose3D();
void TS_PushIdentity3D();
void TS_PushScaling3D(float scaleX, float scaleY, float scaleZ);
void TS_PushRotation3D(float angleX, float angleY, float angleZ);
void TS_PushTranslation3D(float moveX, float moveY, float moveZ);
void TS_PushPerspective(float viewerX, float viewerY, float viewerZ);
Matrix3D *TS_GetMatrix3D(size_t num);

#endif
