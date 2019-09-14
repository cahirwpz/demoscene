#ifndef __CDEFS_H__
#define __CDEFS_H__

#ifdef __GNUC__
#define __GNUC_PREREQ__(x, y)                                                  \
  ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) || (__GNUC__ > (x)))
#else
#define __GNUC_PREREQ__(x, y) 0
#endif

#define __unused __attribute__((__unused__))
#if __GNUC_PREREQ__(2, 92)
#define __restrict __restrict__
#else
#define __restrict restrict
#endif

#define __DECONST(type, var) ((type)(unsigned long)(const void *)(var))

#define __noreturn __attribute__((__noreturn__))

#define offsetof(st, m) ((u_int)((char *)&((st *)0)->m - (char *)0))

#define alloca __builtin_alloca

#define align(x, n) (((x) + (n)-1) & (-(n)))

#define __BIT(x) (1L << (x))

#endif
