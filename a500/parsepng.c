#include <string.h>

#include "io.h"
#include "memory.h"
#include "iff.h"

#define PNG_ID0 MAKE_ID(0x89, 0x50, 0x4e, 0x47)
#define PNG_ID1 MAKE_ID(0x0d, 0x0a, 0x1a, 0x0a)

#define PNG_IDAT MAKE_ID('I', 'D', 'A', 'T')
#define PNG_IEND MAKE_ID('I', 'E', 'N', 'D')
#define PNG_IHDR MAKE_ID('I', 'H', 'D', 'R')
#define PNG_PLTE MAKE_ID('P', 'L', 'T', 'E')
#define PNG_bKGD MAKE_ID('b', 'K', 'G', 'D')
#define PNG_tEXt MAKE_ID('t', 'E', 'X', 't')
#define PNG_tRNS MAKE_ID('t', 'R', 'N', 'S')

#define PNG_GRAYSCALE       0
#define PNG_TRUECOLOR       2
#define PNG_INDEXED         3
#define PNG_GRAYSCALE_ALPHA 4
#define PNG_TRUECOLOR_ALPHA 6

typedef struct {
  ULONG width;
  ULONG height;
  UBYTE bit_depth;
  UBYTE colour_type;
  UBYTE compression_method; // always 0
  UBYTE filter_method;      // always 0
  UBYTE interlace_method;
} __attribute__((packed)) PngHeaderT;

typedef union {
  struct {
    UWORD v;
  } type0;
  struct {
    UWORD r, g, b;
  } type2;
  struct {
    UBYTE alpha[0];
  } type3;
} PngTransparencyT;

typedef union {
  struct {
    UWORD v;
  } type0;
  struct {
    UWORD r, g, b;
  } type2;
  struct {
    UBYTE v;
  } type3;
} PngBackgroundT;

typedef struct {
  UBYTE r, g, b;
} __attribute__((packed)) RGB;

int __nocommandline = 1;
ULONG __oslibversion = 33;

extern char *__commandline;
extern int __commandlen;

static char *s_type[] = {
  "grayscale",
  NULL,
  "truecolor",
  "indexed",
  "grayscale with alpha",
  NULL,
  "truecolor with alpha"
};

static char *s_interlace[] = {
  "no interlace",
  "Adam7 interlace"
};

int main() {
  UWORD len = __commandlen;
  STRPTR filename = __builtin_alloca(len);
  FileT *file;

  memcpy(filename, __commandline, len--);
  filename[len] = '\0';

  Print("Parsing '%s'.\n", filename);

  if ((file = OpenFile(filename, IOF_BUFFERED))) {
    ULONG chk[3];

    memset(chk, 0, sizeof(chk));

    if (FileRead(file, &chk, 8) &&
        chk[0] == PNG_ID0 && chk[1] == PNG_ID1) 
    {
      PngHeaderT ihdr;

      while (chk[0] != PNG_IEND) {
        if (!FileRead(file, &chk, 8))
          break;

        Print("> %4s [%ld]\n", (STRPTR)&chk[1], chk[0]);

        switch (chk[1]) {
          case PNG_IHDR:
            {
              FileRead(file, &ihdr, sizeof(ihdr));

              Print("size : %ld x %ld, %s, bit depth : %ld, %s\n",
                    ihdr.width, ihdr.height, s_type[ihdr.colour_type], 
                    (LONG)ihdr.bit_depth, s_interlace[ihdr.interlace_method]);
            }
            break;

          case PNG_PLTE:
            {
              RGB *plte = MemAlloc(chk[0], MEMF_PUBLIC|MEMF_CLEAR);
              WORD n = chk[0] / 3;

              FileRead(file, plte, chk[0]);

              Print("%ld : ", (LONG)n);
              {
                RGB *ptr = plte;
                WORD i;
                for (i = 0; i < n; i++, ptr++)
                  Print("#%02lx%02lx%02lx ",
                        (LONG)ptr->r, (LONG)ptr->g, (LONG)ptr->b);
              }
              Print("\n");
              MemFree(plte);
            }
            break;

          case PNG_tRNS:
            {
              PngTransparencyT trns;
              UBYTE type = ihdr.colour_type & 3;

              FileRead(file, &trns, chk[0]);

              if (type == PNG_GRAYSCALE)
                Print("#%ld\n", (LONG)trns.type0.v);
              else if (type == PNG_TRUECOLOR)
                Print("#%02lx%02lx%02lx\n",
                      (LONG)trns.type2.r, (LONG)trns.type2.g, (LONG)trns.type2.b);
              else if (type == PNG_INDEXED) {
                WORD n = chk[0];
                WORD i;

                for (i = 0; i < n; i++)
                  Print("#%02lx ", (LONG)trns.type3.alpha[i]);
                Print("\n");
              }
            }
            break;

          case PNG_bKGD:
            {
              PngBackgroundT bkgd;
              UBYTE type = ihdr.colour_type & 3;

              FileRead(file, &bkgd, chk[0]);

              if (type == PNG_GRAYSCALE)
                Print("#%ld\n", (LONG)bkgd.type0.v);
              else if (type == PNG_TRUECOLOR)
                Print("#%02lx%02lx%02lx\n",
                      (LONG)bkgd.type2.r, (LONG)bkgd.type2.g, (LONG)bkgd.type2.b);
              else if (type == PNG_INDEXED)
                Print("#%ld\n", (LONG)bkgd.type3.v);
            }
            break;

          case PNG_tEXt:
            {
              char *text = MemAlloc(chk[0] + 1, MEMF_PUBLIC|MEMF_CLEAR);

              FileRead(file, text, chk[0]);

              Print("%s %s\n", text, text + strlen(text) + 1);

              MemFree(text);
            }
            break;

          default:
            FileSeek(file, chk[0], SEEK_CUR);
            break;
        }

        FileSeek(file, 4, SEEK_CUR); // skip CRC
      }
    } else {
      Print("'%s' is not a PNG file!\n", filename);
    }

    CloseFile(file);
  }

  return 0;
}
