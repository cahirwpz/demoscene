#include "std/math.h"
#include "std/debug.h"
#include "gfx/colors.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

int GetFilteredPixel(PixBufT *pixbuf asm("d0"),
                     float x asm("fp0"), float y asm("fp1"))
{
	float xf = modff(x, &x);
	float yf = modff(y, &x);

  uint8_t *data = &pixbuf->data[pixbuf->width * lroundf(y) + lroundf(x)];

  float p1 = data[0];
  float p2 = data[1];
  float p3 = data[pixbuf->width];
  float p4 = data[pixbuf->width + 1];

  float d31 = (p3 - p1) * yf + p1;
  float d42 = (p4 - p2) * yf + p2;

	return (d42 - d31) * xf + d31;
}
