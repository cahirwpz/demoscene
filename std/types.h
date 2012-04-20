#ifndef __STD_TYPES_H__
#define __STD_TYPES_H__

#include <stddef.h>
#include <sys/types.h>

#ifdef FALSE
#undef FALSE
#endif

#ifdef TRUE
#undef TRUE
#endif

#define BOOL(a) ((a) ? TRUE : FALSE)

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned long uint32_t;
#endif

typedef enum { FALSE, TRUE } bool;

#define abs(a) ((a) > 0 ? (a) : -(a))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define swapi(a,b)	{ (a)^=(b); (b)^=(a); (a)^=(b); }
#define swapf(a,b)	{ float t; t = (b); (b) = (a); (a) = t; }

typedef void* PtrT;

typedef PtrT (*AllocFuncT)();
typedef void (*FreeFuncT)(PtrT);
typedef bool (*InitFuncT)(PtrT);

/**
 * @brief Type of iterator function (can be used with lists, trees, etc.)
 *
 * @param item  Element of iterable data structure.
 * @param data	Auxiliary data that can be used during iteration.
 */
typedef void (*IterFuncT)(PtrT item, PtrT data);

/**
 * @brief Type of search function (can be used with lists, trees, etc.)
 *
 * @param item  Element of iterable data structure.
 * @param data	Auxiliary data that can be used during iteration.
 * @result TRUE if the element matches (i.e. was found), FALSE otherwise.
 */
typedef bool (*SearchFuncT)(PtrT item, PtrT data);

#endif
