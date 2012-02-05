#ifndef __STD_TYPES_H__
#define __STD_TYPES_H__

#include <stdint.h>
#include <stddef.h>

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

typedef enum { FALSE, TRUE } bool;

#define abs(a) ((a) > 0 ? (a) : -(a))
#define swapi(a,b)	{ (a)^=(b); (b)^=(a); (a)^=(b); }
#define swapf(a,b)	{ float t; t = (b); (b) = (a); (a) = t; }

typedef void* (*AllocFuncT)();
typedef void (*FreeFuncT)(void *);
typedef bool (*InitFuncT)(void *);

#endif
