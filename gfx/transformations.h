#ifndef __GFX_TRANSFORMATIONS_H__
#define __GFX_TRANSFORMATIONS_H__

bool TS_Init();
void TS_End();

void TS_Reset();
void TS_Compose2D();
void TS_Transpose2D();
void TS_PushIdentity2D();
void TS_PushRotation2D(float angle);
void TS_PushScaling2D(float sx, float sy);
void TS_PushTranslation2D(float tx, float ty);

#endif
