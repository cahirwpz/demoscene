#ifndef __GFX_HSL_H__
#define __GFX_HSL_H__

#include "gfx/colors.h"

void RGB2HSL(ColorT *src, ColorVectorT *dst);
void HSL2RGB(ColorVectorT *src, ColorT *dst);

#endif
