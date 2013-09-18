#include <string.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
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
      (type == PIXBUF_RGB24) ? 24 : 8, (int)width, (int)height);

  pixbuf->ownership = true;

  switch (type) {
    case PIXBUF_CLUT:
    case PIXBUF_GRAY:
      pixbuf->data = NewTable(uint8_t, width * height);
      pixbuf->lastColor = 255;
      break;

    case PIXBUF_RGB24:
      pixbuf->data = (uint8_t *)NewTable(RGB, width * height);
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
  uint32_t uniqueColors;
  uint8_t  baseColor;
  uint8_t  lastColor;
  uint8_t  data[0];
} DiskPixBufT;

PixBufT *NewPixBufFromFile(const char *fileName) {
  DiskPixBufT *file = (DiskPixBufT *)ReadFileSimple(fileName);

  if (file) {
    PixBufT *pixbuf = NewPixBuf(file->type, file->width, file->height);

    pixbuf->mode = file->mode;
    pixbuf->uniqueColors = file->uniqueColors;

    if (file->type == PIXBUF_CLUT || file->type == PIXBUF_GRAY) {
      pixbuf->baseColor = file->baseColor;
      pixbuf->lastColor = file->lastColor;

      LOG("Image '%s' has size (%d,%d) and %d colors.",
          fileName, (int)file->width, (int)file->height,
          (int)file->lastColor - (int)file->baseColor);
    } else {
      LOG("True color image '%s' has size (%d,%d).",
          fileName, (int)file->width, (int)file->height);
    }

    MemCopy(pixbuf->data, file->data, 
            TableSize(pixbuf->data) * TableElemSize(pixbuf->data));

    MemUnref(file);

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

void PixBufSetColorMap(PixBufT *pixbuf, PixBufT *colorMap, int colorShift) {
  ASSERT((colorMap->type == PIXBUF_GRAY || colorMap->type == PIXBUF_CLUT) &&
         colorMap->width == 256,
         "Color map must be 8-bit gray image of width 256.");
  pixbuf->blit.cmap.data = colorMap->data;
  pixbuf->blit.cmap.shift = colorShift;
}

void PixBufSetColorFunc(PixBufT *pixbuf, uint8_t *colorFunc) {
  pixbuf->blit.cfunc.data = colorFunc;
}

BlitModeT PixBufSetBlitMode(PixBufT *pixbuf, BlitModeT mode) {
  BlitModeT old_mode = pixbuf->mode;
  pixbuf->mode = mode;
  return old_mode;
}

void PixBufSwapData(PixBufT *buf1, PixBufT *buf2) {
  uint8_t *tmp;

  ASSERT(buf1->width == buf2->width,
         "Width does not match (%ld != %ld)", buf1->width, buf2->width);
  ASSERT(buf1->height == buf2->height,
         "Height does not match (%ld != %ld)", buf1->height, buf2->height);

  tmp = buf1->data;
  buf1->data = buf2->data;
  buf2->data = tmp;
}

void PixBufCopy(PixBufT *dst, PixBufT *src) {
  ASSERT(src->width == dst->width,
         "Width does not match (%ld != %ld)", src->width, dst->width);
  ASSERT(src->height == dst->height,
         "Height does not match (%ld != %ld)", src->height, dst->height);
  memcpy(dst->data, src->data, src->width * src->height);
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

static const size_t inline GetPixelIndex(PixBufT *pixbuf, ssize_t x, ssize_t y) {
  ASSERT((x >= 0) && (x < pixbuf->width), "x (%d) out of bound!", x);
  ASSERT((y >= 0) && (y < pixbuf->height), "y (%d) out of bound!", y);
  return x + pixbuf->width * y;
}

void PutPixel(PixBufT *pixbuf asm("a0"), int x asm("a1"), int y asm("d1"),
              int c asm("d0"))
{
  size_t index = GetPixelIndex(pixbuf, x, y);
  pixbuf->data[index] = c;
}

int GetPixel(PixBufT *pixbuf asm("a0"), int x asm("d0"), int y asm("d1"))
{
  return pixbuf->data[GetPixelIndex(pixbuf, x, y)];
}

void PutPixelRGB(PixBufT *pixbuf asm("a0"), int x asm("a1"), int y asm("d1"),
                 RGB c asm("d0"))
{
  size_t index = GetPixelIndex(pixbuf, x, y);
  ((uint32_t *)pixbuf->data)[index] = *(uint32_t *)&c;
}

RGB GetPixelRGB(PixBufT *pixbuf asm("a0"), int x asm("d0"), int y asm("d1"))
{
  return *(RGB *)&pixbuf->data[GetPixelIndex(pixbuf, x, y)];
}

int GetFilteredPixel(PixBufT *pixbuf asm("a0"),
                     Q16T x asm("d0"), Q16T y asm("d1"))
{
  uint8_t *data = &pixbuf->data[pixbuf->width * y.integer + x.integer];

  int p1 = data[0];
  int p2 = data[1];
  int p3 = data[pixbuf->width];
  int p4 = data[pixbuf->width + 1];

  int d31 = p1 + ((p3 - p1) * y.fraction >> 16);
  int d42 = p2 + ((p4 - p2) * y.fraction >> 16);

	return d31 + ((d42 - d31) * x.fraction >> 16);
}
