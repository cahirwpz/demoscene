#include <stdio.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include "std/debug.h"
#include "std/memory.h"
#include "gfx/png.h"
#include "tinf/tinf.h"

void LoadPng(const char *path) {
  PngT *png = PngLoadFromFile(path);

  if (png) {
    puts("IHDR:");
    printf("  width      : %ld\n", png->ihdr.width);
    printf("  height     : %ld\n", png->ihdr.height);
    printf("  bit depth  : %d\n", png->ihdr.bit_depth);
    printf("  color type : ");

    if (png->ihdr.colour_type == PNG_GRAYSCALE)
      puts("gray scale");
    else if (png->ihdr.colour_type == PNG_TRUECOLOR)
      puts("true color");
    else if (png->ihdr.colour_type == PNG_INDEXED)
      puts("indexed color");
    else if (png->ihdr.colour_type == PNG_GRAYSCALE_ALPHA)
      puts("gray scale (with alpha)");
    else
      puts("true color (with alpha)");

    printf("  interlace  : %s\n", png->ihdr.interlace_method ? "yes" : "no");

    if (png->plte.no_colors) {
      puts("PLTE:");
      printf("  colors     : %ld\n", png->plte.no_colors);
    }

    if (png->trns) {
      puts("tRNS:");
      if (png->ihdr.colour_type == PNG_INDEXED)
        printf("  color : %d\n", png->trns->type3.alpha[0]);
    }

    MemUnref(PngDecodeImage(png));
  }

  puts("");
  MemUnref(png);
}

int main(int argc, char **argv) {
  int i;

  tinf_init();

  for (i = 1; i < argc ; i++)
    LoadPng(argv[i]);

  return 0;
}
