#include <pixmap.h>

void PixmapScramble_4_2(const PixmapT *pixmap) {
  if (pixmap->type == PM_CMAP4) {
    u_int *data = pixmap->pixels;
    short n = pixmap->width * pixmap->height / 8;
    register u_int m0 asm("d6") = 0xc3c3c3c3;
    register u_int m1 asm("d7") = 0x0c0c0c0c;

    /* [a0 a1 a2 a3 b0 b1 b2 b3] => [a0 a1 b0 b1 a2 a3 b2 b3] */
    while (--n >= 0) {
      u_int c = *data;
      *data++ = (c & m0) | ((c >> 2) & m1) | ((c & m1) << 2);
    }
  }
}
