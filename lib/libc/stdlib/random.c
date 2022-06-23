#include <stdlib.h>
#include <common.h>

#define roll(a, b) (((a) >> (32 - (b))) | ((a) << (b)))
#define rorl(a, b) (((a) << (32 - (b))) | ((a) >> (b)))

static inline u_int rol(u_int a, u_int b) {
  if (b <= 8)
    return roll(a, b);
  if (b < 16 && b > 8)
    return rorl(swap16(a), 16 - b);
  if (b < 24 && b >= 16)
    return roll(swap16(a), b - 16);
  return rorl(a, 32 - b);
}

static inline u_int lsl9(u_int a) {
  asm volatile("lsl.l #8,%0\n\t"
               "add.l %0,%0\n\t"
               : "=d" (a));
  return a;
}

/*
 * xoroshiro64++ algorithm based on https://arxiv.org/pdf/1805.01407.pdf
 *
 * Please note that the paper states "as for the + scrambler, we do not suggest
 * to use the ++ scrambler with xoroshiro64", but that's exactly what is done
 * below. They recommend using ** scrambler, but it would use 32-bit
 * multiplication and be too slow. Since we care more about efficiency that
 * randomness let's leave it as is.
 */

static u_int state[2] = { 1, 0 };

u_int random(void) {
  u_int *s = state;
  const u_int a = 26;
  /* const u_int b = 9; */
  const u_int c = 13;
  const u_int d = 17;
  u_int s0 = s[0], s1 = s[1];

  s1 ^= s0;
  /* s0 = rol(s0, a) ^ s1 ^ (s1 << b); */
  s0 = rol(s0, a) ^ s1 ^ lsl9(s1);
  s1 = rol(s1, c);

  s[0] = s0, s[1] = s1;

  return rol(s0 + s1, d) + s0;
}
