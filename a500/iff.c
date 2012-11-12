#include <exec/types.h>
#include <exec/execbase.h>
#include <inline/exec.h>
#include <inline/graphics.h>
#include <inline/dos.h> 

#include "iff.h"
#include "print.h"

int __nocommandline = 1;
int __initlibraries = 0;

extern struct ExecBase *SysBase;
struct DosLibrary *DOSBase = NULL;

void ParseIff(const char *filename) {
  BPTR fh = Open(filename, MODE_OLDFILE);
  IffHeaderT header;

  if ((Read(fh, &header, sizeof(header)) == sizeof(header)) &&
      (header.magic == ID_FORM))
  {
    BitmapHeaderT bmhd;
    IffChunkT chunk;

    Print("%4s %ld\n", (STRPTR)&header.type, header.length);

    while (Read(fh, &chunk, sizeof(chunk)) == sizeof(chunk)) {
      int skipPayload = 0;

      Print(".%4s %ld\n", (STRPTR)&chunk.type, chunk.length);

      if (chunk.type == ID_BMHD) {
        if (Read(fh, &bmhd, sizeof(bmhd)) != sizeof(bmhd))
          break;

        Print(" size: %ldx%ld\n", (LONG)bmhd.w, (LONG)bmhd.h);
        Print(" bpls: %ld\n", (LONG)bmhd.nPlanes);
        Print(" mask: %ld\n", (LONG)bmhd.masking);
        Print(" comp: %s\n", bmhd.compression ? "yes" : "no");
      } else if (chunk.type == ID_CMAP) {
        ColorRegisterT color;
        int i;

        for (i = 0; i < chunk.length; i += sizeof(color)) {
          if (Read(fh, &color, sizeof(color)) != sizeof(color))
            break;

          Print(" (%3ld, %3ld, %3ld)\n",
                (LONG)color.red, (LONG)color.green, (LONG)color.blue);
        }
      } else if (chunk.type == ID_BODY) {
        /* WARNING: __mulsi3 (LONG * LONG -> LONG) requires
         * utility.library which is not available on Kickstart 1.3 */
        Print(" %ld bpls, each of size %ld, %ld bytes of raw data\n",
              (LONG)bmhd.nPlanes, (LONG)(bmhd.w * bmhd.h / 8),
              (LONG)(bmhd.h * (UWORD)(bmhd.w * bmhd.nPlanes) / 8));

        Print(" %ld bytes (%s data)\n", (LONG)chunk.length,
              bmhd.compression ? "compressed" : "raw");

        skipPayload = 1;
      } else {
        skipPayload = 1;
      }

      if (skipPayload)
        Seek(fh, chunk.length, OFFSET_CURRENT);

      /* Skip a byte if the lenght of a chunk is odd. */
      if (chunk.length & 1)
        Seek(fh, 1, OFFSET_CURRENT);
    }
  } else {
    Print("%s is not an IFF file.\n", filename);
  }

  Close(fh);
}

int main() {
  if ((DOSBase = (struct DosLibrary *)OpenLibrary("dos.library", 34))) {
    ParseIff("test.ilbm");
    CloseLibrary((struct Library *)DOSBase);
  }
  return 0;
}
