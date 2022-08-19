/* This code is covered by BSD license. */
#ifndef __STAB_H__
#define __STAB_H__

#include <asm.h>

/*
 * Taken from FreeBSD <sys/sys/nlist_aout.h>.
 *
 * https://www.sourceware.org/gdb/onlinedocs/stabs.html#Non_002dStab-Symbol-Types
 */
#define N_UNDF 0x00 /* undefined */
#define N_ABS 0x02  /* absolute address */
#define N_TEXT 0x04 /* text segment */
#define N_DATA 0x06 /* data segment */
#define N_BSS 0x08  /* bss segment */
#define N_INDR 0x0a /* alias definition */
#define N_SIZE 0x0c /* pseudo type, defines a symbol's size */
#define N_COMM 0x12 /* common reference */
#define N_SETA 0x14 /* Absolute set element symbol */
#define N_SETT 0x16 /* Text set element symbol */
#define N_SETD 0x18 /* Data set element symbol */
#define N_SETB 0x1a /* Bss set element symbol */
#define N_SETV 0x1c /* Pointer to set vector in data area. */
#define N_FN 0x1e   /* file name (N_EXT on) */
#define N_WARN 0x1e /* warning message (N_EXT off) */

#define N_EXT 0x01  /* external (global) bit, OR'ed in */
#define N_TYPE 0x1e /* mask for all the type bits */
#define N_STAB 0xe0 /* mask for debugger symbols -- stab(5) */

/*
 * Taken from FreeBSD <include/stab.h>
 *
 * https://www.sourceware.org/gdb/onlinedocs/stabs.html#Stab-Symbol-Types
 */

#define N_GSYM 0x20   /* global symbol */
#define N_FNAME 0x22  /* F77 function name */
#define N_FUN 0x24    /* procedure name */
#define N_STSYM 0x26  /* data segment variable */
#define N_LCSYM 0x28  /* bss segment variable */
#define N_MAIN 0x2a   /* main function name */
#define N_PC 0x30     /* global Pascal symbol */
#define N_RSYM 0x40   /* register variable */
#define N_SLINE 0x44  /* text segment line number */
#define N_DSLINE 0x46 /* data segment line number */
#define N_BSLINE 0x48 /* bss segment line number */
#define N_SSYM 0x60   /* structure/union element */
#define N_SO 0x64     /* main source file name */
#define N_LSYM 0x80   /* stack variable */
#define N_BINCL 0x82  /* include file beginning */
#define N_SOL 0x84    /* included source file name */
#define N_PSYM 0xa0   /* parameter variable */
#define N_EINCL 0xa2  /* include file end */
#define N_ENTRY 0xa4  /* alternate entry point */
#define N_LBRAC 0xc0  /* left bracket */
#define N_EXCL 0xc2   /* deleted include file */
#define N_RBRAC 0xe0  /* right bracket */
#define N_BCOMM 0xe2  /* begin common */
#define N_ECOMM 0xe4  /* end common */
#define N_ECOML 0xe8  /* end common (local name) */
#define N_LENG 0xfe   /* length of preceding entry */

#define STABS(string, type, other, desc, value)                                \
  asm(".stabs \"" __STRING(string) "\"," __STRING(type) ","                    \
      __STRING(other) "," __STRING(desc) "," __STRING(value))

/* Make references to `alias` refer to `symbol` */
#define ALIAS(symbol, alias)                                                   \
  STABS(_L(alias), 11 /* N_INDR | N_EXT */, 0, 0, 0);                          \
  STABS(_L(symbol), 1 /* N_UNDF | N_EXT */, 0, 0, 0)

#endif /* !__SLAB_H__ */
