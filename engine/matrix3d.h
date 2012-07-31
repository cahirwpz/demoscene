#ifndef __ENGINE_MATRIX3D_H__
#define __ENGINE_MATRIX3D_H__

#include "engine/vector3d.h"

typedef float Matrix3D[4][4];

Matrix3D *NewMatrix3D();

void Multiply3D(Matrix3D *d, Matrix3D *a, Matrix3D *b);
void InverseMultiply3D(Matrix3D *d, Matrix3D *a, Matrix3D *b);
void Transpose3D(Matrix3D *d, Matrix3D *a);
void LoadIdentity3D(Matrix3D *d);
void LoadRotation3D(Matrix3D *d, float angleX, float angleY, float angleZ);
void LoadScaling3D(Matrix3D *d, float scaleX, float scaleY, float scaleZ);
void LoadTranslation3D(Matrix3D *d, float moveX, float moveY, float moveZ);
void LoadPerspective3D(Matrix3D *d,
                       float viewerX, float viewerY, float viewerZ);
void Transform3D(Vector3D *dst, Vector3D *src, int n, Matrix3D *m);
void ProjectTo2D(int centerX, int centerY,
                 Vector3D *dst, Vector3D *src, int n, Matrix3D *m);
void LoadCameraFromVector(Matrix3D *camera,
                          Vector3D *direction, Vector3D *position);
void LoadCameraFromAngles(Matrix3D *camera,
                          float azimuth, float elevation, Vector3D *position);

#endif
