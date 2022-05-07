#ifndef __SYSCALL_H__
#define __SYSCALL_H__

#define SCARG0NR(name)                                                         \
  static inline void name(void) {                                              \
    register int _d0 asm("d0");                                                \
    register int _d1 asm("d1");                                                \
    register int _a0 asm("a0");                                                \
    register int _a1 asm("a1");                                                \
    asm volatile("jsr __" #name ":W"                                           \
                 : "=r"(_d0), "=r"(_d1), "=r"(_a0), "=r"(_a1)                  \
                 :                                                             \
                 : "cc", "memory");                                            \
  }

#define SCARG1NR(name, t1, v1, r1)                                             \
  static inline void name(t1 v1) {                                             \
    register int _d0 asm("d0");                                                \
    register int _d1 asm("d1");                                                \
    register int _a0 asm("a0");                                                \
    register int _a1 asm("a1");                                                \
    register t1 _##name##_r1 asm(#r1) = v1;                                    \
    asm volatile("jsr __" #name ":W"                                           \
                 : "=r"(_d0), "=r"(_d1), "=r"(_a0), "=r"(_a1)                  \
                 : "r"(_##name##_r1)                                           \
                 : "cc", "memory");                                            \
  }

#define SCARG2(name, rt, t1, v1, r1, t2, v2, r2)                               \
  static inline rt name(t1 v1, t2 v2) {                                        \
    register rt *_rv asm("d0");                                                \
    register int _d1 asm("d1");                                                \
    register int _a0 asm("a0");                                                \
    register int _a1 asm("a1");                                                \
    register t1 _##name##_r1 asm(#r1) = v1;                                    \
    register t2 _##name##_r2 asm(#r2) = v2;                                    \
    asm volatile("jsr __" #name ":W"                                           \
                 : "=r"(_rv), "=r"(_d1), "=r"(_a0), "=r"(_a1)                  \
                 : "r"(_##name##_r1), "r"(_##name##_r2)                        \
                 : "cc", "memory");                                            \
    return _rv;                                                                \
  }

#endif /* !__SYSCALL_H__ */
