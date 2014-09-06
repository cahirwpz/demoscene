#undef __CONSTLIBBASEDECL__ 
#include <proto/exec.h>
#include <proto/dos.h>

#include "ffp.h"
#include "iff.h"
#include "print.h"

int __nocommandline = 1;
int __initlibraries = 0;

extern char *__commandline;
extern int __commandlen;

struct DosLibrary *DOSBase;
struct Library *MathBase;

#define ID_LWOB MAKE_ID('L', 'W', 'O', 'B')
#define ID_LWO2 MAKE_ID('L', 'W', 'O', '2')
#define ID_SRFS MAKE_ID('S', 'R', 'F', 'S')
#define ID_PNTS MAKE_ID('P', 'N', 'T', 'S')
#define ID_POLS MAKE_ID('P', 'O', 'L', 'S')

static LONG scale = 500;

int main() {
  UWORD len = __commandlen;
  STRPTR filename = __builtin_alloca(len);

  CopyMem(__commandline, filename, len--);
  filename[len] = '\0';

  DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 33);
  MathBase = OpenLibrary("mathffp.library", 33);

  if (DOSBase && MathBase) {
    IffFileT iff;

    if (OpenIff(&iff, filename)) {
      Print("Parsing '%s':\n", filename);

      Print("%.4s %ld\n", (STRPTR)&iff.header.type, iff.header.length);

      while (ParseChunk(&iff)) {
        Print(".%.4s %ld\n", (STRPTR)&iff.chunk.type, iff.chunk.length);

        switch (iff.chunk.type) {
          case ID_PNTS:
            {
              FLOAT *pnts = AllocMem(iff.chunk.length, MEMF_PUBLIC);
              FLOAT s = SPFlt(scale);
              WORD n = iff.chunk.length / 12;
              WORD i;

              ReadChunk(&iff, pnts);

              for (i = 0; i < n; i++) {
                LONG x = SPFix(SPMul(SPFieee(pnts[i * 3 + 0]), s));
                LONG y = SPFix(SPMul(SPFieee(pnts[i * 3 + 1]), s));
                LONG z = SPFix(SPMul(SPFieee(pnts[i * 3 + 2]), s));
                Print("%5ld : [%ld %ld %ld]\n", (LONG)i, x, y, z);
              }

              FreeMem(pnts, iff.chunk.length);
            }
            break;

          case ID_POLS:
            {
              WORD *pols = AllocMem(iff.chunk.length, MEMF_PUBLIC);
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

              FreeMem(pols, iff.chunk.length);
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
  }

  if (MathBase)
    CloseLibrary(MathBase);
  if (DOSBase)
    CloseLibrary((struct Library *)DOSBase);

  return 0;
}
