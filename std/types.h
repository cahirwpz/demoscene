#ifndef __STD_TYPES_H__
#define __STD_TYPES_H__

#include <stddef.h>
#include <stdbool.h>
#include <sys/types.h>

#define BOOL(a) ((a) != 0 ? true : false)

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint16_t
typedef unsigned short uint16_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifndef uint64_t
typedef	unsigned long long uint64_t;
#endif

#ifndef AMIGA
#define asm(a)
#endif

#define abs(a) ((a) > 0 ? (a) : -(a))
#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) < (b) ? (a) : (b))
#define sign(a) ((a) >= 0 ? 1 : -1)
#define swapi(a,b)  { (a)^=(b); (b)^=(a); (a)^=(b); }
#define swapr(a,b)  { asm("exg %0,%1" : "+r" (a), "+r" (b)); }
#define swapf(a,b)  { float t; t = (b); (b) = (a); (a) = t; }

#ifndef MAKE_ID
#define MAKE_ID(a, b, c, d) \
  (((a) << 24) | ((b) << 16) | ((c) << 8) | (d))
#endif

#define bswap32(a) ((((a) >> 24) & 0x000000ff) | (((a) << 24) & 0xff000000) | \
                    (((a) >> 8) & 0x0000ff00) | (((a) << 8) & 0x00ff0000)) 
#define bswap16(a) ((((a) >> 8) & 0x00ff) | (((a) << 8) & 0xff00))

/* Auxiliary types */
typedef enum { CMP_LT = -1, CMP_EQ = 0, CMP_GT = 1 } CmpT;
typedef void* PtrT;

/**
 * @brief Type of resource release function.
 *
 * @param rsc Resource to be released.
 */
typedef void (*FreeFuncT)(PtrT rsc);

/**
 * @brief Type of resource copy function.
 *
 * @param dst Destination for the copy.
 * @param src Structure to be copied.
 * @result original value of dst
 */
typedef PtrT (*CopyFuncT)(PtrT dst, PtrT src);

typedef struct Type {
  size_t size;
  const char *name;
  FreeFuncT free;
  CopyFuncT copy;
} TypeT;

#define TYPEDECL(type, ...) \
  const TypeT Type##type = {sizeof(type),#type,__VA_ARGS__}

#define TYPEDEF(type) \
  extern const TypeT Type##type;

/**
 * @brief Type of a predicate checking function.
 *
 * @param item Item for which the predicate is to be checked.
 * @result Value of the predicate.
 */
typedef bool (*PredicateT)(PtrT item);

/**
 * @brief Type of an iterator function.
 *
 * @param item  Element of iterable data structure.
 * @param data	Auxiliary data that can be used during iteration.
 */
typedef void (*IterFuncT)(PtrT item, PtrT data);

/**
 * @brief Type of a comparison function.
 *
 * @param lhs Value representing left-hand side of comparison.
 * @param rhs Value representing right-hand side of comparison.
 * @result CmpT value that depends on comparison result.
 */
typedef CmpT (*CompareFuncT)(const PtrT lhs, const PtrT rhs);

/**
 * @brief Type of an array's item setting function
 *
 * @param array Array of arbitrary elements.
 * @param index Number of an element in aforementioned array.
 * @param value A pointer to value to be written into the element.
 */
typedef void (*SetItemFuncT)(PtrT array, size_t index, PtrT value);

/*
 * Macros for handling symbol table information (aka linker set elements).
 */

/* Add symbol 's' to list 'l' (type 't': 22=text, 24=data, 26=bss). */
#define ADD2LIST(s, l, t) \
  asm(".stabs \"_" #l "\"," #t ",0,0,_" #s )

/* Install private constructors and destructors pri MUST be in [-127, 127] */
#define ADD2INIT(ctor, pri) \
  ADD2LIST(ctor, __INIT_LIST__, 22); \
  asm(".stabs \"___INIT_LIST__\",20,0,0," #pri "+128")
#define ADD2EXIT(dtor, pri) \
  ADD2LIST(dtor, __EXIT_LIST__, 22); \
  asm(".stabs \"___EXIT_LIST__\",20,0,0," #pri "+128")

#endif
