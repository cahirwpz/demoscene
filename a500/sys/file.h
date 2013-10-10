#ifndef __FILE_H__
#define __FILE_H__

#include <exec/types.h>

typedef struct {
  ULONG size;
  UBYTE data[0];
} FileDataT;

FileDataT *ReadFile(STRPTR path, ULONG memoryFlags);
void FreeFile(FileDataT *data);

#endif
