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
#define PNG_trueCOLOR       2
#define PNG_INDEXED         3
#define PNG_GRAYSCALE_ALPHA 4
#define PNG_trueCOLOR_ALPHA 6

typedef struct {
  u_int width;
  u_int height;
  u_char bit_depth;
  u_char colour_type;
  u_char compression_method; // always 0
  u_char filter_method;      // always 0
  u_char interlace_method;
} __attribute__((packed)) PngHeaderT;

typedef union {
  struct {
    u_short v;
  } type0;
  struct {
    u_short r, g, b;
  } type2;
  struct {
    u_char alpha[0];
  } type3;
} PngTransparencyT;

typedef union {
  struct {
    u_short v;
  } type0;
  struct {
    u_short r, g, b;
  } type2;
  struct {
    u_char v;
  } type3;
} PngBackgroundT;

typedef struct {
  u_char r, g, b;
} __attribute__((packed)) RGB;

int __nocommandline = 1;
u_int __oslibversion = 33;

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
  u_short len = __commandlen;
  char *filename = alloca(len);
  FileT *file;

  memcpy(filename, __commandline, len--);
  filename[len] = '\0';

  Log("Parsing '%s'.\n", filename);

  if ((file = OpenFile(filename, IOF_BUFFERED))) {
    u_int chk[3];

    memset(chk, 0, sizeof(chk));

    if (FileRead(file, &chk, 8) &&
        chk[0] == PNG_ID0 && chk[1] == PNG_ID1) 
    {
      PngHeaderT ihdr;

      while (chk[0] != PNG_IEND) {
        if (!FileRead(file, &chk, 8))
          break;

        Log("> %4s [%d]\n", (char *)&chk[1], chk[0]);

        switch (chk[1]) {
          case PNG_IHDR:
            {
              FileRead(file, &ihdr, sizeof(ihdr));

              Log("size : %d x %d, %s, bit depth : %d, %s\n",
                  ihdr.width, ihdr.height, s_type[ihdr.colour_type], 
                  ihdr.bit_depth, s_interlace[ihdr.interlace_method]);
            }
            break;

          case PNG_PLTE:
            {
              RGB *plte = MemAlloc(chk[0], MEMF_PUBLIC|MEMF_CLEAR);
              short n = chk[0] / 3;

              FileRead(file, plte, chk[0]);

              Log("%d : ", n);
              {
                RGB *ptr = plte;
                short i;
                for (i = 0; i < n; i++, ptr++)
                  Log("#%02x%02x%02x ", ptr->r, ptr->g, ptr->b);
              }
              Log("\n");
              MemFree(plte);
            }
            break;

          case PNG_tRNS:
            {
              PngTransparencyT trns;
              u_char type = ihdr.colour_type & 3;

              FileRead(file, &trns, chk[0]);

              if (type == PNG_GRAYSCALE)
                Log("#%d\n", trns.type0.v);
              else if (type == PNG_trueCOLOR)
                Log("#%02x%02x%02x\n",
                    trns.type2.r, trns.type2.g, trns.type2.b);
              else if (type == PNG_INDEXED) {
                short n = chk[0];
                short i;

                for (i = 0; i < n; i++)
                  Log("#%02x ", trns.type3.alpha[i]);
                Log("\n");
              }
            }
            break;

          case PNG_bKGD:
            {
              PngBackgroundT bkgd;
              u_char type = ihdr.colour_type & 3;

              FileRead(file, &bkgd, chk[0]);

              if (type == PNG_GRAYSCALE)
                Log("#%d\n", bkgd.type0.v);
              else if (type == PNG_trueCOLOR)
                Log("#%02x%02x%02x\n",
                    bkgd.type2.r, bkgd.type2.g, bkgd.type2.b);
              else if (type == PNG_INDEXED)
                Log("#%d\n", bkgd.type3.v);
            }
            break;

          case PNG_tEXt:
            {
              char *text = MemAlloc(chk[0] + 1, MEMF_PUBLIC|MEMF_CLEAR);

              FileRead(file, text, chk[0]);

              Log("%s %s\n", text, text + strlen(text) + 1);

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
      Log("'%s' is not a PNG file!\n", filename);
    }

    CloseFile(file);
  }

  return 0;
}
