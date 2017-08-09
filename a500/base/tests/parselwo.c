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

  OpenIff(&iff, filename);
  Log("Parsing '%s':\n", filename);
  Log("%.4s %ld\n", (STRPTR)&iff.header.type, iff.header.length);

  while (ParseChunk(&iff)) {
    Log(".%.4s %ld\n", (STRPTR)&iff.chunk.type, iff.chunk.length);

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
            Log("%5ld : [%ld %ld %ld]\n", (LONG)i, x, y, z);
          }

          MemFree(pnts);
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
              Log("%5ld: [", (LONG)j++);
              while (--vertices > 0)
                Log("%ld ", (LONG)pols[i++]);
              Log("%ld] ", (LONG)pols[i++]);

              /* Polygon flags. */
              Log("{%ld}\n", (LONG)pols[i++]);
            }
          } else {
            Log("..%.4s\n", (STRPTR)pols);

            i += 2; /* Skip POLS type field. */

            while (i < n) {
              /* Face vertex indices. */
              WORD vertices = pols[i++];
              Log("%5ld: [", (LONG)j++);
              while (--vertices > 0)
                Log("%ld ", (LONG)pols[i++]);
              Log("%ld]\n", (LONG)pols[i++]);
            }
          }

          MemFree(pols);
        }
        break;

      default:
        SkipChunk(&iff);
        break;
    }
  }

  CloseIff(&iff);

  return 0;
}
