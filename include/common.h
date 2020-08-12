#ifndef __COMMON_H__
#define __COMMON_H__

#include <types.h>

#define abs(x)                                                                 \
  ({                                                                           \
    typeof(x) _x = (x);                                                        \
    (_x < 0) ? -_x : _x;                                                       \
  })

#define min(a, b)                                                              \
  ({                                                                           \
    typeof(a) _a = (a);                                                        \
    typeof(b) _b = (b);                                                        \
    _a < _b ? _a : _b;                                                         \
  })

#define max(a, b)                                                              \
  ({                                                                           \
    typeof(a) _a = (a);                                                        \
    typeof(b) _b = (b);                                                        \
    _a > _b ? _a : _b;                                                         \
  })

#define swap(a, b)                                                             \
  ({                                                                           \
    typeof(a) _a = (a);                                                        \
    typeof(a) _b = (b);                                                        \
    (a) = _b;                                                                  \
    (b) = _a;                                                                  \
  })

#define roundup(x, y) ((((x) + ((y) - 1)) / (y)) * (y))
#define rounddown(x, y) (((x) / (y)) * (y))

#define VA_NARGS_IMPL(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define VA_NARGS(...) VA_NARGS_IMPL(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

#define ITER(_VAR, _BEGIN, _END, _EXPR) { \
  short _VAR; \
  for (_VAR = _BEGIN; _VAR <= _END; _VAR++) { \
    _EXPR; \
  } \
}

/* assumes that abs(idx) < 32768 */
static inline short getword(void *tab, short idx) {
  short res;
  asm("addw  %1,%1\n"
      "movew (%2,%1:w),%0\n"
      : "=r" (res)
      : "d" (idx), "a" (tab)
      : "1");
  return res;
}

/* assumes that abs(idx) < 16384 */
static inline int getlong(void *tab, short idx) {
  int res;
  asm("addw  %1,%1\n"
      "addw  %1,%1\n"
      "movel (%2,%1:w),%0\n"
      : "=r" (res)
      : "d" (idx), "a" (tab)
      : "1");
  return res;
}

static inline short absw(short a) {
  if (a < 0)
    return -a;
  return a;
}

static inline u_int swap16(u_int a) {
  asm("swap %0": "+d" (a));
  return a;
}

static inline u_short swap8(u_short a) {
  return (a << 8) | (a >> 8);
}

static inline short div16(int a, short b) {
  asm("divs %1,%0"
      : "+d" (a)
      : "dm" (b));
  return a;
}

static inline short mod16(int a, short b) {
  asm("divs %1,%0\n"
      "swap %0"
      : "+d" (a)
      : "dm" (b));
  return a;
}

static inline void bclr(u_char *ptr, char bit) {
  asm("bclr %1,%0" :: "m" (*ptr), "dI" (bit));
}

static inline void bset(u_char *ptr, char bit) {
  asm("bset %1,%0" :: "m" (*ptr), "dI" (bit));
}

static inline void bchg(u_char *ptr, char bit) {
  asm("bchg %1,%0" :: "m" (*ptr), "dI" (bit));
}

#define rorw(a, b) \
  (((a) << (16 - (b))) | ((a) >> (b)))

#define swapr(a, b) \
  asm ("exg %0,%1" : "+r" (a), "+r" (b))

#define pushl(a) \
  asm ("movel %0,%-" :: "r" (a))

#define popl(a) \
  asm ("movel %+,%0" : "=r" (a))

static inline void *GetSP(void) {
  void *sp;
  asm("movel sp,%0" : "=r" (sp));
  return sp;
}

/*
 * Macros for handling symbol table information (aka linker set elements).
 *
 * https://sourceware.org/gdb/onlinedocs/stabs/Non_002dStab-Symbol-Types.html
 */

/* Add symbol 's' to list 'l' (type 't': 22=text, 24=data, 26=bss). */
#define ADD2LIST(s, l, t) \
  asm(".stabs \"_" #l "\"," #t ",0,0,_" #s )

/*
 * Install private constructors and destructors pri MUST be in [-127, 127]
 * Constructors are called in ascending order of priority,
 * while destructors in descending.
 */
#define ADD2INIT(ctor, pri) \
  ADD2LIST(ctor, __INIT_LIST__, 22); \
  asm(".stabs \"___INIT_LIST__\",20,0,0," #pri "+128")

#define ADD2EXIT(dtor, pri) \
  ADD2LIST(dtor, __EXIT_LIST__, 22); \
  asm(".stabs \"___EXIT_LIST__\",20,0,0,128-" #pri)

/* Make symbol alias from a to b. */
#define ALIAS(a,b) \
  asm(".stabs \"_" #a "\",11,0,0,0;.stabs \"_" #b "\",1,0,0,0")

#define PROFILE_BEGIN(NAME)                                             \
{                                                                       \
  static int average_ ## NAME = 0;                                      \
  static short count_ ## NAME = 0;                                      \
  int lines_ ## NAME = ReadLineCounter();

#define PROFILE_END(NAME)                                               \
  average_ ## NAME += ReadLineCounter() - lines_ ## NAME;               \
  count_ ## NAME ++;                                                    \
  Log(#NAME ": %d\n", div16(average_ ## NAME, count_ ## NAME));         \
}                                                                       \

#endif
