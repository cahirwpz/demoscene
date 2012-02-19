#include <string.h>
#include <stdarg.h>

#include "system/memory.h"
#include "system/fileio.h"
#include "system/debug.h"
#include "gfx/palette.h"

PaletteT *NewPalette(size_t count) {
  PaletteT *palette = NEW_SZ(PaletteT);

  if (palette) {
    palette->count = count;
    palette->colors = NEW_A(ColorT, count);

    if (palette->colors)
      return palette;

    DeletePalette(palette);
  }

  return NULL;
}

PaletteT *NewPaletteFromFile(const char *fileName, uint32_t memFlags) {
  uint16_t *data = ReadFileSimple(fileName, memFlags);

  if (data) {
    uint16_t count = data[0];

    PaletteT *palette = NewPalette(count);

    if (palette) {
      uint8_t *raw = (uint8_t *)&data[1];

      int i;

      for (i = 0; i < count; i++) {
        palette->colors[i].r = raw[i*3];
        palette->colors[i].g = raw[i*3+1];
        palette->colors[i].b = raw[i*3+2];
      }
    }

    LOG("Palette '%s' has %d colors.\n", fileName, count);

    DELETE(data);

    return palette;
  }

  return NULL;
}

void DeletePalette(PaletteT *palette) {
  while (palette) {
    PaletteT *next = palette->next;

    DELETE(palette->colors);
    DELETE(palette);

    palette = next;
  }
}

PaletteT *CopyPalette(PaletteT *palette) {
  PaletteT *rec_copy = NULL;
  PaletteT *copy;

  if (palette->next) {
    rec_copy = CopyPalette(palette->next);

    if (!rec_copy)
      return NULL;
  }

  copy = NewPalette(palette->count);

  if (copy) {
    copy->start = palette->start;
    copy->next = rec_copy;
    memcpy(copy->colors, palette->colors, sizeof(ColorT) * palette->count);
  }

  return copy;
}

bool LinkPalettes(size_t count, ...) {
  PaletteT *prev = NULL;
  int start = 0;
  va_list ap;

  va_start(ap, count);

  while (count--) {
    PaletteT *palette = va_arg(ap, PaletteT *);

    palette->start = start;

    if (prev)
      prev->next = palette;

    prev = palette;
    start += palette->count;
  }

  va_end(ap);

  return (start <= 256) ? TRUE : FALSE;
}

void UnlinkPalettes(PaletteT *palette) {
  while (palette) {
    PaletteT *next = palette->next;

    palette->next = NULL;

    palette = next;
  }
}
