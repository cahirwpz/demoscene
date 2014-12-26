#ifndef __READER_H__
#define __READER_H__

#include "common.h"

__regargs char *SkipSpaces(char *str);
__regargs char *NextLine(char *str);
__regargs BOOL ReadNumber(char **strptr, WORD *numptr);
__regargs WORD ReadSymbol(char **strptr, char **symptr); /* Returns length of the symbol. */

#endif
