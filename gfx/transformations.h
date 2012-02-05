#ifndef __GFX_TRANSFORMATIONS_H__
#define __GFX_TRANSFORMATIONS_H__

#include "gfx/matrix2d.h"

bool TS_Init();
void TS_End();

void TS_Reset();
void TS_Compose2D();
void TS_Transpose2D();
void TS_PushIdentity2D();
void TS_PushRotation2D(float angle);
void TS_PushScaling2D(float scaleX, float scaleY);
void TS_PushTranslation2D(float moveX, float moveY);
Matrix2D *TS_GetMatrix2D(size_t num);

#endif
