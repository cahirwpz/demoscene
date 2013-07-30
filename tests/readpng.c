#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "std/types.h"
#include "tinf/tinf.h"

static char PNGID[8] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a };

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

  if (ihdr->colour_type == 2)
    pixelWidth = 3;
  else if (ihdr->colour_type == 4)
    pixelWidth = 2;
  else if (ihdr->colour_type == 6)
    pixelWidth = 4;

  return pixelWidth;
}

typedef struct {
  uint32_t length;
  uint8_t *data;
} IDAT;

typedef struct {
  uint8_t r, g, b;
} __attribute__((packed)) RGB;

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
    uint8_t alpha[0];
  } type3;
} tRNS;

typedef struct {
  IHDR *ihdr;
  IDAT idat;
  PLTE plte;
  uint32_t trns_length;
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

static bool DecodeImage(uint8_t *pixels, uint8_t *compressed, int srcLength,
                        int width, int height, int pixelWidth)
{
  unsigned int dstLength;
  uint32_t length = width * height * pixelWidth + height;
  uint8_t *encoded;
 
  if ((encoded = malloc(length))) {
    dstLength = length;

    puts("Uncompressing the image.");

    assert(tinf_uncompress(encoded, &dstLength, compressed + 2, srcLength - 2) == TINF_OK);
    assert(length == dstLength);

    puts("Decoding pixels.");

    ReconstructImage(pixels, encoded, width, height, pixelWidth);

    free(encoded);
    return true;
  }

  return false;
}

typedef struct {
  uint32_t length;
  char id[4];
} PNGChunkT;

static bool ReadPNG(PNG *png, int fd) {
  char id[8];
  bool error = false;
  PNGChunkT chunk;

  memset(png, 0, sizeof(PNG));

  if (read(fd, id, 8) != 8)
    return false;

  if (memcmp(id, PNGID, 8))
    return false;

  memset(&chunk, 0, sizeof(chunk));

  while (memcmp(chunk.id, "IEND", 4) && !error) {
    unsigned int their_crc, my_crc;
    unsigned char *ptr;

    if (read(fd, &chunk, 8) != 8)
      return false;

    printf("%.4s: length: %ld, ", chunk.id, chunk.length);
    my_crc = tinf_crc32(0, (void *)chunk.id, 4);

    ptr = malloc(chunk.length);

    if (read(fd, ptr, chunk.length) != chunk.length)
      return false;

    my_crc = tinf_crc32(my_crc, ptr, chunk.length);

    if (!memcmp(chunk.id, "IHDR", 4)) {
      IHDR *ihdr = (IHDR *)ptr;

      assert(ihdr->compression_method == 0);
      assert(ihdr->filter_method == 0);

      png->ihdr = ihdr;
    } else if (!memcmp(chunk.id, "IDAT", 4)) {
      if (png->idat.data > 0) {
        puts("error!");
        puts("Only one IDAT chunk supported. "
             "Use pngcrush or optipng tools to simplify PNG structure.");
        error = true;
        free(ptr);
      } else {
        png->idat.length = chunk.length;
        png->idat.data = ptr;
      }
    } else if (!memcmp(chunk.id, "PLTE", 4)) {
      png->plte.no_colors = chunk.length / 3;
      png->plte.colors = (RGB *)ptr;
    } else if (!memcmp(chunk.id, "tRNS", 4)) {
      png->trns = (tRNS *)ptr;
      if (png->ihdr->colour_type == 3)
        png->trns_length = chunk.length;
    } else {
      free(ptr);
    }

    if (read(fd, &their_crc, 4) != 4)
      return false;

    printf("crc: %s\n", their_crc == my_crc ? "ok" : "bad");

    if (their_crc != my_crc)
      return false;
  }

  return !error;
}

void LoadPNG(const char *path) {
  PNG png;
  int fd;

  if ((fd = open(path, O_RDONLY)) != -1) {
    if (ReadPNG(&png, fd)) {
      printf("PNG '%s'\n", path);

      puts("IHDR:");
      printf("  width      : %ld\n", png.ihdr->width);
      printf("  height     : %ld\n", png.ihdr->height);
      printf("  bit depth  : %d\n", png.ihdr->bit_depth);
      printf("  color type : ");

      if (png.ihdr->colour_type == 0)
        puts("gray scale");
      else if (png.ihdr->colour_type == 2)
        puts("true color");
      else if (png.ihdr->colour_type == 3)
        puts("indexed color");
      else if (png.ihdr->colour_type == 4)
        puts("gray scale (with alpha)");
      else if (png.ihdr->colour_type == 6)
        puts("true color (with alpha)");
      else
        printf("? (%d)\n", png.ihdr->colour_type);

      printf("  interlace  : %s\n", png.ihdr->interlace_method ? "yes" : "no");

      if (png.plte.no_colors) {
        puts("PLTE:");
        printf("  colors     : %ld\n", png.plte.no_colors);
      }

      if (png.trns) {
        puts("tRNS:");
        if (png.ihdr->colour_type == 3)
          printf("  color : %d\n", png.trns->type3.alpha[0]);
      }

      if (png.idat.data) {
        if (png.ihdr->interlace_method != 0) {
          puts("Interlaced PNG not supported.");
        } else if (png.ihdr->bit_depth != 8) {
          puts("Non 8-bit components not supported.");
        } else {
          int pixelWidth = GetPixelWidth(png.ihdr);
          int length = png.ihdr->width * png.ihdr->height * pixelWidth;
          uint8_t *pixels = malloc(length);

          if (DecodeImage(pixels, png.idat.data, png.idat.length,
                          png.ihdr->width, png.ihdr->height, GetPixelWidth(png.ihdr)))
          {
            free(png.idat.data);
            png.idat.data = pixels;
            png.idat.length = length;
          }
        }
      }

      puts("");
    }

    close(fd);

    if (png.ihdr)
      free(png.ihdr);
    if (png.trns)
      free(png.trns);
    if (png.idat.data)
      free(png.idat.data);
    if (png.plte.colors)
      free(png.plte.colors);
  }
}

int main(int argc, char **argv) {
  int i;

  tinf_init();

  for (i = 1; i < argc ; i++)
    LoadPNG(argv[i]);

  return 0;
}
