#ifndef __GFX_MATRIX3D_H__
#define __GFX_MATRIX3D_H__

#include "gfx/common.h"

typedef float Matrix3D[4][4];

Matrix3D *NewMatrix3D();
void DeleteMatrix3D(Matrix3D *matrix);

void M3D_Multiply(Matrix3D *d, Matrix3D *a, Matrix3D *b);
void M3D_Transpose(Matrix3D *d, Matrix3D *a);
void M3D_LoadIdentity(Matrix3D *d);
void M3D_LoadRotation(Matrix3D *d, float angleX, float angleY, float angleZ);
void M3D_LoadScaling(Matrix3D *d, float scaleX, float scaleY, float scaleZ);
void M3D_LoadTranslation(Matrix3D *d, float moveX, float moveY, float moveZ);

void M3D_Transform(VertexT *dst, VertexT *src, int n, Matrix3D *m);
void M3D_Project2D(PointT *dst, VertexT *src, int n, Matrix3D *m);

#endif
