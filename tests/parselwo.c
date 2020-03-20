#include "ffp.h"
#include "memory.h"
#include "io.h"
#include "iff.h"

int __nocommandline = 1;
u_int __oslibversion = 33;

extern char *__commandline;
extern int __commandlen;

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_SRFS MAKE_ID('S', 'R', 'F', 'S')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')

static int scale = 1000;

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

    switch (iff.chunk.type) {
      case ID_PNTS:
        {
          float *pnts = MemAlloc(iff.chunk.length, MEMF_PUBLIC);
          float s = SPFlt(scale);
          float *p = pnts;
          short n = iff.chunk.length / 12;
          short i;

          ReadChunk(&iff, pnts);

          for (i = 0; i < n; i++) {
            int x = SPFix(SPMul(SPFieee(*p++), s));
            int y = SPFix(SPMul(SPFieee(*p++), s));
            int z = SPFix(SPMul(SPFieee(*p++), s));
            Log("%5d : [%d %d %d]\n", i, x, y, z);
          }

          MemFree(pnts);
        }
        break;

      case ID_POLS:
        {
          short *pols = MemAlloc(iff.chunk.length, MEMF_PUBLIC);
          short n = iff.chunk.length / 2;
          short i = 0, j = 0;

          ReadChunk(&iff, pols);

          if (iff.header.type == ID_LWOB) {
            while (i < n) {
              /* Face vertex indices. */
              short vertices = pols[i++];
              Log("%5d: [", j++);
              while (--vertices > 0)
                Log("%d ", pols[i++]);
              Log("%d] ", pols[i++]);

              /* Polygon flags. */
              Log("{%d}\n", pols[i++]);
            }
          } else {
            Log("..%.4s\n", (char *)pols);

            i += 2; /* Skip POLS type field. */

            while (i < n) {
              /* Face vertex indices. */
              short vertices = pols[i++];
              Log("%5d: [", j++);
              while (--vertices > 0)
                Log("%d ", pols[i++]);
              Log("%d]\n", pols[i++]);
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
