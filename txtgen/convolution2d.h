#ifndef __GFX_CONVOLUTION2D_H__
#define __GFX_CONVOLUTION2D_H__

#include "gfx/pixbuf.h"

typedef struct CvlKernel {
  size_t n;
  float k;
  float matrix[0];
} CvlKernelT;

void Convolution2D(PixBufT *dst, PixBufT *src, CvlKernelT *kernel);

#endif
