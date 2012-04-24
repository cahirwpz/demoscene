#include <string.h>
#include <stdarg.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "gfx/palette.h"

PaletteT *NewPalette(size_t count) {
  PaletteT *palette = NEW_S(PaletteT);

  palette->count = count;
  palette->colors = NEW_A(ColorT, count);

  return palette;
}

PaletteT *NewPaletteFromFile(const StrT fileName) {
  uint16_t *data = ReadFileSimple(fileName);

  if (data) {
    uint16_t count = data[0];
    PaletteT *palette = NewPalette(count);
    uint8_t *raw = (uint8_t *)&data[1];
    int i;

    for (i = 0; i < count; i++) {
      palette->colors[i].r = raw[i*3];
      palette->colors[i].g = raw[i*3+1];
      palette->colors[i].b = raw[i*3+2];
    }

    LOG("Palette '%s' has %d colors.", fileName, count);

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

  copy->start = palette->start;
  copy->next = rec_copy;

  memcpy(copy->colors, palette->colors, sizeof(ColorT) * palette->count);

  return copy;
}

bool LinkPalettes(PaletteT *palette, ...) {
  int start = 0;
  va_list ap;

  va_start(ap, palette);

  for (;;) {
    PaletteT *next = va_arg(ap, PaletteT *);

    palette->start = start;
    palette->next = next;

    start += palette->count;

    if (!next)
      break;

    palette = next;
  }

  va_end(ap);

  return BOOL(start <= 256);
}

void UnlinkPalettes(PaletteT *palette) {
  while (palette) {
    PaletteT *next = palette->next;

    palette->next = NULL;
    palette = next;
  }
}
