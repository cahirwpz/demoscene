#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/rwops.h"
#include "gfx/pixbuf.h"

static void DeletePixBuf(PixBufT *pixbuf) {
  if (pixbuf->ownership)
    MemUnref(pixbuf->data);
}

TYPEDECL(PixBufT, (FreeFuncT)DeletePixBuf);

PixBufT *NewPixBuf(uint16_t type, size_t width, size_t height) {
  PixBufT *pixbuf = NewInstance(PixBufT);

  pixbuf->type = type;
  pixbuf->width = width;
  pixbuf->height = height;

  LOG("Creating %d-bit image of size (%d,%d).",
      (type >= PIXBUF_RGB) ? 24 : 8, (int)width, (int)height);

  pixbuf->ownership = true;

  switch (type) {
    case PIXBUF_CLUT:
    case PIXBUF_GRAY:
      pixbuf->data = NewTable(uint8_t, width * height);
      pixbuf->lastColor = 255;
      break;

    case PIXBUF_RGB:
      pixbuf->data = (uint8_t *)NewTable(RGB, width * height);
      break;

    case PIXBUF_RGBA:
      pixbuf->data = (uint8_t *)NewTable(RGBA, width * height);
      break;

    default:
      PANIC("Unknown PixBuf type: %d", type);
      break;
  }

  pixbuf->fgColor = 255;

  return pixbuf;
}

typedef struct DiskPixBuf {
  uint8_t  type;
  uint8_t  mode;
  uint16_t width;
  uint16_t height;
  uint8_t  data[0];
} DiskPixBufT;

PixBufT *NewPixBufFromFile(const char *fileName) {
  DiskPixBufT *file = (DiskPixBufT *)ReadFileSimple(fileName);

  if (file) {
    PixBufT *pixbuf = NewPixBuf(file->type, file->width, file->height);

    pixbuf->mode = file->mode;

    MemCopy(pixbuf->data, file->data, 
            TableSize(pixbuf->data) * TableElemSize(pixbuf->data));

    MemUnref(file);

    if (pixbuf->type == PIXBUF_CLUT || pixbuf->type == PIXBUF_GRAY) {
      PixBufCalculateHistogram(pixbuf);

      LOG("Image '%s' has size (%d,%d) and %d colors.",
          fileName, pixbuf->width, pixbuf->height,
          (int)pixbuf->lastColor - (int)pixbuf->baseColor + 1);
    } else {
      LOG("True color image '%s' has size (%d,%d).",
          fileName, pixbuf->width, pixbuf->height);
    }

    return pixbuf;
  }

  return NULL;
}

PixBufT *NewPixBufWrapper(size_t width, size_t height, uint8_t *data) {
  PixBufT *pixbuf = NewInstance(PixBufT);

  pixbuf->type = PIXBUF_GRAY;
  pixbuf->width = width;
  pixbuf->height = height;
  pixbuf->ownership = false;
  pixbuf->data = data;
  pixbuf->lastColor = 255;
  pixbuf->fgColor = 1;

  return pixbuf;
}

void PixBufSetColorMap(PixBufT *pixbuf, PixBufT *colorMap) {
  ASSERT((colorMap->type == PIXBUF_GRAY || colorMap->type == PIXBUF_CLUT) &&
         colorMap->width == 256,
         "Color map must be 8-bit gray image of width 256.");
  pixbuf->blit.cmap = colorMap->data;
}

void PixBufSetColorFunc(PixBufT *pixbuf, uint8_t *colorFunc) {
  pixbuf->blit.cfunc = colorFunc;
}

BlitModeT PixBufSetBlitMode(PixBufT *pixbuf, BlitModeT mode) {
  BlitModeT old_mode = pixbuf->mode;
  pixbuf->mode = mode;
  return old_mode;
}

void PixBufSwapData(PixBufT *buf1, PixBufT *buf2) {
  uint8_t *tmp;

  ASSERT(buf1->width == buf2->width,
         "Width does not match (%d != %d)", buf1->width, buf2->width);
  ASSERT(buf1->height == buf2->height,
         "Height does not match (%d != %d)", buf1->height, buf2->height);

  tmp = buf1->data;
  buf1->data = buf2->data;
  buf2->data = tmp;
}

void PixBufCopy(PixBufT *dst, PixBufT *src) {
  ASSERT(src->width == dst->width,
         "Width does not match (%d != %d)", src->width, dst->width);
  ASSERT(src->height == dst->height,
         "Height does not match (%d != %d)", src->height, dst->height);
  MemCopy(dst->data, src->data, src->width * src->height);
}

void PixBufClear(PixBufT *pixbuf) {
  memset(pixbuf->data, pixbuf->bgColor, pixbuf->width * pixbuf->height);
}

void PixBufRemap(PixBufT *pixbuf, PaletteT *palette) {
  ASSERT(pixbuf->type == PIXBUF_CLUT || pixbuf->type == PIXBUF_GRAY,
         "Cannot remap non-8bit image!");

  ASSERT((pixbuf->lastColor - pixbuf->baseColor + 1) <= palette->count,
         "There are more colors in PixBuf than in palette (%d > %d).",
         (int)(pixbuf->lastColor - pixbuf->baseColor + 1), (int)palette->count);

  {
    int color = palette->start - pixbuf->baseColor;
    int i;

    LOG("Remapping by %d colors.", color);

    for (i = 0; i < pixbuf->width * pixbuf->height; i++) {
      if ((pixbuf->mode == BLIT_TRANSPARENT) && (pixbuf->data[i] == 0))
          continue;

      pixbuf->data[i] += color;
    }

    pixbuf->baseColor = palette->start;
    pixbuf->lastColor = palette->start + palette->count - 1;
  }
}

void PixBufCalculateHistogram(PixBufT *pixbuf) {
  if (pixbuf->type == PIXBUF_CLUT || pixbuf->type == PIXBUF_GRAY) {
    uint32_t *histogram = NewTable(uint32_t, 256);
    uint32_t i;

    for (i = 0; i < pixbuf->width * pixbuf->height; i++)
      histogram[pixbuf->data[i]]++;

    /* Calculate unique colors. */
    for (i = 0; i < 256; i++) {
      if (histogram[i])
        pixbuf->uniqueColors++;
    }

    /* Calculare the lowest color. */
    for (i = 0; i < 256; i++)
      if (histogram[i])
        break;

    pixbuf->baseColor = i;

    /* Calculare the highest color. */
    for (i = 255; i >= 0; i--)
      if (histogram[i])
        break;

    pixbuf->lastColor = i;

    MemUnref(histogram);
  }
}

__regargs int GetFilteredPixel(PixBufT *pixbuf, FP16 x, FP16 y) {
  uint8_t *data = &pixbuf->data[pixbuf->width * FP16_i(y) + FP16_i(x)];

  int p1 = data[0];
  int p2 = data[1];
  int p3 = data[pixbuf->width];
  int p4 = data[pixbuf->width + 1];

  int d31 = p1 + ((p3 - p1) * FP16_f(y) >> 16);
  int d42 = p2 + ((p4 - p2) * FP16_f(y) >> 16);

	return d31 + ((d42 - d31) * FP16_f(x) >> 16);
}
