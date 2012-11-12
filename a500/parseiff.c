#include <proto/exec.h>
#include <inline/dos.h>

#include "iff.h"
#include "print.h"

int __nocommandline = 1;
int __initlibraries = 0;

struct DosLibrary *DOSBase;

static const char *filename = "test.ilbm";

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 34))) {
    IffFileT iff;

    if (OpenIff(&iff, filename)) {
      Print("%4s %ld\n", (STRPTR)&iff.header.type, iff.header.length);

      while (ParseChunk(&iff)) {
        Print(".%4s %ld\n", (STRPTR)&iff.chunk.type, iff.chunk.length);
        SkipChunk(&iff);
      }

      CloseIff(&iff);
    } else {
      Print("%s is not an IFF file.\n", filename);
    }

    CloseLibrary((struct Library *)DOSBase);
  }

  return 0;
}
