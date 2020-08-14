#ifndef __LINKERSET_H__
#define __LINKERSET_H__

/*
 * Macros for handling symbol table information (aka linker set elements).
 *
 * https://sourceware.org/gdb/onlinedocs/stabs/Non_002dStab-Symbol-Types.html
 */

/* Add symbol 's' to list 'l' (type 't': 22=text, 24=data, 26=bss). */
#define ADD2LIST(s, l, t) \
  asm(".stabs \"_" #l "\"," #t ",0,0,_" #s )

/*
 * Install private constructors and destructors pri MUST be in [-127, 127]
 * Constructors are called in ascending order of priority,
 * while destructors in descending.
 */
#define ADD2INIT(ctor, pri) \
  ADD2LIST(ctor, __INIT_LIST__, 22); \
  asm(".stabs \"___INIT_LIST__\",20,0,0," #pri "+128")

#define ADD2EXIT(dtor, pri) \
  ADD2LIST(dtor, __EXIT_LIST__, 22); \
  asm(".stabs \"___EXIT_LIST__\",20,0,0,128-" #pri)

/* Make symbol alias from a to b. */
#define ALIAS(a,b) \
  asm(".stabs \"_" #a "\",11,0,0,0;.stabs \"_" #b "\",1,0,0,0")

#endif
