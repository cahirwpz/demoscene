#ifndef __UVMAP_MISC_H__
#define __UVMAP_MISC_H__

#include <math.h>

#include "std/math.h"
#include "uvmap/generate.h"

UVMapGenerate(0,
              0.3f / (r + 0.5f * x),
              3.0f * a / M_PI);

UVMapGenerate(1,
              x * cos(2.0f * r) - y * sin(2.0f * r),
              y * cos(2.0f * r) + x * sin(2.0f * r));

UVMapGenerate(2,
              pow(r, 0.33f),
              a / M_PI + r);

UVMapGenerate(3,
              cos(a) / (3 * r),
              sin(a) / (3 * r));

UVMapGenerate(4,
              0.04f * y + 0.06f * cos(a * 3) / r,
              0.04f * x + 0.06f * sin(a * 3) / r);

UVMapGenerate(5,
              0.1f * y / (0.11f + r * 0.15f),
              0.1f * x / (0.11f + r * 0.15f));

UVMapGenerate(6,
              0.5f * a / M_PI + 0.25f * r,
              pow(r, 0.25f));

UVMapGenerate(7,
              0.5f * a / M_PI,
              sin(5.0f * r));

UVMapGenerate(8,
              3.0f * a / M_PI,
              sin(6.0f * r) + 0.5f * cos(7.0f * a));

UVMapGenerate(9,
              x * log(0.5f * r * r),
              y * log(0.5f * r * r));

UVMapGenerate(10,
              8 * x * (1.5-r) * (1.5-r),
              8 * y * (1.5-r) * (1.5-r));

#endif
