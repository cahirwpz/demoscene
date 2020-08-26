#ifndef __ASM_H__
#define __ASM_H__

#ifdef __ELF__
#define _L(name) name
#else
#define _L(name) _##name
#endif

#define ENTRY(name)                                                            \
  .text;                                                                       \
  .even;                                                                       \
  .globl _L(name);                                                             \
  .type _L(name), @function;                                                   \
  _L(name) :

#define END(name) .size _L(name), .- _L(name)

#define STRONG_ALIAS(alias, sym)                                               \
  .globl _L(alias);                                                            \
  _L(alias) = _L(sym)

#endif
