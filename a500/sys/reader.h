#ifndef __READER_H__
#define __READER_H__

#include "common.h"

__regargs BOOL NextWord(char **strptr);
__regargs void SkipWord(char **strptr);
__regargs BOOL NextLine(char **strptr);
__regargs void SkipLine(char **strptr);
__regargs BOOL EndOfLine(char **strptr);
__regargs BOOL ReadShort(char **strptr, WORD *numptr);
__regargs BOOL ReadInt(char **strptr, LONG *numptr);
__regargs WORD ReadSymbol(char **strptr, char **symptr); /* Returns length of the symbol. */

#endif
