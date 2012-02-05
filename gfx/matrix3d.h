#ifndef __GFX_MATRIX3D_H__
#define __GFX_MATRIX3D_H__

#include "gfx/common.h"

typedef float Matrix3D[4][4];

Matrix3D *NewMatrix3D();
void DeleteMatrix3D(Matrix3D *matrix);

#endif
