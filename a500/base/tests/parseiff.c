#include "io.h"
#include "iff.h"

int __nocommandline = 1;
u_int __oslibversion = 33;

extern char *__commandline;
extern int __commandlen;

int main() {
  u_short len = __commandlen;
  char *filename = alloca(len);
  IffFileT iff;

  memcpy(filename, __commandline, len--);
  filename[len] = '\0';

  OpenIff(&iff, filename);
  Log("Parsing '%s':\n", filename);
  Log("%.4s %d\n", (char *)&iff.header.type, iff.header.length);

  while (ParseChunk(&iff)) {
    Log(".%.4s %d\n", (char *)&iff.chunk.type, iff.chunk.length);
    SkipChunk(&iff);
  }

  CloseIff(&iff);

  return 0;
}
