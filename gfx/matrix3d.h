#ifndef __GFX_MATRIX3D_H__
#define __GFX_MATRIX3D_H__

#include "gfx/common.h"
#include "gfx/vector3d.h"

typedef float Matrix3D[4][4];

Matrix3D *NewMatrix3D();
void DeleteMatrix3D(Matrix3D *matrix);

void Multiply3D(Matrix3D *d, Matrix3D *a, Matrix3D *b);
void InvMultiply3D(Matrix3D *d, Matrix3D *a, Matrix3D *b);
void Transpose3D(Matrix3D *d, Matrix3D *a);
void LoadIdentity3D(Matrix3D *d);
void LoadRotation3D(Matrix3D *d, float angleX, float angleY, float angleZ);
void LoadScaling3D(Matrix3D *d, float scaleX, float scaleY, float scaleZ);
void LoadTranslation3D(Matrix3D *d, float moveX, float moveY, float moveZ);
void LoadPerspective3D(Matrix3D *d,
                       float viewerX, float viewerY, float viewerZ);
void Transform3D(Vector3D *dst, Vector3D *src, int n, Matrix3D *m);
void ProjectTo2D(int centerX, int centerY,
                 PointT *dst, Vector3D *src, int n, Matrix3D *m);
void LoadCameraFromVector(Matrix3D *camera,
                          Vector3D *direction, Vector3D *position);
void LoadCameraFromAngles(Matrix3D *camera,
                          float azimuth, float elevation, Vector3D *position);

#endif
