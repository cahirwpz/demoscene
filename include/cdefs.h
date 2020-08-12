#ifndef __CDEFS_H__
#define __CDEFS_H__

#ifdef __GNUC__
#define __GNUC_PREREQ__(x, y)                                                  \
  ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) || (__GNUC__ > (x)))
#else
#define __GNUC_PREREQ__(x, y) 0
#endif

#define ___STRING(x) __STRING(x)
#define ___CONCAT(x, y) __CONCAT(x, y)

#define __unused __attribute__((unused))
#define __constfunc __attribute__((const))
#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))

#define __data_chip __attribute__((section(".datachip")))
#define __data_bss __attribute__((section(".bsschip")))

#if __GNUC_PREREQ__(3, 0)
#define __noinline __attribute__((noinline))
#define __always_inline __attribute__((__always_inline__))
#else
#define __noinline
#define __always_inline
#endif

#if __GNUC_PREREQ__(2, 92)
#define __restrict __restrict__
#else
#define __restrict restrict
#endif

#define __DECONST(type, var) ((type)(unsigned long)(const void *)(var))

#define offsetof(st, m) ((u_int)((char *)&((st *)0)->m - (char *)0))

#define alloca __builtin_alloca

#define align(x, n) (((x) + (n)-1) & (-(n)))

#define __BIT(x) (1L << (x))

#endif
