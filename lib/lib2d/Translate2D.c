#include <2d.h>

void Translate2D(Matrix2D *M, short x, short y) {
  M->x += x;
  M->y += y;
}
