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

  if (OpenIff(&iff, filename)) {
    Print("Parsing '%s':\n", filename);

    Print("%.4s %ld\n", (STRPTR)&iff.header.type, iff.header.length);

    while (ParseChunk(&iff)) {
      Print(".%.4s %ld\n", (STRPTR)&iff.chunk.type, iff.chunk.length);
      SkipChunk(&iff);
    }

    CloseIff(&iff);
  } else {
    Print("'%s' is not an IFF file.\n", filename);
  }

  return 0;
}
