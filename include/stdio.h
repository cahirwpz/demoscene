#ifndef __STDIO_H__
#define __STDIO_H__

#include <stdarg.h>
#include <types.h>

typedef void (kvprintf_fn_t)(void *, char);

int kvprintf(kvprintf_fn_t *func, void *arg, char const *fmt, va_list ap);
int snprintf(char *buf, size_t size, const char *cfmt, ...)
  __attribute__ ((format (printf, 3, 4)));

#endif
