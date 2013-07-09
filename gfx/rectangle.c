#include "gfx/rectangle.h"

void DrawRectange(PixBufT *canvas,
                  int x, int y, unsigned int w, unsigned int h)
{
  /* clipping */
  if (x < 0) {
    w += x;
    x = 0;
  }
  
  if (y < 0) {
    h += y;
    y = 0;
  }

  if (x + w > canvas->width)
    w = canvas->width - x;

  if (y + h > canvas->height)
    h = canvas->height - y;

  /* drawing */
  if (w > 0 && h > 0) {
    uint32_t dstride = canvas->width - w;
    uint8_t *dst = &canvas->data[y * canvas->width + x];
    uint8_t c = canvas->fgColor;

    do {
      int n = w;

      do {
        *dst++ = c;
      } while (--n);

      dst += dstride;
    } while (--h);
  }
}
