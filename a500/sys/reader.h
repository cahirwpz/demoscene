#ifndef __READER_H__
#define __READER_H__

#include "common.h"

__regargs void SkipSpaces(char **strptr);
__regargs BOOL ReadNumber(char **strptr, WORD *numptr);
/* Returns length of the symbol. */
__regargs WORD ReadSymbol(char **strptr, char **symptr);

__regargs BOOL ExpectSymbol(char **strptr, char *expect);

#endif
