#ifndef _STDARG_H_
#define _STDARG_H_

typedef void *__gnuc_va_list;

#define __va_rounded_size(TYPE)                                                \
  (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))
#define va_start(AP, LASTARG)                                                  \
  (AP = ((__gnuc_va_list)__builtin_next_arg(LASTARG)))
#define va_arg(AP, TYPE)                                                       \
  (AP = (__gnuc_va_list)((char *)(AP) + __va_rounded_size(TYPE)),              \
   *((TYPE *)(void *)((char *)(AP) - ((sizeof(TYPE) < __va_rounded_size(char)  \
                                           ? sizeof(TYPE)                      \
                                           : __va_rounded_size(TYPE))))))
#define va_end(AP) ((void)0)
#define __va_copy(DEST, SRC) (DEST) = (SRC)

typedef __gnuc_va_list va_list;

#endif
