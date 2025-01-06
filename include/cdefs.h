#ifndef __CDEFS_H__
#define __CDEFS_H__

#ifdef __GNUC__
#define __GNUC_PREREQ__(x, y)                                                  \
  ((__GNUC__ == (x) && __GNUC_MINOR__ >= (y)) || (__GNUC__ > (x)))
#else
#define __GNUC_PREREQ__(x, y) 0
#endif

#define __unused __attribute__((unused))
#define __constfunc __attribute__((const))
#define __packed __attribute__((packed))
#define __noreturn __attribute__((noreturn))
#undef __aligned
#define __aligned(x) __attribute__((aligned(x)))
#define __section(x) __attribute__((section(".section " ## x)))

/* Annotate variable to be put into text section. This should be used only on
 * scalar variable if you want it to be accessed with PC-relative addressing.
 * This saves 4 cycles / 2 bytes of instruction memory / 1 relocation. */
#define __code __section(".text")
/* Annotate initialized variable to be put into memory section. */
#define __data __section(".data")
/* Annotate initialized variable to be put into memory accessible
 * by custom chipset. */
#define __data_chip __section(".datachip")
/* Annotate uninitialized variable to be put into memory accessible
 * by custom chipset. */
#define __bss_chip __section(".bsschip")

#if __GNUC_PREREQ__(4, 1)
#define __returns_twice __attribute__((returns_twice))
#else
#define __returns_twice
#endif

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

#define offsetof(st, m) ((int)((char *)&((st *)0)->m - (char *)0))

#define alloca __builtin_alloca

#define align(x, n) (((x) + (n)-1) & (-(n)))

#define __BIT(x) (1L << (x))

#define __CONCAT1(x, y) x##y
#define __CONCAT(x, y) __CONCAT1(x, y)
#define __STRING1(x) #x
#define __STRING(x) __STRING1(x)

#endif
