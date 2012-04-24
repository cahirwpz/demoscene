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

typedef enum { FALSE, TRUE } bool;

#define BOOL(a) ((a) != 0 ? TRUE : FALSE)

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned long uint32_t;
#endif

#define abs(a) ((a) > 0 ? (a) : -(a))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define swapi(a,b)	{ (a)^=(b); (b)^=(a); (a)^=(b); }
#define swapf(a,b)	{ float t; t = (b); (b) = (a); (a) = t; }

/* Auxiliary types */
typedef enum { CMP_LT = -1, CMP_EQ = 0, CMP_GT = 1 } CmpT;
typedef void* PtrT;
typedef char* StrT;

/* Function types definition */
typedef PtrT (*AllocFuncT)();
typedef void (*FreeFuncT)(PtrT);
typedef bool (*InitFuncT)(PtrT);

/**
 * @brief Type of iterator function.
 *
 * @param item  Element of iterable data structure.
 * @param data	Auxiliary data that can be used during iteration.
 */
typedef void (*IterFuncT)(PtrT item, PtrT data);

/**
 * @brief Type of comparison function.
 *
 * @param lhs Value representing left-hand side of comparison.
 * @param rhs Value representing right-hand side of comparison.
 * @result CmpT value that depends on comparison result.
 */
typedef CmpT (*CompareFuncT)(const PtrT lhs, const PtrT rhs);

#endif
