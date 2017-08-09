#ifndef __READER_H__
#define __READER_H__

#include "common.h"

__regargs BOOL MatchString(char **data, const char *pattern);
__regargs BOOL NextWord(char **data);
__regargs void SkipWord(char **data);
__regargs BOOL NextLine(char **data);
__regargs void SkipLine(char **data);
__regargs BOOL EndOfLine(char **data);
__regargs BOOL ReadByte(char **data, BYTE *numptr);
__regargs BOOL ReadShort(char **data, WORD *numptr);
__regargs BOOL ReadInt(char **data, LONG *numptr);
__regargs BOOL ReadFloat(char **data, FLOAT *numptr);
__regargs WORD ReadString(char **data, char *buf, WORD buflen);

#endif
