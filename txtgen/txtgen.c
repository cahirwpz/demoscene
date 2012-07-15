#include "std/debug.h"
#include "gfx/colors.h"
#include "gfx/pixbuf.h"
#include "txtgen/txtgen.h"

int GetFilteredPixel(PixBufT *pixbuf asm("a0"),
                     int x asm("d0"), int y asm("d1"))
{
	int xf = x & 0xffff;
	int yf = y & 0xffff;

  int xi = x >> 16;
  int yi = y >> 16;

  uint8_t *data = &pixbuf->data[pixbuf->width * yi + xi];

  int p1 = data[0];
  int p2 = data[1];
  int p3 = data[pixbuf->width];
  int p4 = data[pixbuf->width + 1];

  int d31 = p1 + ((p3 - p1) * yf >> 16);
  int d42 = p2 + ((p4 - p2) * yf >> 16);

	return d31 + ((d42 - d31) * xf >> 16);
}
