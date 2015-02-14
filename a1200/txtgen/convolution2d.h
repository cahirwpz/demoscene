#ifndef __GFX_CONVOLUTION2D_H__
#define __GFX_CONVOLUTION2D_H__

#include "gfx/pixbuf.h"

typedef struct CvltKernel {
  size_t n;
  int k;
  int matrix[0];
} CvltKernelT;

void Convolution2D(PixBufT *dst, PixBufT *src, CvltKernelT *kernel);

#endif
