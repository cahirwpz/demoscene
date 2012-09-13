#include <string.h>
#include <stdarg.h>

#include "std/debug.h"
#include "std/memory.h"
#include "system/fileio.h"
#include "gfx/palette.h"

static void DeletePalette(PaletteT *palette) {
  MemUnref(palette->colors);

  if (palette->next)
    MemUnref(palette->next);
}

static void CopyPalette(PaletteT *dst, PaletteT *src) {
  dst->colors = MemClone(src->colors);
  dst->start = src->start;
  dst->count = src->count;
  dst->next = (src->next) ? MemClone(src->next) : NULL;
}

TYPEDECL(PaletteT, (FreeFuncT)DeletePalette, (CopyFuncT)CopyPalette);

PaletteT *NewPalette(size_t count) {
  PaletteT *palette = NewInstance(PaletteT);

  palette->count = count;
  palette->colors = NewTable(RGB, count);

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

    MemUnref(data);

    return palette;
  }

  return NULL;
}

bool LinkPalettes(PaletteT *palette, ...) {
  int start = 0;
  va_list ap;

  va_start(ap, palette);

  for (;;) {
    PaletteT *next = va_arg(ap, PaletteT *);

    /* Just for safety. */
    UnlinkPalettes(palette);

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
