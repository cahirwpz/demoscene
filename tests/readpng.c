#include <stdio.h>

#include "std/debug.h"
#include "std/memory.h"
#include "gfx/png.h"
#include "tinf/tinf.h"

void LoadPng(const char *path) {
  PngT *png = PngLoadFromFile(path);

  if (png) {
    PixBufT *pixbuf;
    int16_t type;

    puts("IHDR:");
    printf("  width      : %d\n", png->ihdr.width);
    printf("  height     : %d\n", png->ihdr.height);
    printf("  bit depth  : %d\n", png->ihdr.bit_depth);
    printf("  color type : ");

    if (png->ihdr.colour_type == PNG_GRAYSCALE) {
      type = PIXBUF_GRAY;
      puts("gray scale");
    } else if (png->ihdr.colour_type == PNG_TRUECOLOR) {
      type = PIXBUF_RGB;
      puts("true color");
    } else if (png->ihdr.colour_type == PNG_INDEXED) {
      type = PIXBUF_CLUT;
      puts("indexed color");
    } else if (png->ihdr.colour_type == PNG_GRAYSCALE_ALPHA) {
      type = PIXBUF_GRAY;
      puts("gray scale (with alpha)");
    } else {
      type = PIXBUF_RGBA;
      puts("true color (with alpha)");
    }

    printf("  interlace  : %s\n", png->ihdr.interlace_method ? "yes" : "no");

    if (png->plte.no_colors) {
      puts("PLTE:");
      printf("  colors     : %d\n", png->plte.no_colors);
    }

    if (png->trns) {
      puts("tRNS:");
      if (png->ihdr.colour_type == PNG_INDEXED)
        printf("  color : %d\n", png->trns->type3.alpha[0]);
    }

    pixbuf = NewPixBuf(type, png->ihdr.width, png->ihdr.height);
    PngDecodeImage(png, pixbuf);
    MemUnref(pixbuf);
  }

  puts("");
  MemUnref(png);
}

int main(int argc, char **argv) {
  int i;

  for (i = 1; i < argc ; i++)
    LoadPng(argv[i]);

  return 0;
}
