#undef __CONSTLIBBASEDECL__
#include <proto/exec.h>
#include <proto/dos.h>

#include "iff.h"
#include "print.h"

int __nocommandline = 1;
int __initlibraries = 0;

extern char *__commandline;
extern int __commandlen;

struct DosLibrary *DOSBase;

int main() {
  UWORD len = __commandlen;
  STRPTR filename = __builtin_alloca(len);

  CopyMem(__commandline, filename, len--);
  filename[len] = '\0';

  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 34))) {
    IffFileT iff;

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

    CloseLibrary((struct Library *)DOSBase);
  }

  return 0;
}
