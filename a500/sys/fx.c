#include "fx.h"

/* Source: http://www.cc.utah.edu/~nahaj/factoring/isqrt.c.html */ 

__regargs LONG isqrt(LONG x) {
  LONG root = 0;

  if (x > 0) {
    LONG squaredbit = 0x40000000;
    LONG remainder = x;

    while (squaredbit > 0) {
      if (remainder >= (squaredbit | root)) {
        remainder -= (squaredbit | root);
        root >>= 1;
        root |= squaredbit;
      } else {
        root >>= 1;
      }
      squaredbit >>= 2; 
    }
  }

  return root;
}
