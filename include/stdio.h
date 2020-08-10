#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>
#include <types.h>

typedef __regargs void (kvprintf_fn_t)(int, void *);

int kvprintf(char const *fmt, kvprintf_fn_t *func, void *arg, va_list ap);
int snprintf(char *buf, size_t size, const char *cfmt, ...)
  __attribute__ ((format (printf, 3, 4)));

#endif
