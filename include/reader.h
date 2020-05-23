#ifndef __READER_H__
#define __READER_H__

#include "types.h"

__regargs bool MatchString(char **data, const char *pattern);
__regargs bool NextWord(char **data);
__regargs void SkipWord(char **data);
__regargs bool NextLine(char **data);
__regargs void SkipLine(char **data);
__regargs bool EndOfLine(char **data);
__regargs bool ReadByte(char **data, char *numptr);
__regargs bool ReadShort(char **data, short *numptr);
__regargs bool ReadInt(char **data, int *numptr);
__regargs short ReadString(char **data, char *buf, short buflen);

#define ReadByteU(data, numptr) ReadByte(data, (char *)numptr)
#define ReadShortU(data, numptr) ReadShort(data, (short *)numptr)

#endif
