#ifndef __GFX_MATRIX2D_H__
#define __GFX_MATRIX2D_H__

#include "gfx/common.h"

typedef float Matrix2D[3][3];

void M2D_Multiply(Matrix2D *d, Matrix2D *a, Matrix2D *b);
void M2D_Transpose(Matrix2D *d, Matrix2D *a);

void M2D_LoadIdentity(Matrix2D *d);
void M2D_LoadRotation(Matrix2D *d, float angle);
void M2D_LoadScaling(Matrix2D *d, float sx, float sy);
void M2D_LoadTranslation(Matrix2D *d, float tx, float ty);

void M2D_Transform(PointT *dst, PointT *src, int n, Matrix2D *m);

#endif
