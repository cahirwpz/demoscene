#include <stdio.h>
#include <dos/dos.h>
#include <proto/dos.h>

#include "gfx/colors.h"
#include "std/debug.h"
#include "std/memory.h"
#include "std/types.h"
#include "tinf/tinf.h"

#define PNG_ID0 MAKE_ID(0x89, 0x50, 0x4e, 0x47)
#define PNG_ID1 MAKE_ID(0x0d, 0x0a, 0x1a, 0x0a)

#define PNG_IEND MAKE_ID('I', 'E', 'N', 'D')
#define PNG_IHDR MAKE_ID('I', 'H', 'D', 'R')
#define PNG_IDAT MAKE_ID('I', 'D', 'A', 'T')
#define PNG_PLTE MAKE_ID('P', 'L', 'T', 'E')
#define PNG_tRNS MAKE_ID('t', 'R', 'N', 'S')

#define PNG_GRAYSCALE       0
#define PNG_TRUECOLOR       2
#define PNG_INDEXED         3
#define PNG_GRAYSCALE_ALPHA 4
#define PNG_TRUECOLOR_ALPHA 6

typedef struct {
  uint32_t width;
  uint32_t height;
  uint8_t bit_depth;
  uint8_t colour_type;
  uint8_t compression_method;
  uint8_t filter_method;
  uint8_t interlace_method;
} IHDR;

static int GetPixelWidth(IHDR *ihdr) {
  int pixelWidth = 1;

  if (ihdr->colour_type == PNG_TRUECOLOR)
    pixelWidth = 3;
  else if (ihdr->colour_type == PNG_GRAYSCALE_ALPHA)
    pixelWidth = 2;
  else if (ihdr->colour_type == PNG_TRUECOLOR_ALPHA)
    pixelWidth = 4;

  return pixelWidth;
}

typedef struct IDAT_s IDAT;

struct IDAT_s {
  IDAT *next;
  int32_t length;
  uint8_t *data;
};

typedef struct {
  uint32_t no_colors;
  RGB *colors;
} PLTE;

typedef union {
  struct {
    uint16_t v;
  } type0;
  struct {
    uint16_t r, g, b;
  } type2;
  struct {
    size_t length;
    uint8_t alpha[0];
  } type3;
} tRNS;

typedef struct {
  IHDR ihdr;
  IDAT idat;
  PLTE plte;
  tRNS *trns;
} PNG;

inline static int
PaethPredictor(uint8_t a, uint8_t b, uint8_t c) {
  int p = a + b - c;
  int pa = abs(p - a);
  int pb = abs(p - b);
  int pc = abs(p - c);

  if (pa <= pb && pa <= pc)
    return a;

  if (pb <= pc)
   return b;

  return c;
}

static void ReconstructImage(uint8_t *pixels, uint8_t *encoded,
                             int width, int height, int pixelWidth)
{
  int row = width * pixelWidth;
  int i, j, k;

  for (i = 0; i < height; i++) {
    uint8_t method = *encoded++;

    for (k = 0; k < pixelWidth; k++) {
      pixels[0] = encoded[0];
      if (method == 2 || method == 4) /* Up & Paeth */
        pixels[0] += pixels[-row];
      else if (method == 3) /* Average */
        pixels[0] += pixels[-row] / 2;

      for (j = pixelWidth; j < row; j += pixelWidth) {
        pixels[j] = encoded[j];
        if (method == 1) /* Sub */
          pixels[j] += pixels[j - pixelWidth];
        else if (method == 2) /* Up */
          pixels[j] += pixels[j - row];
        else if (method == 3) /* Average */
          pixels[j] += (pixels[j - pixelWidth] + pixels[j - row]) / 2;
        else if (method == 4) /* Paeth */
          pixels[j] += PaethPredictor(pixels[j - pixelWidth],
                                      pixels[j - row],
                                      pixels[j - row - pixelWidth]);
      }

      pixels++;
      encoded++;
    }

    pixels += row - pixelWidth;
    encoded += row - pixelWidth;
  }
}

/* Collapse multiple IDAT chunks into single one. */
void MergeIDATs(PNG *png) {
  if (png->idat.next) {
    uint32_t length, i;
    uint8_t *data;
    IDAT *idat;

    for (idat = &png->idat, length = 0; idat; idat = idat->next)
      length += idat->length;

    LOG("Merged chunk length: %ld.", length);

    data = MemNew(length);

    for (idat = &png->idat, i = 0; idat;) {
      IDAT *next = idat->next;

      memcpy(data + i, idat->data, idat->length);
      i += idat->length;

      MemUnref(idat->data);
      if (idat != &png->idat)
        MemUnref(idat);

      idat = next;
    }

    png->idat.data = data;
    png->idat.length = length;
    png->idat.next = NULL;
  }
}

typedef struct {
  uint32_t length;
  uint32_t id;
} PNGChunkT;

static bool ReadPNG(PNG *png, int fd) {
  uint32_t id[2];
  bool error = false;
  PNGChunkT chunk;

  memset(png, 0, sizeof(PNG));

  if (Read(fd, id, 8) != 8)
    return false;

  if (id[0] != PNG_ID0 || id[1] != PNG_ID1)
    return false;

  memset(&chunk, 0, sizeof(chunk));

  while (chunk.id != PNG_IEND && !error) {
    unsigned int their_crc, my_crc;
    unsigned char *ptr;

    if (Read(fd, &chunk, 8) != 8)
      return false;

    my_crc = tinf_crc32(0, (void *)&chunk.id, 4);

    ptr = MemNew(chunk.length);

    if (Read(fd, ptr, chunk.length) != chunk.length)
      return false;

    my_crc = tinf_crc32(my_crc, ptr, chunk.length);

    if (chunk.id == PNG_IHDR) {
      memcpy(&png->ihdr, ptr, sizeof(IHDR));
    } else if (chunk.id == PNG_IDAT) {
      if (!png->idat.data) {
        png->idat.length = chunk.length;
        png->idat.data = ptr;
      } else {
        IDAT *idat = &png->idat;

        while (idat->next)
          idat = idat->next;

        idat->next = NewRecord(IDAT);
        idat->next->length = chunk.length;
        idat->next->data = ptr;
      }
      ptr = NULL;
    } else if (chunk.id == PNG_PLTE) {
      png->plte.no_colors = chunk.length / 3;
      png->plte.colors = (RGB *)ptr;
      ptr = NULL;
    } else if (chunk.id == PNG_tRNS) {
      if (png->ihdr.colour_type == PNG_INDEXED) {
        png->trns = MemNew(sizeof(uint32_t) + chunk.length);
        png->trns->type3.length = chunk.length;
        memcpy(png->trns->type3.alpha, ptr, chunk.length);
      } else {
        png->trns = (tRNS *)ptr;
        ptr = NULL;
      }
    }

    if (ptr)
      MemUnref(ptr);

    if (Read(fd, &their_crc, 4) != 4)
      return false;

    LOG("%.4s: length: %ld, crc: %s",
        (char *)&chunk.id, chunk.length, their_crc == my_crc ? "ok" : "bad");

    if (their_crc != my_crc)
      return false;
  }

  if (!error) {
    MergeIDATs(png);
  }

  return !error;
}

void LoadPNG(const char *path) {
  PNG png;
  BPTR fd;

  if ((fd = Open(path, MODE_OLDFILE))) {
    if (ReadPNG(&png, fd)) {
      printf("PNG '%s'\n", path);

      puts("IHDR:");
      printf("  width      : %ld\n", png.ihdr.width);
      printf("  height     : %ld\n", png.ihdr.height);
      printf("  bit depth  : %d\n", png.ihdr.bit_depth);
      printf("  color type : ");

      if (png.ihdr.colour_type == PNG_GRAYSCALE)
        puts("gray scale");
      else if (png.ihdr.colour_type == PNG_TRUECOLOR)
        puts("true color");
      else if (png.ihdr.colour_type == PNG_INDEXED)
        puts("indexed color");
      else if (png.ihdr.colour_type == PNG_GRAYSCALE_ALPHA)
        puts("gray scale (with alpha)");
      else
        puts("true color (with alpha)");

      printf("  interlace  : %s\n", png.ihdr.interlace_method ? "yes" : "no");

      if (png.plte.no_colors) {
        puts("PLTE:");
        printf("  colors     : %ld\n", png.plte.no_colors);
      }

      if (png.trns) {
        puts("tRNS:");
        if (png.ihdr.colour_type == PNG_INDEXED)
          printf("  color : %d\n", png.trns->type3.alpha[0]);
      }

      if (png.idat.data) {
        if (png.ihdr.interlace_method != 0) {
          puts("Interlaced PNG not supported.");
        } else if (png.ihdr.bit_depth != 8) {
          puts("Non 8-bit components not supported.");
        } else {
          unsigned int pixelWidth = GetPixelWidth(&png.ihdr);
          unsigned int length = png.ihdr.width * png.ihdr.height * pixelWidth;
          unsigned int dstLength = length + png.ihdr.height;
          uint8_t *pixels = MemNew(length);
          uint8_t *encoded = MemNew(dstLength);

          LOG("Uncompressing the image.");

          ASSERT(tinf_uncompress(encoded, &dstLength,
                                 png.idat.data + 2, png.idat.length - 2) == TINF_OK,
                 "Decompression failed.");
          ASSERT(length + png.ihdr.height == dstLength, "Decompressed data size differs.");

          LOG("Decoding pixels.");

          ReconstructImage(pixels, encoded, png.ihdr.width, png.ihdr.height, pixelWidth);

          MemUnref(encoded);
          MemUnref(pixels);
        }
      }

      puts("");
    }

    Close(fd);

    if (png.idat.data)
      MemUnref(png.idat.data);
    if (png.plte.colors)
      MemUnref(png.plte.colors);
    if (png.trns)
      MemUnref(png.trns);
  }
}

int main(int argc, char **argv) {
  int i;

  tinf_init();

  for (i = 1; i < argc ; i++)
    LoadPNG(argv[i]);

  return 0;
}
