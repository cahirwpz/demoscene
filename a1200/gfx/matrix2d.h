#ifndef __GFX_MATRIX2D_H__
#define __GFX_MATRIX2D_H__

#include "gfx/common.h"

typedef float Matrix2D[3][3];

void Multiply2D(Matrix2D *d, Matrix2D *a, Matrix2D *b);
void Transpose2D(Matrix2D *d, Matrix2D *a);

void LoadIdentity2D(Matrix2D *d);
void LoadRotation2D(Matrix2D *d, float angle);
void LoadScaling2D(Matrix2D *d, float scaleX, float scaleY);
void LoadTranslation2D(Matrix2D *d, float moveX, float moveY);

void Transform2D(PointT *dst, PointT *src, int n, Matrix2D *m);

#endif
