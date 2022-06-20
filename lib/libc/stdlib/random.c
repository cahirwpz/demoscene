#include <stdlib.h>
#include <common.h>

/*
 * xoroshiro64++ algorithm, based on:
 * https://github.com/ZiCog/xoroshiro/blob/master/src/main/c/xoroshiro.h
 */
u_int random(void) {
  static u_int s[2] = { 1, 0 };
  const u_int a = 26;
  const u_int b = 9;
  const u_int c = 13;
  const u_int d = 17;
  u_int s0 = s[0], s1 = s[1];

  s1 ^= s0;
  s0 = rol(s0, a) ^ s1 ^ (s1 << b);
  s1 = rol(s1, c);

  s[0] = s0, s[1] = s1;

  return rol(s0 + s1, d) + s0;
}
