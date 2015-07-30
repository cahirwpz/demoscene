#include "ffp.h"
#include "memory.h"
#include "io.h"
#include "iff.h"

int __nocommandline = 1;
ULONG __oslibversion = 33;

extern char *__commandline;
extern int __commandlen;

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_SRFS MAKE_ID('S', 'R', 'F', 'S')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')

static LONG scale = 1000;

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

      switch (iff.chunk.type) {
        case ID_PNTS:
          {
            FLOAT *pnts = MemAlloc(iff.chunk.length, MEMF_PUBLIC);
            FLOAT s = SPFlt(scale);
            FLOAT *p = pnts;
            WORD n = iff.chunk.length / 12;
            WORD i;

            ReadChunk(&iff, pnts);

            for (i = 0; i < n; i++) {
              LONG x = SPFix(SPMul(SPFieee(*p++), s));
              LONG y = SPFix(SPMul(SPFieee(*p++), s));
              LONG z = SPFix(SPMul(SPFieee(*p++), s));
              Print("%5ld : [%ld %ld %ld]\n", (LONG)i, x, y, z);
            }

            MemFree(pnts, iff.chunk.length);
          }
          break;

        case ID_POLS:
          {
            WORD *pols = MemAlloc(iff.chunk.length, MEMF_PUBLIC);
            WORD n = iff.chunk.length / 2;
            WORD i = 0, j = 0;

            ReadChunk(&iff, pols);

            if (iff.header.type == ID_LWOB) {
              while (i < n) {
                /* Face vertex indices. */
                WORD vertices = pols[i++];
                Print("%5ld: [", (LONG)j++);
                while (--vertices > 0)
                  Print("%ld ", (LONG)pols[i++]);
                Print("%ld] ", (LONG)pols[i++]);

                /* Polygon flags. */
                Print("{%ld}\n", (LONG)pols[i++]);
              }
            } else {
              Print("..%.4s\n", (STRPTR)pols);

              i += 2; /* Skip POLS type field. */

              while (i < n) {
                /* Face vertex indices. */
                WORD vertices = pols[i++];
                Print("%5ld: [", (LONG)j++);
                while (--vertices > 0)
                  Print("%ld ", (LONG)pols[i++]);
                Print("%ld]\n", (LONG)pols[i++]);
              }
            }

            MemFree(pols, iff.chunk.length);
          }
          break;

        default:
          SkipChunk(&iff);
          break;
      }
    }

    CloseIff(&iff);
  } else {
    Print("'%s' is not an IFF file.\n", filename);
  }

  return 0;
}
