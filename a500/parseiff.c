#include "io.h"
#include "iff.h"

int __nocommandline = 1;
ULONG __oslibversion = 33;

extern char *__commandline;
extern int __commandlen;

int main() {
  UWORD len = __commandlen;
  STRPTR filename = __builtin_alloca(len);
  IffFileT iff;

  memcpy(filename, __commandline, len--);
  filename[len] = '\0';

  OpenIff(&iff, filename);
  Log("Parsing '%s':\n", filename);
  Log("%.4s %ld\n", (STRPTR)&iff.header.type, iff.header.length);

  while (ParseChunk(&iff)) {
    Log(".%.4s %ld\n", (STRPTR)&iff.chunk.type, iff.chunk.length);
    SkipChunk(&iff);
  }

  CloseIff(&iff);

  return 0;
}
