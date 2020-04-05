#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdarg.h>
#include "cdefs.h"

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;

typedef char int8_t;
typedef unsigned char uint8_t;
typedef short int16_t;
typedef unsigned short uint16_t;
typedef int int32_t;
typedef unsigned int uint32_t;
typedef long long int int64_t;
typedef unsigned long long int uint64_t;

typedef unsigned long int size_t;
typedef long int ssize_t;
typedef long int ptrdiff_t;

typedef unsigned long int uintptr_t;
typedef long int intptr_t;

typedef	long long int	intmax_t;
typedef	unsigned long long int uintmax_t;

typedef enum __packed {
  false = 0,
  true = 1
} bool;

#ifndef NULL
#define NULL (0L)
#endif

#endif
