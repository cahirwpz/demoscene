#include "random.h"

static ULONG seed[] = { 123456789, 362436069, 521288629, 88675123 };

ULONG random() {
  register ULONG *data asm("a0") = seed;
  ULONG t = data[0] ^ (data[0] << 11);
  data[0] = data[1];
  data[1] = data[2];
  data[2] = data[3];
  return data[3] = data[3] ^ (data[3] >> 19) ^ (t ^ (t >> 8));
}
