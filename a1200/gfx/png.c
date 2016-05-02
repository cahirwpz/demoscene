#include "std/debug.h"
#include "std/memory.h"
#include "system/inflate.h"
#include "system/rwops.h"
#include "gfx/png.h"

#define PNG_ID0 MAKE_ID(0x89, 0x50, 0x4e, 0x47)
#define PNG_ID1 MAKE_ID(0x0d, 0x0a, 0x1a, 0x0a)

#define PNG_IEND MAKE_ID('I', 'E', 'N', 'D')
#define PNG_IHDR MAKE_ID('I', 'H', 'D', 'R')
#define PNG_IDAT MAKE_ID('I', 'D', 'A', 'T')
#define PNG_PLTE MAKE_ID('P', 'L', 'T', 'E')
#define PNG_tRNS MAKE_ID('t', 'R', 'N', 'S')

static inline int GetPixelWidth(PngT *png) {
  int pixelWidth = 1;

  if (png->ihdr.colour_type == PNG_TRUECOLOR)
    pixelWidth = 3;
  else if (png->ihdr.colour_type == PNG_GRAYSCALE_ALPHA)
    pixelWidth = 2;
  else if (png->ihdr.colour_type == PNG_TRUECOLOR_ALPHA)
    pixelWidth = 4;

  return pixelWidth;
}

static inline int PaethPredictor(int a, int b, int c) {
  int p = a + b - c;
  int pa = p - a;
  int pb = p - b;
  int pc = p - c;

  if (pa < 0)
    pa = -pa;
  if (pb < 0)
    pb = -pb;
  if (pc < 0)
    pc = -pc;

  if ((pa <= pb) && (pa <= pc)) return a;
  if (pb <= pc) return b;
  return c;
}

static void ReconstructImage(uint8_t *pixels, uint8_t *encoded,
                             int width, int height, int pixelWidth)
{
  int row = width * pixelWidth;
  int i;

  for (i = 0; i < height; i++) {
    uint8_t method = *encoded++;

    if (method == 2 && i == 0)
      method = 0;

    if (method == 4 && i == 0)
      method = 1;

    /*
     * Filters are applied to bytes, not to pixels, regardless of the bit depth
     * or colour type of the image. The filters operate on the byte sequence
     * formed by a scanline.
     */

    if (method == 0) {
      memcpy(pixels, encoded, row);
      encoded += row; pixels += row;
    } else if (method == 1) {
      uint8_t *left = pixels;
      int16_t j = row - 1;
      *pixels++ = *encoded++;
      do {
        *pixels++ = *encoded++ + *left++;
      } while (--j);
    } else if (method == 2) {
      uint8_t *up = pixels - row;
      int16_t j = row;
      do {
        *pixels++ = *encoded++ + *up++;
      } while (--j);
    } else if (method == 3) {
      uint8_t *left = pixels;
      int16_t j = row - 1;
      if (i > 0) {
        uint8_t *up = pixels - row;
        *pixels++ = *encoded++ + *up++ / 2;
        do {
          *pixels++ = *encoded++ + (*left++ + *up++) / 2;
        } while (--j);
      } else {
        *pixels++ = *encoded++;
        do {
          *pixels++ = *encoded++ + *left++ / 2;
        } while (--j);
      }
    } else if (method == 4) {
      uint8_t *left = pixels;
      uint8_t *leftup = pixels - row;
      uint8_t *up = pixels - row;
      int16_t j = row - 1;
      *pixels++ = *encoded++ + *up++;
      do {
        *pixels++ = *encoded++ + PaethPredictor(*left++, *up++, *leftup++);
      } while (--j);
    }
  }
}

/* Collapse multiple IDAT chunks into single one. */
static void MergeIDATs(PngT *png) {
  if (png->idat.next) {
    uint32_t length, i;
    uint8_t *data;
    IdatT *idat;

    for (idat = &png->idat, length = 0; idat; idat = idat->next)
      length += idat->length;

    LOG("Merged chunk length: %d.", length);

    data = MemNew(length);

    for (idat = &png->idat, i = 0; idat;) {
      IdatT *next = idat->next;

      MemCopy(data + i, idat->data, idat->length);
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

bool PngDecodeImage(PngT *png, PixBufT *pixbuf) {
  if (png->ihdr.interlace_method != 0) {
    LOG("Interlaced PNG not supported.");
  } else if (png->ihdr.bit_depth != 8) {
    LOG("Non 8-bit components not supported.");
  } else {
    uint32_t pixelWidth = GetPixelWidth(png);
    uint32_t length = png->ihdr.width * png->ihdr.height * pixelWidth;
    uint32_t dstLength = length + png->ihdr.height;
    uint8_t *encoded = MemNew(dstLength);

    MergeIDATs(png);

    LOG("Uncompressing the image.");

    Inflate(png->idat.data + 2, encoded);

    LOG("Decoding pixels.");

    ReconstructImage(pixbuf->data, encoded,
                     png->ihdr.width, png->ihdr.height, pixelWidth);

    MemUnref(encoded);

    return true;
  }

  return false;
}

typedef struct {
  uint32_t length;
  uint32_t id;
} PngChunkT;

static bool ReadPNG(PngT *png, RwOpsT *stream) {
  uint32_t id[2];
  bool error = false;
  PngChunkT chunk;

  memset(png, 0, sizeof(PngT));

  if (!IoRead32(stream, &id[0]) || !IoRead32(stream, &id[1])) {
    LOG("Could not read PNG header!");
    return false;
  }

  if (id[0] != PNG_ID0 || id[1] != PNG_ID1) {
    LOG("Not a PNG file!");
    return false;
  }

  memset(&chunk, 0, sizeof(chunk));

  while (chunk.id != PNG_IEND && !error) {
    uint32_t their_crc;
    uint8_t *ptr;

    if (IoRead(stream, &chunk, 8) != 8)
      return false;

    LOG("%.4s: length: %d", (char *)&chunk.id, chunk.length);

    ptr = MemNew(chunk.length);

    if (IoRead(stream, ptr, chunk.length) != chunk.length)
      return false;

    if (chunk.id == PNG_IHDR) {
      MemCopy(&png->ihdr, ptr, sizeof(IhdrT));
    } else if (chunk.id == PNG_IDAT) {
      if (!png->idat.data) {
        png->idat.length = chunk.length;
        png->idat.data = ptr;
      } else {
        IdatT *idat = &png->idat;

        while (idat->next)
          idat = idat->next;

        idat->next = NewRecord(IdatT);
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
        MemCopy(png->trns->type3.alpha, ptr, chunk.length);
      } else {
        png->trns = (TrnsT *)ptr;
        ptr = NULL;
      }
    }

    if (ptr)
      MemUnref(ptr);

    if (!IoRead32(stream, &their_crc))
      return false;
  }

  return !error;
}

static void DeletePng(PngT *png) {
  IdatT *idat = &png->idat;

  do {
    IdatT *next = idat->next;

    MemUnref(idat->data);
    if (idat != &png->idat)
      MemUnref(idat);

    idat = next;
  } while (idat);

  if (png->plte.colors)
    MemUnref(png->plte.colors);

  if (png->trns)
    MemUnref(png->trns);
}

TYPEDECL(PngT, (FreeFuncT)DeletePng);

PngT *PngLoadFromFile(const char *path) {
  RwOpsT *stream;

  if ((stream = RwOpsFromFile(path, "r"))) {
    PngT *png = NewInstance(PngT);

    if (ReadPNG(png, stream)) {
      LOG("Loaded '%s' file.", path);
    } else {
      MemUnref(png);
      png = NULL;
    }

    IoClose(stream);
    return png;
  }

  return NULL;
}

static PixBufT *PngToPixBuf(PngT *png) {
  PixBufT *pixbuf = NULL;

  if (png->ihdr.bit_depth == 8) {
    int16_t type;

    switch (png->ihdr.colour_type) {
      case PNG_GRAYSCALE:
        type = PIXBUF_GRAY;
        break;

      case PNG_INDEXED:
        type = PIXBUF_CLUT;
        break;

      default:
        type = -1;
        break;
    }

    if (type >= 0) {
      pixbuf = NewPixBuf(type, png->ihdr.width, png->ihdr.height);

      if (!PngDecodeImage(png, pixbuf)) {
        MemUnref(pixbuf);
        pixbuf = NULL;
      } else {
        PixBufCalculateHistogram(pixbuf);

        LOG("The image has %d (%d..%d) colors.",
            (int)pixbuf->lastColor - (int)pixbuf->baseColor + 1,
            pixbuf->baseColor, pixbuf->lastColor);

        if (png->trns) {
          pixbuf->mode = BLIT_TRANSPARENT;
          pixbuf->baseColor = png->trns->type3.alpha[0];

          LOG("The image is transparent (color = %d).", pixbuf->baseColor);
        }
      }
    }
  }

  return pixbuf;
}

static PaletteT *PngToPalette(PngT *png) {
  if ((png->ihdr.bit_depth == 8) && (png->ihdr.colour_type == PNG_INDEXED)) {
    PaletteT *palette = NewPalette(png->plte.no_colors);

    LOG("The palette has %d colors.", png->plte.no_colors);
    MemCopy(palette->colors, png->plte.colors, sizeof(RGB) * palette->count);

    return palette;
  }

  return NULL;
}

void LoadPngImage(PixBufT **imgPtr, PaletteT **palPtr, const char *pngFile) {
  PngT *png = PngLoadFromFile(pngFile);

  if (!png)
    PANIC("Could not load '%s' file.", pngFile);

  if (imgPtr)
    *imgPtr = PngToPixBuf(png);
  if (palPtr)
    *palPtr = PngToPalette(png);

  MemUnref(png);
}
