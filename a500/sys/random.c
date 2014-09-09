#include "random.h"

ULONG random() {
  static ULONG x = 123456789;
  static ULONG y = 362436069;
  static ULONG z = 521288629;
  static ULONG w = 88675123;
  ULONG t;

  t = x ^ (x << 11);
  x = y; y = z; z = w;
  return w = w ^ (w >> 19) ^ (t ^ (t >> 8));
}
