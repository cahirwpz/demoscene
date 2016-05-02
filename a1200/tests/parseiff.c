#include "std/debug.h"
#include "std/memory.h"
#include "system/iff.h"

int main(int argc, char **argv) {
  int i;

  for (i = 1; i < argc; i++) {
    IffFileT *iff;

    printf("Parsing '%s':\n", argv[i]);

    if ((iff = IffOpen(argv[i]))) {
      printf("%.4s %d\n", (char *)&iff->header.type, iff->header.length);

      while (IffParseChunk(iff)) {
        printf(".%.4s %d\n", (char *)&iff->chunk.type, iff->chunk.length);
        IffSkipChunk(iff);
      }

      MemUnref(iff);
    } else {
      printf("%s is not an IFF file.\n", argv[i]);
    }
  }

  return 0;
}
